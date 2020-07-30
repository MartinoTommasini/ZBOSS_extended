#include "zb_sniffer_radio.h"
#include "zb_sniffer_time.h"
#include "zb_sniffer_leds.h"
#include "zb_sniffer_transport.h"

zb_bool_t is_sniffing;
zb_bool_t fifo_overflow;

void zb_sniffer_rf_init_radio(void)
{
  /* Auto CRC + append RSSI & LQI */
  FRMCTRL0 |= 0x30;
  /* Set promiscuous mode */
  FRMFILT0 = 0;
  SRCMATCH = 0;
  /* Recommended RX settings, copy/paste */
  TXFILTCFG = 0x09;
  AGCCTRL1  = 0x15;
  FSCAL1    = 0x00;
  /* Enable RX interrupt : RXPKTDONE interrupt 1 << 6 & SFD 1 << 1 */
  RFIRQM0 |= 0x42;  // SFD interrupt bit enable can be removed
  /*  enable TX_done interrupt */
  RFIRQM1 |= 0x02;   
  
  /* ... general RF interrupts */
  IEN2 |= 0x01;
#ifdef DEBUG_RF
  /* Enable RFERR interrupt: RX over- under - flow and Strobe error */
  RFERRM |= 0x4C;
  /* RFERR */
  IEN0 |= 0x01;
#endif

  is_sniffing = ZB_FALSE;
  fifo_overflow = ZB_FALSE;
}


void zb_sniffer_rf_start_rx()
{
  if (!is_sniffing)
  {
    ZB_SNIFFER_RF_ISRXON(); 
    is_sniffing = ZB_TRUE;
  }
}

void zb_set_channel(zb_uint8_t channel)
{
  ZB_SNIFFER_RF_SET_CHANNEL(channel);
}

/* Used while debugging firmware. Fill one record in transport_rb */
void zb_populate_transport_rb()
{
  /* Reset transport ring buffer to 0. */
  ZB_RING_BUFFER_INIT(&transport_rb);
  
  zb_sniffer_transport_rb_record_t *record;
  /* pointer to the free write slot */
  record = ZB_RING_BUFFER_WRITE_PEEK(&transport_rb);
  
  /*  Example of ACK frame
    0x5 :       len of an ACK frame (FCS included)
    0x20:       FCF value
    0x00:       //
    0x85:       sequence number
    FCS non included because generated automatically by transceiver.
  */
  //zb_uint8_t data[]= {0x5,0x20,0x00,0x85};
      
    zb_uint8_t data[126];
    data[0]=0x2;
    for(int i=1;i<125;i++)
    {
      data[i]=0x1;
    }
    data[125] = 0x3;
  
  /* insert record header in ring buffer */
  
  record->header.len= sizeof(data);
  
  /* mac packet. mhr + mcd (mac payload).
    Insert record data in ring buffer*/
  for(int i=0;i<sizeof(data);i++)
  {
    record->data[i]=data[i];
  }
  /* update status in ring buffer */
 ZB_RING_BUFFER_WRITTEN(&transport_rb);
}



void zb_sniffer_send_iteration()
{
   while (!is_sniffing && !ZB_RING_BUFFER_IS_EMPTY(&transport_rb))
   {
    zb_rf_start_tx();   
   }

}

void zb_rf_start_tx(){ 
  /* prepare transceiver */
  /* disable rx interrupt */
  RFIRQM0 &= ~(1<<6);   
  /* disable general RF interrupts */
  IEN2 &= ~(0x01);
  
  /* Flush TXFIFO buffer */
  ZB_RF_ISFLUSHTX(); 
  /* Enable TX transceiver */
  ZB_RF_ISTXON();
  /* tx done interrupt cleared */
  RFIRQF1 = ~0x02; 
  {
    zb_uint8_t i;
    zb_sniffer_transport_rb_record_t *record;
    
    /* read record from ring buffer */
    record = ZB_RING_BUFFER_GET(&transport_rb);
    /*pointer to the packet to transmit */
    zb_uint8_t *ptr = record->data;
    
    /*Set red led. It will be cleaned in the interrupt handler.
      Used to provide a visual check of trasmission done*/
    ZB_SNIFFER_RED_LED_SET(); 
    /*record-> header.len contains the len of:
      LEN+ MHR + MCD(payload of mac frame).
      FCS is trasmitted automatically (not in ptr) */
    for (i = 0;i<record->header.len;i++)
    {
      /* write to tx queue */
      RFD = ptr[i];
    }
    /* signal ring buffer that packet has been read */
    ZB_RING_BUFFER_FLUSH_GET(&transport_rb);
  }
  IEN2 |= 0x01; /* enable rf general interrupt back */
  RFIRQM1 |= 0x03;
  RFIRQM0 |= (1<<6); 
}


void zb_sniffer_rf_stop_rx()
{
  if (is_sniffing)
  {
    ZB_SNIFFER_RF_ISFLUSHRX();
    ZB_SNIFFER_RF_ISRFOFF();

    is_sniffing = ZB_FALSE;
  }
}

#ifdef ZB_SNIFFER_IAR
#pragma vector=ZB_SNIFFER_RF_INTERRUPT
#endif
__interrupt void zb_sniffer_rf_interrupt_handler(void)
{
  zb_bool_t need_flush;
  
  ZB_SNIFFER_DISABLE_ALL_INTER();
  
  need_flush = ZB_FALSE;
  

  /* TX packet transmitted */
  if(ZB_SNIFFER_RF_CHECK_PACKET_TX())
  {
     /* TODO: implement check for errors(overflow, underflow)!!! */
     
     /* clear red led, set before trasmission */
     ZB_SNIFFER_RED_LED_CLR(); 
 
  }
  /* RX packet received */
  else if (ZB_SNIFFER_RF_CHECK_PACKET_RX())
  {
    if (!ZB_RING_BUFFER_IS_FULL(&transport_rb))
    {
      zb_sniffer_transport_rb_record_t *record;
      record = ZB_RING_BUFFER_WRITE_PEEK(&transport_rb);
      
      zb_uint8_t *ptr = record->data;
      zb_uint8_t i;

      record->header.len = RXFIFOCNT;
      for (i = 0; i < record->header.len; i++)
      {
        ptr[i] = RFD;
      }
      ZB_RING_BUFFER_WRITTEN(&transport_rb);
    
    }
    else 
    {
      need_flush = ZB_TRUE;
    }

    /* check error flags and flush RX if needed */
    if (ZB_SNIFFER_RF_CHECK_OVERFLOW() || need_flush)
    {
      ZB_SNIFFER_RF_ISFLUSHRX();
    }
    
  }
  
   /* clear interrupt status */
   ZB_SNIFFER_RF_CLEAR_ISRSTS();
   ZB_SNIFFER_ENABLE_ALL_INTER();
 }

#ifdef ZB_SNIFFER_IAR
#pragma vector=ZB_SNIFFER_RFERR_INTERRUPT
#endif
__interrupt void zb_sniffer_rferr_interrupt_handler(void)
{
  TCON &= ~0x02;
  RFERRF = 0;
  ZB_SNIFFER_RF_ISFLUSHRX();
  ZB_SNIFFER_RF_ISRXON();

  fifo_overflow = ZB_TRUE;
}

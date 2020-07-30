#include "zb_ringbuffer.h"
#include "zb_sniffer_time.h"
#include "zb_sniffer_radio.h"
#include "zb_sniffer_transport.h"
#include "zb_sniffer_leds.h"

#define ZB_SNIFFER_START_CMD 0xAA
#define ZB_SNIFFER_STOP_CMD  0xBB
#define ZB_SEND_PACKET_CMD   0xCC   

zb_sniffer_transport_rb_t transport_rb;
zb_sniffer_cmd_rb_t cmd_rb;
zb_bool_t packet_is_arriving = ZB_FALSE;

void zb_sniffer_init(void)
{
  zb_sniffer_leds_init();
  zb_sniffer_timer_init();
  zb_sniffer_rf_init_radio();
  zb_sniffer_transport_init();
}

void zb_sniffer_start()
{
  zb_sniffer_start_timer();
  zb_sniffer_rf_start_rx();
}

void zb_sniffer_stop()
{
  zb_sniffer_rf_stop_rx();
  zb_sniffer_stop_timer();
  zb_sniffer_transport_clear();
}

static void execute_cmd(zb_sniffer_cmd_record_t *cmd)
{
  switch (*cmd)
  {
  case ZB_SNIFFER_START_CMD:
    zb_sniffer_start();
    break;
  case ZB_SNIFFER_STOP_CMD:
    zb_sniffer_stop();
    break;
   case ZB_SEND_PACKET_CMD:
     packet_is_arriving = ZB_TRUE;
    break;
  default:
    if (*cmd >= ZB_SNIFFER_RF_START_CHANNEL_NUMBER && *cmd <= ZB_SNIFFER_RF_MAX_CHANNEL_NUMBER)
    {
      zb_set_channel(*cmd);
    }
    break;
  }
}

void zb_sniffer_process_incoming_cmds(void)
{
  while (!ZB_RING_BUFFER_IS_EMPTY(&cmd_rb))
  {
    zb_sniffer_cmd_record_t *cmd = ZB_RING_BUFFER_GET(&cmd_rb);

    execute_cmd(cmd);
    ZB_RING_BUFFER_FLUSH_GET(&cmd_rb);
  }
}

void zb_sniffer_iteration(void)
{
  zb_sniffer_process_incoming_cmds();
  zb_sniffer_transport_iteration();
  zb_sniffer_send_iteration();
  zb_sniffer_leds_iteration(is_sniffing, fifo_overflow, (zb_bool_t)ZB_RING_BUFFER_IS_FULL(&transport_rb));
}

void main(void)
{
  zb_sniffer_init();
  while(1)
  {
    zb_sniffer_iteration();
  }
}

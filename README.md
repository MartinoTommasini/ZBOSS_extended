# ZBOSS_extended
*ZBOSS_extended* extends the *ZBOSS packet sniffer*, adding injection features for the  CC2531 hardware by Texas Instruments and providing compatibility with KillerBee framework.

## Description

*ZBOSS packet sniffer* is a solution developed by ZBOSS, https://zboss.dsr-wireless.com/, that allows the usage of the CC2531 as a packet sniffer. However it doesn't provide any functionalities regarding the trasmission of packets.  
*ZBOSS_extended* extends the *ZBOSS packet sniffer firmware* implementing injection capabilities in the firmware and modifying it in order to provide support to KillerBee framework. 

The solution is not ready out of the box yet because it needs the KillerBee drivers to be implemented (A Push Request is pending).

## How to use it
The file *zboss_firmware.hex* is the hex of the firmware. It can be flashed to the CC2531 using the CC debugger by Texas Instruments or using a Raspberry Pi. Further instruction on the flashing process and alternative methods can be found in the Zigbee2Mqtt website, https://www.zigbee2mqtt.io/.

The source code of the firmware is available in the *hw* folder.   
Once the project is loaded in IAR, it has to be configured to run on CC2531. Go to Project->Edit Configurations->CC2531.

Datasheet CC2531: https://www.ti.com/lit/ug/swru191f/swru191f.pdf?ts=1596190022846&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FCC2531


In order to add the new features mentioned above, changes has been apported to the following original ZBOSS files:
- hw/usb/class_cdc/usb_uart.c
- hw/zb_sniffer_radio.h
- hw/zb_sniffer.c
- hw/zb_sniffer_radio.c
- hw/zb_sniffer_transport.c


## TODO:
- Add support for 64 byte packets arriving from the host to the CC2531 endpoints.
- Remove reduntant passage of Len field to the host.

## Contacts:
Do not hexitate to contact me at martino.tommasini@gmail.com.

    

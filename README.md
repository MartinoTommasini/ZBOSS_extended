# ZBOSS_extended
ZBOSS packet sniffer extended with injection features for the CC2531 hardware by Texas Instruments.

## Description

*ZBOSS packet sniffer*, https://zboss.dsr-wireless.com/ , allows the usage of the CC2531 as a packet sniffer but it doesn't provide any functionalities regarding the trasmission of packets. *ZBOSS_extended* extends the *ZBOSS packet sniffer* firmware implementing injection capabilities in the firmware and modifying it to support KillerBee framework. 

The solution is not ready out of the box yet because it needs the KillerBee drivers to be implemented (A Push Request is pending).

## How to use it
The file *zboss_extended_firmware.hex* is the hex of the firmware. It can be flashed to the CC2531 using the CC debugger by Texas Instruments or using a Raspberry Pi. Further instruction on the flashing process and alternative methods can be found in the Zigbee2Mqtt website, https://www.zigbee2mqtt.io/.

The source code of the firmware is available in the *hw* folder.   
Once the project is loaded in IAR, it has to be configured to run on CC2531. Go to Project->Edit Configurations->CC2531.

Datasheet CC2531: 

## TODO:
- Add support for 64 byte packets arriving from the host to the CC2531 endpoints.
- Remove reduntant passage of Len field to the host.

## Contacts:
Do not hexitate to contact me at martino.tommasini@gmail.com.

    

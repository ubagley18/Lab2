/* ###################################################################
**     Filename    : packet.c
**     Project     : Lab2
**     Processor   : MK70FN1M0VMJ12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2019-03-21, 13:27, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*
 *  File name  : packet.c
 *  Purpose    : Check and pack data into 5 bytes (A command, three parameters and a checksum)
 *  Project    : Lab1
 *  Created on : 21 Mar 2019
 *  Updated on : 04 Apr 2019
 *  Author     : Jeong Bin Lee & Uldis Bagley
 *  Processor   : MK70FN1M0VMJ12
 *
 */
/*!
**  @addtogroup packet_module packet module documentation
**  @{
*/

#include "packet.h"
#include "UART.h"


//Global variable declaration
uint16union_t TowerNumParam;

uint8_t packetComplete;

/*! @brief Initializes the packets by calling the initialization routines of the supporting software modules.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz.
 *
 *  @return bool - TRUE if the packet module was successfully initialized.
 */
bool Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  //Initiate UART_Init with same baud rate and module clock
  return UART_Init(baudRate, moduleClk);
}


/*! @brief Attempts to get a packet from the received data.
 *         commands in response to sent packets:
 *
 *  @return bool - TRUE if a valid packet was received.
 */
bool Packet_Get(void)
{
  //get and store value into Packet_Checksum as long as RxFIFOGet has data
  while(UART_InChar(&Packet_Checksum))
  {
    //Check the packet checksum
    if(Packet_Checksum == (Packet_Command^Packet_Parameter1^Packet_Parameter2^Packet_Parameter3))
	return true;

    //Shift down the packet and re-attempt to build packet
    Packet_Command = Packet_Parameter1;
    Packet_Parameter1 = Packet_Parameter2;
    Packet_Parameter2 = Packet_Parameter3;
    Packet_Parameter3 = Packet_Checksum;
  }
  return false;
}


/*! @brief Builds a packet and places it in the transmit FIFO buffer.
 *         sends commands to tower:
 *
 * @param command - initial byte of the packet.
 * @param parameter1 - first byte, parameter1 of the packet.
 * @param parameter2 - second byte, parameter2 of the packet.
 * @param parameter3 - third byte, parameter3 of the packet.
 *
 *  @return bool - TRUE if a valid packet was sent.
 */
bool Packet_Put(const uint8_t command, const uint8_t parameter1,
                const uint8_t parameter2, const uint8_t parameter3)
{
  //go through each parameter TxFIFO_Put and store the values in to TXFIFO
  if(!UART_OutChar(command))
    return false;
  if(!UART_OutChar(parameter1))
    return false;
  if(!UART_OutChar(parameter2))
    return false;
  if(!UART_OutChar(parameter3))
    return false;
  if(!UART_OutChar(command^parameter1^parameter2^parameter3))
    return false;
  return true;
}
/*!
** @}
*/

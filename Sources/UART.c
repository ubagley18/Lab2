/* ###################################################################
**     Filename    : UART.c
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
/*!
 *  @file UART.c
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  @author Jeong Bin Lee & Uldis Bagley
 *  @date 21 Mar 2019
 *
 */
/*!
**  @addtogroup UART_module UART module documentation
**  @{
*/

#include "UART.h"
#include "MK70F12.h"
#include "FIFO.h"
#include "Cpu.h"

//Transmitter is driven by baud rate clock divided by 16
//Receiver has aquisition rate of 16 samples per bit time
static const uint8_t SAMPLE_DATA_RATE = 16;
//Baud Rate Fractional Divisor for baud rate of 38400 with 0.047% error
static const uint8_t BAUD_RATE_DIVISOR = 32;



//Globally declared transmit and receive FIFO
static TFIFO TxFIFO, //Put from packet and Get into UART output by setting TDRE
             RxFIFO; //When RDRF is set Put and Get from RxFIFO



bool UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  //13 bit salve baud rate p.1916
  uint16union_t brSlave;
  //baud rate fine adjust to add more timing resolution
  uint8_t brAdjust;

  //System Clock Gating Control
  //Register 4 Enable UART2 module p.352
  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
  //Register 5 Enable PortE module p.354
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

  //Enables UART2_RX see p.287
  PORTE_PCR16 = PORT_PCR_MUX(3);
  //Enables UART2_TX see p.287
  PORTE_PCR17 = PORT_PCR_MUX(3);

  //If baud rate = 0 since you cant divide by 0 return false
  if (baudRate == 0)
    return false;

  //SBR calculation
  brSlave.l = (moduleClk / (baudRate * SAMPLE_DATA_RATE));
  //BRFA calculation (modulo only returns numerator, therefore divide by the denominator)
  brAdjust  = (((moduleClk % (baudRate * SAMPLE_DATA_RATE))*BAUD_RATE_DIVISOR) / (baudRate * SAMPLE_DATA_RATE));


  //Sends baud rate fine adjust to UART control register 4 to add timing resolution
  UART2_C4 = UART_C4_BRFA(brAdjust);

  //Bus Data High & Low contains operand before memory write cycle or after memory read cycle.
  UART2_BDH |= UART_BDH_SBR(brSlave.s.Hi); //First 5 bit of SBR
  UART2_BDL |= UART_BDL_SBR(brSlave.s.Lo); //Last 8 bit of SBR

  //Enable UART Control Register 2 Transmitter and Receiver
  //Note |= (OR Equals) only changes the MASK bits and does not disturb the register
  UART2_C2 |= UART_C2_TE_MASK | UART_C2_RE_MASK;

  //8N1: enables 8 data bit, no parity bit and one stop bit p.1917
  UART2_C1 &= ~UART_C1_M_MASK;

  //Initialise TxFIFO and RxFIFO
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);
}

bool UART_InChar(uint8_t* const dataPtr)
{
  //call FIFO_Get function and return true if successful and false if unsuccessful
  return FIFO_Get(&RxFIFO, dataPtr);
}


bool UART_OutChar(const uint8_t data)
{
  //call FIFO_Put function and return true if successful and false if unsuccessful
  return FIFO_Put(&TxFIFO, data);
}


void UART_Poll (void)
{
  //Check if buffer is full
  if ((UART2_S1 & UART_S1_RDRF_MASK))
    //Put data in RxFIFO to input by putting data into serial port
    FIFO_Put(&RxFIFO, UART2_D);

  //Check if buffer is empty
  if ((UART2_S1 & UART_S1_TDRE_MASK))
    //Get data from TxFIFO to output by getting data to serial port
    FIFO_Get(&TxFIFO, (uint8_t*) &UART2_D);
}

/*!
** @}
*/

/* ###################################################################
**     Filename    : LEDs.c
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
 *  @file LEDs.c
 *
 *  @brief Routines to access the LEDs on the TWR-K70F120M.
 *
 *  @author Jeong Bin Lee & Uldis Bagley
 *  @date 21 Mar 2019
 *
 */
/*!
**  @addtogroup LEDs_module LEDs module documentation
**  @{
*/

#include "LEDs.h"
#include "MK70F12.h"

bool LEDs_Init(void)
{
  //Initialise PortA
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  //Initialise LED PORTA pins with Alt1 (see tower schematics & p.289)
  //Also initialise Drive Strength
  PORTA_PCR10 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
  PORTA_PCR11 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
  PORTA_PCR28 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
  PORTA_PCR29 |= PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;

  //initialise the LEDs to be off
  GPIOA_PSOR |= LED_ORANGE | LED_YELLOW |LED_GREEN | LED_BLUE;

  //Set GPIO Port Data Direct Register as output
  GPIOA_PDDR |= LED_ORANGE;
  GPIOA_PDDR |= LED_YELLOW;
  GPIOA_PDDR |= LED_GREEN;
  GPIOA_PDDR |= LED_BLUE;

  return true;
}


void LEDs_On(const TLED color)
{
  //setting general input output register turns LED on
  //Turn on LED
  GPIOA_PCOR = color;
}

void LEDs_Off(const TLED color)
{
  //setting general input output register turns LED off
  //Turn off LED
  GPIOA_PSOR = color;
}

void LEDs_Toggle(const TLED color)
{
  //Toggle LED
  GPIOA_PTOR = color;
}
/*!
** @}
*/

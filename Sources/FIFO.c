/* ###################################################################
**     Filename    : FIFO.c
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
 *  @file FIFO.c
 *
 *  @brief Put and get one character using circular FIFO method
 *         Initialise, store and read data using circular first in first out method.
 *
 *  @author Jeong Bin Lee & Uldis Bagley
 *  @date 21 Mar 2019
 *
 */
/*!
**  @addtogroup FIFO_module FIFO module documentation
**  @{
*/


#include "FIFO.h"


bool FIFO_Init(TFIFO * const fifo)
{
  //Initialise everything to 0
  fifo->NbBytes = 0;
  fifo->Start = 0;
  fifo->End = 0;
  return true;
}


bool FIFO_Put(TFIFO * const fifo, const uint8_t data)
{
  //Check if FIFO is full
  if (fifo->NbBytes == FIFO_SIZE)
    return false; // FIFO is full return false
  else
  {
    fifo->Buffer[fifo->End] = data; //store data into buffer
    fifo->End++; //go to next buffer location
    fifo->NbBytes++;

    if (fifo->End == FIFO_SIZE-1)
      fifo->End = 0; // Wrap around the Buffer

    return true; //return true after successful FIFO put
  }
}



bool FIFO_Get(TFIFO* const fifo, uint8_t* const dataPtr)
{
  //check if FIFO is empty
  if (fifo->NbBytes == 0)
    return false; //Buffer is empty return false

  *dataPtr = fifo->Buffer[fifo->Start]; //Store data into dataPtr value
  fifo->Start++; //go to next buffer location
  fifo->NbBytes--; //reduce number of bytes in FIFO after get

  if (fifo->Start == FIFO_SIZE-1)
    fifo->Start = 0; // Warp around the Buffer.
  return true; // return true after successful get from buffer

}
/*!
** @}
*/

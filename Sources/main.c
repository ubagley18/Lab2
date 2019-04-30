/* ###################################################################
**     Filename    : main.c
**     Project     : Lab2
**     Processor   : MK70FN1M0VMJ12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-07-20, 13:27, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 2.0
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


// CPU module - contains low level hardware initialization routines
#include "Cpu.h"
#include "Events.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "types.h"
#include "Flash.h"
#include "LEDs.h"
#include "UART.h"
#include "FIFO.h"
#include "packet.h"

//Command constants
#define TOWER_STARTUP      0x04
#define TOWER_PROGRAM_BYTE 0x07
#define TOWER_READ_BYTE    0x08
#define TOWER_VERSION      0x09
#define TOWER_NUMBER       0x0B
#define TOWER_MODE         0x0D

//Acknowledgment command constants
#define ACK_TOWER_STARTUP      0x84
#define ACK_TOWER_PROGRAM_BYTE 0x87
#define ACK_TOWER_READ_BYTE    0x88
#define ACK_TOWER_VERSION      0x89
#define ACK_TOWER_NUMBER       0x8B
#define ACK_TOWER_MODE         0x8D

//GLOBAL VARIABLES
//Constant variable of of packet acknowledgment mask
const uint8_t PACKET_ACK_MASK = (0x80);
static volatile uint16union_t* NvTowerNb; //non-volatile tower number
static volatile uint16union_t* NvTowerMode; //non-volatile tower mode
TPacket Packet;

//PRIVATE FUNCTIONS
/*! @brief Handles packets by checking command byte and responding accordingly
 *
 *  @param void
 *  @return bool - true if handling was successful, if not false
 */
static bool PacketHandler(void);

/*! @brief Puts Tower Startup packets into TxFIFO
 *
 *  @param requestACK - true if ACK was requested, if not false
 *  @return bool - true if handling was successful, if not false
 */
static bool TowerStartup(bool requestACK);

/*! @brief Write a byte into Flash
 *
 *  @param requestACK - true if ACK was requested, if not false
 *  @return bool - true if handling was successful, if not false
 */
static bool TowerProgramByte(bool requestACK);

/*! @brief read a byte into flash
 *
 *  @param requestACK - true if ACK was requested, if not false
 *  @return bool - true if handling was successful, if not false
 */
static bool TowerReadByte(bool requestACK);

/*! @brief Puts Tower Version packet into TxFIFO
 *
 *  @param requestACK - true if ACK was requested, if not false
 *  @param command -
 *  @return bool - true if handling was successful, if not false
 */
static bool TowerVersion(bool requestACK);

/*! @brief Puts Tower Number packet into TxFIFO and decides whether to set or get tower number
 *
 *  @param requestACK - true if ACK was requested, if not false
 *  @return bool - true if handling was successful, if not false
 */
static bool TowerNumber(bool requestACK);

/*! @brief Put Tower Mode packet into TxFIFO as set or get
 *
 *  @param requestACK - true if ACK was requested, if not false
 *  @return bool - true if handling was successful, if not false
 */
static bool TowerMode(bool requestACK);



static bool TowerStartup(bool requestACK)
{
  //check Parameter 1 and acknowledgement request
  if ((Packet_Parameter1 == 0) && (requestACK == false))
  {
    //Put tower startup packet values without ACK into TxFIFO
    Packet_Put(TOWER_STARTUP, 0, 0, 0);
    Packet_Put(TOWER_VERSION, 'v', 1, 0);
    Packet_Put(TOWER_NUMBER, 1, NvTowerNb->s.Lo, NvTowerNb->s.Hi);
    Packet_Put(TOWER_MODE, 1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);
    return true;
  }
  //check Parameter 1 and acknowledgement request
  if ((Packet_Parameter1 == 0) && (requestACK == true))
  {
    Packet_Put(TOWER_STARTUP, 0, 0, 0);
    Packet_Put(TOWER_VERSION, 'v', 1, 0);
    Packet_Put(TOWER_NUMBER, 1, NvTowerNb->s.Lo, NvTowerNb->s.Hi);
    Packet_Put(TOWER_MODE, 1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);
    //Put tower startup packet values with ACK requested into TxFIFO
    Packet_Put(ACK_TOWER_STARTUP, 0, 0, 0);
    return true;
  }
  return false;
}


static bool TowerProgramByte(bool requestACK)
{
  bool success = false;

  //check offset, if equal to 8 erase flash, if less than 8 write to address+offset, if greater than 8 or less than 0 error.
  if (Packet_Parameter1 > 0x08)
    success = false;
  else if (Packet_Parameter1 == 0x08)
    success = Flash_Erase();
  else
  {
    //assign offset to the address
    volatile uint8_t *address = (uint8_t*)(FLASH_DATA_START + Packet_Parameter1);
    success = Flash_Write8(address, Packet_Parameter3);
  }

  return success;
}

static bool TowerReadByte(bool requestACK)
{
  //check the range of offset, return false if offset is out of range
  if (Packet_Parameter1 < 0x00 || Packet_Parameter1 > 0x07)
    return false;
  else if (requestACK == true)
    return Packet_Put(ACK_TOWER_READ_BYTE, Packet_Parameter1, 0, _FB(FLASH_DATA_START+Packet_Parameter1));
  else if (requestACK == false)
    return Packet_Put(TOWER_READ_BYTE, Packet_Parameter1, 0, _FB(FLASH_DATA_START+Packet_Parameter1));
}

static bool TowerVersion(bool requestACK)
{
  //check for valid parameter 1
  if (Packet_Parameter1 == 'v')
  {
    //check for acknowledgment request
    if (requestACK == false)
      return Packet_Put(TOWER_VERSION, 'v', 1, 0);
    else
      return Packet_Put(ACK_TOWER_VERSION, 'v', 1, 0);
  }
  else if (Packet_Parameter1 != 'v' && requestACK == true)
    return Packet_Put(TOWER_VERSION, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
  else
    return false;
}


static bool TowerNumber(bool requestACK)
{
  switch (Packet_Parameter1)
  {
    case (1): //if Packet_Parameter1 == get
    {
      //check for acknowledgment request
      if (requestACK == true)
        return Packet_Put(ACK_TOWER_NUMBER, 1, NvTowerNb->s.Lo, NvTowerNb->s.Hi); //Sets Tower number to 0
      else
        return Packet_Put(TOWER_NUMBER, 1, NvTowerNb->s.Lo, NvTowerNb->s.Hi); //Sets Tower number to 0
    }
    case (2): //if Packet_Parameter1 == set
    {
      //write tower number parameter23 to flash
      Flash_Write16((volatile uint16_t*)NvTowerNb, Packet_Parameter23);
      //check for acknowledgment request
      if (requestACK == true)
        return Packet_Put(ACK_TOWER_NUMBER, 2, NvTowerNb->s.Lo, NvTowerNb->s.Hi); //Sets Tower number to 4 specified digits
      else
        return Packet_Put(TOWER_NUMBER, 2, NvTowerNb->s.Lo, NvTowerNb->s.Hi); //Sets Tower number to 4 specified digits
    }
    default: //if Packet_Parameter1 does not equal to 1 or 2
    {
      if (requestACK == true)
        return Packet_Put(TOWER_NUMBER, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    }
  }
}


static bool TowerMode(bool requestACK)
{
  switch (Packet_Parameter1)
  {
    case (1): //if Packet_Parameter1 == get
    {
      //check for acknowledgment request
      if (requestACK == true)
        return Packet_Put(ACK_TOWER_MODE, Packet_Parameter1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);
      else
        return Packet_Put(TOWER_MODE, Packet_Parameter1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);
    }
    case (2): //if Packet_Parameter1 == set
    {
      //write tower mode parameter23 to flash
      Flash_Write16((volatile uint16_t*)NvTowerMode, Packet_Parameter23);
      //check for acknowledgment request
      if (requestACK == true)
        return Packet_Put(ACK_TOWER_MODE, Packet_Parameter1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);
      else
        return Packet_Put(TOWER_MODE, Packet_Parameter1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);
    }
    default: //if Packet_Parameter1 does not equal to 1 or 2
    {
      if (requestACK == true)
        return Packet_Put(TOWER_MODE, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    }
  }
}

static bool PacketHandler(void)
{
  //Initialise local variable, requestACK
  bool requestACK;
  //record of successes in the fuctions
  bool success;
  uint8_t packetCommand = (Packet_Command & ~PACKET_ACK_MASK);


  //Check if ACK is requested
  if ((Packet_Command & PACKET_ACK_MASK) == PACKET_ACK_MASK)
    requestACK = true;
  else
    requestACK = false;

  //Compares given command with initialised command to execute the function
  switch (packetCommand)
  {
    case (TOWER_STARTUP):
      success = TowerStartup(requestACK);
    case (TOWER_PROGRAM_BYTE):
      success = TowerProgramByte(requestACK);
    case (TOWER_READ_BYTE):
      success = TowerReadByte(requestACK);
    case (TOWER_VERSION):
      success = TowerVersion(requestACK);
    case (TOWER_NUMBER):
      success = TowerNumber(requestACK);
    case (TOWER_MODE):
      success = TowerMode(requestACK);
  }

  //after executing the function of command, check for acknowledgment request
  if (requestACK == true)
  {
    if(success == true)
      return Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    else
      return Packet_Put(packetCommand, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
  }
  else
    return success;
}


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */
  uint32_t baudRate = 115200; //initiate baudrate to be 115200
  uint32_t moduleClk = CPU_BUS_CLK_HZ; //use the bus clock to initiate UART after changing processor expert
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  //initialise packet, LEDs and flash
  bool packetInit = Packet_Init(baudRate, moduleClk);
  bool initLED = LEDs_Init();
  bool flashInit = Flash_Init();

  //check if initialise is successful
  if (packetInit && initLED && flashInit)
  {
    TLED colour = LED_ORANGE;
    //turn orange LED on
    LEDs_On(colour);
  }

  //allocate flash memory to variables NvTowerNb and NvTowerMode
  if (Flash_AllocateVar((volatile void**)&NvTowerNb, sizeof(*NvTowerNb)))
    //initiate tower number with last 4 digit of student number: 12935084
    Flash_Write16((volatile uint16_t *)NvTowerNb, 5084);

  if (Flash_AllocateVar((volatile void**)&NvTowerMode, sizeof(*NvTowerMode)))
    //initiate tower mode with 1
    Flash_Write16((volatile uint16_t *)NvTowerMode, 1);

  //call the initial four packet_put
  Packet_Put(TOWER_STARTUP, 0, 0, 0);
  Packet_Put(TOWER_VERSION, 'v', 1, 0);
  Packet_Put(TOWER_NUMBER, 1, NvTowerNb->s.Lo, NvTowerNb->s.Hi);
  Packet_Put(TOWER_MODE, 1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);

  /* Write your code here */
  for (;;)
  {
    if(Packet_Get())
      PacketHandler();

    UART_Poll();
  }

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/

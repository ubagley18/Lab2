/* ###################################################################
**     Filename    : Flash.c
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
 *  @file Flash.c
 *
 *  @brief Allocate variable memory, erase and write to flash memory
 *
 *  @author Jeong Bin Lee & Uldis Bagley
 *  @date 21 Mar 2019
 *
 */
/*!
**  @addtogroup Flash_module Flash module documentation
**  @{
*/

#include "Flash.h"
#include "MK70F12.h"

//Declare structure union to store FCCOB register data
typedef struct
{
  //command of FCCOB register
  uint8_t command;
  union
  {
    //combined address in FCCOB register
    uint32_t combined;
    struct
    {
      //decending order for Big-Endian
      uint8_t address3;
      uint8_t address2;
      uint8_t address1;
    }s;
  }address;
  union
  {
    //combined data byte in FCCOB register
    uint64_t combined;
    struct
    {
      uint8_t byte0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t byte3;
      uint8_t byte4;
      uint8_t byte5;
      uint8_t byte6;
      uint8_t byte7;
    }s;
  }byte;
} TFCCOB;


//GLOBAL VARIABLE
//representing the allocated flash memory
static uint8_t FlashMemory = 0xFF;
//command to erase the sector
static const uint8_t FlashEraseSector = 0x09;
//command to write to a phrase to the sector
static const uint8_t FlashWritePhrase = 0x07;


//PRIVATE FUNCTIONS
/*! @brief Waits until Command Complete Interrupt Flag file system operation
 *         has completed.
 */
static void WaitCCIF(void);

/*! @brief Acts as a buffer between FCCOB register and the program structure.
 *
 *  @param commonCommandObject Structure variable to access the structure
 *  @return bool - TRUE if CCIF system operation has completed, FALSE if CCIF system operation could not complete.
 */
static bool LaunchCommand(TFCCOB* commonCommandObject);

/*! @brief Erases the sector of given address.
 *
 *  @param address The address to be erased
 *  @return bool - TRUE if the phrase was erased successfully, FLASE if error in the code
 */
static bool EraseSector(const uint32_t address);

/*! @brief Writes phrase into the the given address.
 *
 *  @param address The address of the data
 *  @param pharase The 32-bit data to write
 *  @return bool - TRUE if phrase was written successfully, FALSE if the CCIF could not complete system operation
 */
static bool WritePhrase(const uint32_t address, const uint64union_t phrase);

/*! @brief Modifies the current phrase by erasing the address sector then write phrase into the address
 *
 *  @param address The address of the data
 *  @param pharase The 64-bit data to write
 *  @return bool - TRUE if erase and write was successful, FALSE if unsuccessful
 */
static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase);



bool Flash_Init(void)
{
  //nothing to initialise for flash?
  return true;
}


bool Flash_AllocateVar(volatile void** variable, const uint8_t size)
{
  static uint8_t memoryAlloc; //memory mask to allocate memory position
  static int variableAddress; //temporary variable address

  //check the size of the variable
  switch (size)
  {
    //assign potential memory depending on the size
    case 1:
      memoryAlloc = 0x01; // 0000 0001
      break;
    case 2:
      memoryAlloc = 0x03; // 0000 0011
      break;
    case 4:
      memoryAlloc = 0x0F; // 0000 1111
      break;
  }

  //loop through the address incrementing by variable size to find an unused memory
  for (variableAddress = FLASH_DATA_START; variableAddress <= FLASH_DATA_END; variableAddress += size)
  {
    //check if the flash is empty
    if (memoryAlloc == (FlashMemory & memoryAlloc))
    {
      //write the selected address to the address of the memory
      *variable = (void *) variableAddress;
      //change the current memory as used
      FlashMemory = (FlashMemory ^ memoryAlloc);
      return true;
    }
    //shift down the memoryAlloc until Flash Memory is empty
    memoryAlloc = memoryAlloc << size;
  }
  return false;
}

static void WaitCCIF(void)
{
  while (!(FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK)); //waits for CCIF to be set to 1 so that a command can be executed
}

static bool LaunchCommand(TFCCOB* commonCommandObject)
{

  //Wait for system operation to complete
  //check CCIF flag
  WaitCCIF();

  //check FSTAT register flags and set them if not already
  if (FTFE_FSTAT & FTFE_FSTAT_FPVIOL_MASK == FTFE_FSTAT_FPVIOL_MASK)
    FTFE_FSTAT = FTFE_FSTAT_FPVIOL_MASK; //set 1 to clear
  if (FTFE_FSTAT & FTFE_FSTAT_ACCERR_MASK == FTFE_FSTAT_ACCERR_MASK)
    FTFE_FSTAT = FTFE_FSTAT_ACCERR_MASK; //set 1 to clear
  if (FTFE_FSTAT & FTFE_FSTAT_RDCOLERR_MASK == FTFE_FSTAT_RDCOLERR_MASK)
    FTFE_FSTAT = FTFE_FSTAT_RDCOLERR_MASK; //set 1 to clear

  //see p813 for flowchart, p796 for FCCOB number, p789 for Big-Endian order
  //store the structure into FCCOB register using Big-Endian
  FTFE_FCCOB0 = commonCommandObject->command;
  FTFE_FCCOB1 = commonCommandObject->address.s.address1;
  FTFE_FCCOB2 = commonCommandObject->address.s.address2;
  FTFE_FCCOB3 = commonCommandObject->address.s.address3;
  //using Big-Endian to store into FCCOB
  FTFE_FCCOB4 = commonCommandObject->byte.s.byte3;
  FTFE_FCCOB5 = commonCommandObject->byte.s.byte2;
  FTFE_FCCOB6 = commonCommandObject->byte.s.byte1;
  FTFE_FCCOB7 = commonCommandObject->byte.s.byte0;
  //using Big-Endian to store into FCCOB
  FTFE_FCCOB8 = commonCommandObject->byte.s.byte7;
  FTFE_FCCOB9 = commonCommandObject->byte.s.byte6;
  FTFE_FCCOBA = commonCommandObject->byte.s.byte5;
  FTFE_FCCOBB = commonCommandObject->byte.s.byte4;

  //return true after CIFF flag is set
  FTFE_FSTAT|= FTFE_FSTAT_CCIF_MASK;

  //wait for CCIF flag to be set
  WaitCCIF();

  return true;
}

static bool EraseSector(const uint32_t address)
{
  TFCCOB erase;
  //assign erase command
  erase.command = FlashEraseSector;
  //assign address to be erased
  erase.address.combined = address;
  return LaunchCommand(&erase);
}

bool Flash_Erase(void)
{
  //initialise sector as start of flash memory
  uint32_t sectorErase = FLASH_DATA_START;
  return EraseSector(sectorErase);
}

static bool WritePhrase(const uint32_t address, const uint64union_t phrase)
{
  TFCCOB write;
  //assign write phrase command
  write.command = FlashWritePhrase;
  //assign address to be written to
  write.address.combined = address;
  //assign given phrase to the structure
  write.byte.combined = phrase.l;

  return LaunchCommand(&write);
}

static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase)
{
  //first erase flash, then write entire phrase
  return Flash_Erase() && WritePhrase(address, phrase);
}

bool Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{
  uint64union_t phrase; //phrase union 64bit
  uint32_t newAddress = (uint32_t)address; //declare new address as 32bit

  //check if the address is aligned with phrase
  if (((newAddress/4) % 2) == 0)
  {
    phrase.s.Lo = data; //store data into low 32-bit of union
    phrase.s.Hi = _FW(newAddress+4); //read from flash into high 32-bit of union
    return ModifyPhrase(newAddress, phrase); //modify phrase to erase and write
  }
  else
  {
    phrase.s.Lo = _FW(newAddress-4); //read from flash into low 32-bit of union
    phrase.s.Hi = data; //store data into high 32-bit of union
    return ModifyPhrase(newAddress-4, phrase); //modify phrase to erase and write
  }
}

bool Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{
  uint32union_t word; //word union 32bit
  uint32_t newAddress = (uint32_t)address; //declare new address as 32bit

  //check if the address is aligned with word
  if((newAddress % 4) == 0)
  {
    word.s.Lo = data; //store data into low 16-bit of union
    word.s.Hi = _FH(newAddress+2); //read from flash into high 16-bit of union
    return Flash_Write32(&(_FW(newAddress)), word.l); //write to 32bit flash
  }
  else
  {
    word.s.Lo = _FH(newAddress-2); //read from flash into low 16-bit of union
    word.s.Hi = data; //store data into high 16-bit of union
    return Flash_Write32(&(_FW(newAddress-2)), word.l); //write to 32bit flash
  }
}

bool Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{
  uint16union_t halfword; //halfword union 16bit
  uint32_t newAddress = (uint32_t)address; //declare new address as 32bit

  //check if the address is aligned with halfword
  if ((newAddress % 2) == 0)
  {
    halfword.s.Lo = data; //store data into low byte of union
    halfword.s.Hi = _FB(newAddress+1); //read from flash into high byte of union
    return Flash_Write16(&(_FH(newAddress)), halfword.l); //write to 16bit flash
  }
  else
  {
    halfword.s.Lo = _FB(newAddress-1); //read from flash into low byte of union
    halfword.s.Hi = data;  //store data into high byte of union
    return Flash_Write16(&(_FH(newAddress-1)),halfword.l); //write to 16bit flash
  }
}

/*!
** @}
*/

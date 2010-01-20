//---------------------
//  Copyright (C) 2000-2009  <Laurent SAINT-MARCEL>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
//
//  you can write to me at <lstmarcel@yahoo.fr>
//

// the driver is intended for SDCARD
//


#ifndef __SDCARD_H__
# define __SDCARD_H__

# include "type_def.h"

// command the following lines if you just want read/write sdCard functions
// (to save program disk space)
//#define SD_CARD_REGISTER_COMMANDS

#define SD_CARD_BLOCK_SIZE  512 // a block size of the SDCard. Always 512 bytes
#define SD_CARD_MAX_BLOCK   0x300000 // maximum number of blocks for a 2Go SDCard = Capacity / SD_CARD_BLOCK_SIZE = 0x400000

typedef u32 sdCardAddr_t;

// initialization of the SDCard component (setup atmel SPI port)
extern u8 SD_init(void* pt);

#if 1 // TODO: remove this commands and use SDMGR instead
// read len byte(s) from SDCARD address addr and copy them in data
extern u8 SD_read(u16 addr, u8* data, u8 len);

// write len byte(s) to SDCARD address addr from data
extern u8 SD_write(u16 addr, u8* data, u8 len);

// check if SDCARD write has ended
extern u8 SD_is_fini(void);
#endif

/**
 * @brief This function returns the SD error code after a SD_XXX function return KO
 */
extern u16 SD_get_error();

/**
 * @brief Detect and initialize the SDcard
 *
 * This function detects and initializes the SDcard
 * @return true  if the card is detected
 * @return false if the card is not detected. Use sdGetError to get the error code.
 */
extern u8 SD_init_card();

/**
 * @brief Writes SD_CARD_BLOCK_SIZE bytes in the sdcard at the specified blockId
 *
 * This function writes SD_CARD_BLOCK_SIZE bytes in the sdcard at the 
 * specified blockId. At 8MHz it takes around 5.3ms ~= 85ko/s
 *
 * @param blockId the number of the block to write:
 *                0, 1, ... cardCapacity/SD_CARD_BLOCK_SIZE
 * @param buffer an array of at least SD_CARD_BLOCK_SIZE bytes containing the
 *               data to write in the sdCard
 * @return true  if the block has been written in the sdCard
 * @return false if the block has not been written completely. Use sdGetError to get the error code.
 */
extern u8 SD_write_block(sdCardAddr_t blockId,
                         u8* buffer);

/**
 * @brief Read SD_CARD_BLOCK_SIZE bytes from the sdcard at the specified blockId
 *
 * This function reads SD_CARD_BLOCK_SIZE bytes in the sdcard at the 
 * specified blockId. At 8MHz it takes around 2.8ms
 *
 * @param blockId the number of the block to read: 0, 1, ... cardCapacity/SD_CARD_BLOCK_SIZE
 * @param buffer an array of at least SD_CARD_BLOCK_SIZE bytes containing the
 *               data to write in the sdCard
 * @return true  if the block has been read from the sdCard
 * @return false if the block has not been read completely. Use sdGetError to get the error code.
 */
extern u8 SD_read_block(sdCardAddr_t blockId,
                        u8* buffer);

// ===========================================================================
// Functions to get sd card registers
// ===========================================================================

#ifdef SD_CARD_REGISTER_COMMANDS

/** 
 * @brief Retrieve the 16 bytes Card Specific Data register
 *
 * This function retrieves the 16 bytes Card Specific Data register
 *
 * @param buffer an array of at least 16 bytes that will be fill with the 
 *               register data
 * @return true on success, false in the other case
 */
extern u8 SD_get_CSD(u8* buffer);

/** 
 * @brief Retrieve the 16 bytes Card Identification register
 *
 * This function retrieves the 16 bytes Card Identification register
 *
 * @param buffer an array of at least 16 bytes that will be fill with the 
 *               register data
 * @return true on success, false in the other case
 */
extern u8 SD_get_CID(u8* buffer);

/** 
 * @brief Retrieve the 4 bytes Operating Conditions Register
 *
 * This function retrieves the 4 bytes Card Identification register
 *
 * @param buffer an array of at least 4 bytes that will be fill with the 
 *               register data
 * @return true on success, false in the other case
 */
extern u8 SD_get_OCR(u8* buffer);

/** 
 * @brief Retrieve the 2 bytes Status Register
 *
 * This function retrieves the 2 bytes Card Identification register
 *
 * @param buffer an array of at least 2 bytes that will be fill with the 
 *               register data
 * @return true on success, false in the other case
 */
extern u8 SD_get_status(u8* buffer);

#endif // SD_CARD_REGISTER_COMMANDS



#endif	// __SDCARD_H__

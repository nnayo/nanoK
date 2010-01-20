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


#ifndef __SDCARD_MGR_H__
# define __SDCARD_MGR_H__

# include "type_def.h"

// --------------------------------- INIT --------------------------------------

// initialization of the SDCard component)
extern void SDMGR_init();

// run the sd card manager thread
extern void SDMGR_run();

// ------------------------------- STATUS --------------------------------------

// return OK if the sd card is detected and the communication is OK.
// if KO, the programm will automatically try to detect the card every seconds
extern u8   SDMGR_is_sdcard_detected();

// return OK if the MCU is currently communicating with the SD card (when it
// performs read/write operations)
extern u8   SDMGR_is_sdcard_communicating();

// return OK if all internal buffers are full and SDMGR_write will return KO
// if SDMGR_is_sdcard_detected() is OK then this flag will change to KO when 
// the data will be written to the sdcard
extern u8   SDMGR_are_buffers_full();

// -------------------------------- WRITE --------------------------------------

// set the block address for writing data in the sdcard (it is automatically 
// incremented when the internal buffers are flushed to the sdcard)
// This function is usefull only during the initialization
extern void SDMGR_set_block_addr(u32 addr);

// return the address of the block that will be used for writing the next data
extern u32  SDMGR_get_block_addr();

// Write len bytes of data in the flash (the data are temporary stored in an
// internal buffer of SD_CARD_BLOCK_SIZE before being written to the sdcard
// when the buffer is full or when SDMGR_flush is called
// return OK on success, KO if all buffers are full and the data cannot be saved
extern u8   SDMGR_write(u8* data, u8 len); 

// force writing the current data in the sdcard even if they don't reach the 
// sd card block size. This increment the SDMGR.blockAddr
extern void SDMGR_flush();

// return true when flush has written all its data to the sdcard
extern u8   SDMGR_flush_is_fini();

// -------------------------------- READ --------------------------------------

// read SD_CARD_BLOCK_SIZE bytes (512 bytes) at the specified block address.
// data is updated with a pointer to a buffer of 512 bytes.
// WARNING this function clear the write buffer, it should not be call while
// there are data to write in the sdcard (you can call SDMGR_flush before)
// return OK
extern u8   SDMGR_read(u32 addr, u8** data); 

// return OK when the read command has finished and data of **data are valid
// for read
extern u8   SDMGR_read_is_fini();


extern void SDMGR_sync_buffers();

#endif	// __SDCARD_MGR_H__

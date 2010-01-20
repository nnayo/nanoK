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

// Manage a double buffer for writing data of ny size in the sdCard
// instead of just supporting sdCard blocks 
//

#define SDMGR_USE_THREADS

#ifdef SDMGR_USE_THREADS
#include "utils/pt.h"
#endif
#include "externals/sdCard.h"
#include "externals/sdCardMgr.h"



// ===========================================================================
// SDCARD Manager status bits
// ===========================================================================
#define SDMGR_CARD_DETECTED  0x01 // indicates that the sd card has been detect by the MCU
#define SDMGR_SD_COM_ACTIVE  0x02 // indicates that the programm is accessing the sdcard for read/write
#define SDMGR_READ_BLOCK     0x04 // indicates that the user wants to copy the block at addr SDMGR.blockAddr in SDMGR.buffer0
                                  // this flag is reseted when the operation is done
#define SDMGR_BUFFER0_FULL   0x10 // indicates that the data in SDMGR.buffer0 can be flushed in the sdcard
#define SDMGR_BUFFER1_FULL   0x20 // indicates that the data in SDMGR.buffer1 can be flushed in the sdcard

static struct {
#ifdef SDMGR_USE_THREADS
    pt_t        pt;
#endif
    volatile u8 status;     // 8bits status of the manager
    volatile sdCardAddr_t wBlockAddr; // address of the next sdCardBlock that will be written
    volatile sdCardAddr_t rBlockAddr; // address of the sdCardBlock that will be read
    u8          buffer0[SD_CARD_BLOCK_SIZE];
    u8          buffer1[SD_CARD_BLOCK_SIZE]; 
    u8*         buffer; // temporary buffer used to store the data before the are written to the sdcard
    u16         index;  // index in the buffer
    
} SDMGR;

// ----------------------------------------------------------------------------
// SDMGR_is_sdcard_detected
// ----------------------------------------------------------------------------
// Return OK if the sdCard has been detected by the MCU and the communication 
// is OK
// ----------------------------------------------------------------------------
u8 SDMGR_is_sdcard_detected()
{
    return (SDMGR.status & SDMGR_CARD_DETECTED)?OK:KO;
}

// ----------------------------------------------------------------------------
// SDMGR_is_sdcard_communicating
// ----------------------------------------------------------------------------
// Return OK if the MCU is reading/writting in the sdCard
// ----------------------------------------------------------------------------
u8 SDMGR_is_sdcard_communicating()
{
    return (SDMGR.status & SDMGR_SD_COM_ACTIVE)?OK:KO;
}

// ----------------------------------------------------------------------------
// SDMGR_activate_com
// ----------------------------------------------------------------------------
// Set the SDMGR_is_sdcard_communicating state
// Call this function when the communiction with the sd card starts
// ----------------------------------------------------------------------------
static void SDMGR_activate_com()
{
    SDMGR.status |= SDMGR_SD_COM_ACTIVE;
}

// ----------------------------------------------------------------------------
// SDMGR_disactivate_com
// ----------------------------------------------------------------------------
// Reset the SDMGR_is_sdcard_communicating state
// Call this function when the communiction with the sd card is finished
// ----------------------------------------------------------------------------
static void SDMGR_disactivate_com()
{
    SDMGR.status &= ~SDMGR_SD_COM_ACTIVE;
}

// ----------------------------------------------------------------------------
// sdmgr_thread
// ----------------------------------------------------------------------------
// SDMGR threads which perform the read/write operations in the sdCard when
// necessary (read on demand, write when a buffer is full)
// ----------------------------------------------------------------------------
#ifdef SDMGR_USE_THREADS
void SDMGR_sync_buffers() {}

static PT_THREAD( sdmgr_thread(pt_t* pt) )
{
    PT_BEGIN(pt);
#else
void SDMGR_sync_buffers()
{
#endif
    // try to connect to the sd card:
    if ((SDMGR.status & SDMGR_CARD_DETECTED) == 0) {
        SDMGR_activate_com();
        if (SD_init_card() == OK) {
            SDMGR_disactivate_com();
            SDMGR.status |= SDMGR_CARD_DETECTED;
        } else {
            SDMGR_disactivate_com();
            // wait 1s and restart
            //TODO
        }
    }

#ifdef SDMGR_USE_THREADS
    // wait for a buffer to be full
    PT_WAIT_UNTIL(pt, (SDMGR.status & (SDMGR_READ_BLOCK | SDMGR_BUFFER0_FULL | SDMGR_BUFFER1_FULL)) != 0);
#else
	if ((SDMGR.status & (SDMGR_READ_BLOCK | SDMGR_BUFFER0_FULL | SDMGR_BUFFER1_FULL)) == 0) {
		return;
	}
#endif
    // read sdBlock and copy it in buffer0
    if (SDMGR.status & SDMGR_READ_BLOCK) {  
        SDMGR_activate_com();
        if (SD_read_block(SDMGR.rBlockAddr, SDMGR.buffer0) == OK) {
            SDMGR.status &= ~SDMGR_READ_BLOCK;
        }
        SDMGR_disactivate_com();
        // loop back
#ifdef SDMGR_USE_THREADS
        PT_RESTART(pt);
#endif
    }

    // write buffer0 in the sdCard when it is full
    if (SDMGR.status & SDMGR_BUFFER0_FULL) {  
        SDMGR_activate_com();
        if (SD_write_block(SDMGR.wBlockAddr, SDMGR.buffer0) == OK) {
            SDMGR.status &= ~SDMGR_BUFFER0_FULL; // buffer0 is ready for use
        } else {
            SDMGR.status &= ~SDMGR_CARD_DETECTED; // com error: reset the sdCard communication
        }
        ++SDMGR.wBlockAddr; // increment block addr so next time we will write in another block 
        SDMGR_disactivate_com();
    }

    // write buffer1 in the sdCard when it is full
    if (SDMGR.status & SDMGR_BUFFER1_FULL) {  
        SDMGR_activate_com();
        if (SD_write_block(SDMGR.wBlockAddr, SDMGR.buffer1) == OK) {
            SDMGR.status &= ~SDMGR_BUFFER1_FULL; // buffer1 is ready for use
        } else {
            SDMGR.status &= ~SDMGR_CARD_DETECTED; // com error: reset the sdCard communication
        }
        ++SDMGR.wBlockAddr; // increment block addr so next time we will write in another block 
        SDMGR_disactivate_com();
    }
#ifdef SDMGR_USE_THREADS
    // loop back
    PT_RESTART(pt);

    PT_END(pt);
#endif
}

// ----------------------------------------------------------------------------
// SDMGR_init
// ----------------------------------------------------------------------------
// Initialize the SdCard Manager module
// ----------------------------------------------------------------------------
void SDMGR_init()
{
    SDMGR.status     = 0; 

    SDMGR.wBlockAddr = 0;
    SDMGR.rBlockAddr = 0;
    SDMGR.buffer     = SDMGR.buffer0; // start writting in buffer0
    SDMGR.index      = 0;         // index in the buffer

#ifdef SDMGR_USE_THREADS
    PT_INIT(&SDMGR.pt);
    SD_init(&SDMGR.pt);
#else
	SD_init(0);
#endif
}


// ----------------------------------------------------------------------------
// SDMGR_run
// ----------------------------------------------------------------------------
// Run the SdCard Manager thread
// ----------------------------------------------------------------------------
void SDMGR_run()
{
#ifdef SDMGR_USE_THREADS
    (void)PT_SCHEDULE(sdmgr_thread(&SDMGR.pt));
#endif
}

// ----------------------------------------------------------------------------
// SDMGR_set_block_addr
// ----------------------------------------------------------------------------
// Set the next block address to write in
// The block address is automatically incremented when data are flushed in the
// sdCard
// ----------------------------------------------------------------------------
void SDMGR_set_block_addr(sdCardAddr_t addr)
{
    SDMGR.wBlockAddr = addr;
}

// ----------------------------------------------------------------------------
// SDMGR_set_block_addr
// ----------------------------------------------------------------------------
// Return the next block address used by the write command
// ----------------------------------------------------------------------------
sdCardAddr_t SDMGR_get_block_addr()
{
    return SDMGR.wBlockAddr;
}

// ----------------------------------------------------------------------------
// SDMGR_are_buffers_full
// ----------------------------------------------------------------------------
// Return OK if all internal buffers are full (when the mgr cannot write fast
// enough in the sdCard)
// ----------------------------------------------------------------------------
u8 SDMGR_are_buffers_full()
{
    return (SDMGR.status & (SDMGR_BUFFER0_FULL | SDMGR_BUFFER1_FULL)) 
        == (SDMGR_BUFFER0_FULL | SDMGR_BUFFER1_FULL);
}

// ----------------------------------------------------------------------------
// SDMGR_is_buffer0_full
// ----------------------------------------------------------------------------
// Return != 0 if buffer 0 is full and wait for being written in the sdCard
// ----------------------------------------------------------------------------
static u8 SDMGR_is_buffer0_full()
{
     return SDMGR.status & (SDMGR_BUFFER0_FULL);
}

// ----------------------------------------------------------------------------
// SDMGR_is_buffer1_full
// ----------------------------------------------------------------------------
// Return != 0 if buffer 1 is full and wait for being written in the sdCard
// ----------------------------------------------------------------------------
static u8 SDMGR_is_buffer1_full()
{
    return SDMGR.status & (SDMGR_BUFFER1_FULL);
}

// ----------------------------------------------------------------------------
// SDMGR_is_buffer_full
// ----------------------------------------------------------------------------
// Return != 0 if the current write buffer full 
// ----------------------------------------------------------------------------
static u8 SDMGR_is_buffer_full()
{
    if (SDMGR.buffer == SDMGR.buffer0) {
        return SDMGR_is_buffer0_full();
    } else {
        return SDMGR_is_buffer1_full();
    }
}

// ----------------------------------------------------------------------------
// SDMGR_swapBuffers
// ----------------------------------------------------------------------------
// set the IS_FULL flag for the current buffer, reset the SDMGR.index, change 
// the current buffer if one of the double buffer is free.
// Return OK if buffer has moved, KO if all internal buffers are full
// ----------------------------------------------------------------------------
static u8 SDMGR_swapBuffers()
{
    // buffer is full, try using the other buffer
    if (SDMGR.buffer == SDMGR.buffer0) {
        SDMGR.status |= SDMGR_BUFFER0_FULL;
        SDMGR.index = 0;
        if (SDMGR_is_buffer1_full()) {
            return KO; // all buffers are full
        } else {
            SDMGR.buffer = SDMGR.buffer1; 
        }
    } else {
        SDMGR.status |= SDMGR_BUFFER1_FULL;
        SDMGR.index = 0;
        if (SDMGR_is_buffer0_full()) {
            return KO; // all buffers are full
        } else {
            SDMGR.buffer = SDMGR.buffer0; 
        }
    }
    return OK;
}

// ----------------------------------------------------------------------------
// SDMGR_swapBuffers
// ----------------------------------------------------------------------------
// Force writting the current buffer in the sdCard
// ----------------------------------------------------------------------------
void SDMGR_flush()
{
    SDMGR_swapBuffers();
}

// ----------------------------------------------------------------------------
// SDMGR_flush_is_fini
// ----------------------------------------------------------------------------
// Return OK when all internal buffers have been flushed to the sdCard
// ----------------------------------------------------------------------------
u8 SDMGR_flush_is_fini()
{
    return ((SDMGR.status & (SDMGR_BUFFER0_FULL | SDMGR_BUFFER1_FULL)) != 0)?KO:OK;
}

// ----------------------------------------------------------------------------
// SDMGR_write(data[], len)
// ----------------------------------------------------------------------------
// write len byte(s) of data to the SDMGR buffer. They will be flushed in the
// sdCard when the buffer is full (see SD_CARD_BLOCK_SIZE, see SDMGR_set_block_addr)
// return KO if all internal buffers are full and the data cannot be saved
// ----------------------------------------------------------------------------
u8 SDMGR_write(u8* data, u8 len)
{
    if (SDMGR_are_buffers_full()) {
        return KO;
    }
    // one of the internal buffers is free
    if (SDMGR_is_buffer_full()) { 
        // current buffer is full but the other one is free
        // move the the other buffer
        if (SDMGR_swapBuffers() != OK) 
            return KO;
    }
    if ((SDMGR.index + len) > SD_CARD_BLOCK_SIZE) { 
        // not enough free size in the current buffer try to use the other one
        if (SDMGR_swapBuffers() != OK)
            return KO;
    }

    // copy the data in the SDMGR buffer
    u8 i;
    for(i=0; i < len; ++i) {
        SDMGR.buffer[++SDMGR.index] = data[i];
    }

    // flush the buffer immediatly when it is full, don't wait the next write
    if (SDMGR.index >= SD_CARD_BLOCK_SIZE) {
        SDMGR_swapBuffers(); // update the BUFFERX_FULL flag
    }
    return OK;
}

// ----------------------------------------------------------------------------
// SDMGR_read(addr, *data[])
// ----------------------------------------------------------------------------
// Send the read order to the sdCard. It Updates a pointer to a 
// SD_CARD_BLOCK_SIZE buffer containing the data when SDMGR_read_is_fini() == OK
// ----------------------------------------------------------------------------
u8 SDMGR_read(sdCardAddr_t addr, u8** data)
{
    SDMGR.rBlockAddr = addr;
    *data = SDMGR.buffer0;
    SDMGR.status |= SDMGR_READ_BLOCK;
    return OK;
}
// ----------------------------------------------------------------------------
// SDMGR_read_is_fini
// ----------------------------------------------------------------------------
// Return OK when the SDMGR_read command has finished and the data are valid
// ----------------------------------------------------------------------------
u8 SDMGR_read_is_fini()
{
    return (SDMGR.status & SDMGR_READ_BLOCK)?KO:OK;
}


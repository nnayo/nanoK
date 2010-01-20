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

//#define SPI_ASYNCHRONOUS

#include <avr/io.h>
#ifdef SPI_ASYNCHRONOUS
# include "drivers/spi.h"
# include "utils/pt.h"
#endif
#include "externals/sdcard.h"
#include "externals/sdcardProtocol.h"


// SPI port PIN configuration 
// The following configuration is valid for an ATMEGA324
#define SPI_PORT PORTB
#define SPI_DDR  DDRB
#define SPI_SS	    4	// Port B bit 4 (pin5:  chip select for MMC     SS
#define SPI_MOSI    5	// Port B bit 5 (pin6): (data to MMC)         MOSI
#define SPI_MISO    6	// Port B bit 6 (pin7): (data from MMC)       MISO
#define SPI_CSK	    7	// Port B bit 7 (pin8): clock                  CSK

#define SD_WAIT_TIMEOUT   0xC350 // 16 bits timeout

static struct {
    u16          error;
#ifdef SPI_ASYNCHRONOUS
    pt_t*        pt;
    volatile u8  spiState;
#endif
} SDCARD;

// ---------------------------------------------------------------------------
// set chip select to low (sdCard is selected is selected)
// ---------------------------------------------------------------------------
static void SD_select_chip()
{
    SPI_PORT &= ~_BV(SPI_SS);	
}

// ---------------------------------------------------------------------------
// set chip select to low (sdCard is selected is selected)
// ---------------------------------------------------------------------------
static void SD_deselect_chip()
{
    SPI_PORT |= _BV(SPI_SS);
}

#ifdef SPI_ASYNCHRONOUS
void SD_SPI_done_CB(spi_state_t st, void* misc)
{
    if (st == SPI_MASTER_END) {
        SDCARD.spiState = 1; // OK
    } else {
        SDCARD.spiState = 2; // error
    }
}
#endif

// ---------------------------------------------------------------------------
// set slow clock speed during card initialization
// ---------------------------------------------------------------------------
static void SD_set_slow_clock_speed()
{
#ifdef SPI_ASYNCHRONOUS 
    SPI_set_clock(SPI_DIV_64);
#else
    // SPI enabled
    // master mode
    // SPIclock = Fclock/64=124kHz (less than 4OOkHz for Multimedia cards)
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
    SPSR |= _BV(SPI2X);
#endif
}

// ---------------------------------------------------------------------------
// set fast SPI clock speed for card read/write
// ---------------------------------------------------------------------------
static void SD_set_fast_clock_speed()
{
#ifdef SPI_ASYNCHRONOUS
    SPI_set_clock(SPI_DIV_2);
#else
    // SPI enabled
    // master mode
    // SPIclock = Fclock/2 = 4MHz (less than 20MHz)
    SPCR = (1 << SPE) | (1 << MSTR) | (0 << SPR1) | (0 << SPR0);
    SPSR |= _BV(SPI2X);
#endif
}

// ---------------------------------------------------------------------------
// Send a char on the SPI and return the answer
// ---------------------------------------------------------------------------
static u8 SD_SPI(u8 d) 
{  
#ifdef SPI_ASYNCHRONOUS
    // send character over SPI
    SDCARD.spiState=0;
    u8 tx_buf[1];
    tx_buf[0] = d;
    u8 rx_buf[1];
    SPI_master(tx_buf, 1, rx_buf, 1);
    // wait for the answer
    PT_WAIT_WHILE(SDCARD.pt, SDCARD.spiState==0);
    if (SDCARD.spiState == 1) 
        return rx_buf[0];
    else
        return 0xFF; // SPI error
#else
    // send character over SPI
    SPDR = d;
    while(!(SPSR & (1<<SPIF)));
    return SPDR;
#endif
}

// ---------------------------------------------------------------------------
// Send a command to the SD card
// ---------------------------------------------------------------------------
static void SD_command(char cmd, uint32_t Addr, char crc)
{	// sends a command to the MMC
    SD_SPI(0xFF);
    SD_SPI(0x40 | cmd);
    SD_SPI((uint8_t)(Addr >> 24));
    SD_SPI((uint8_t)(Addr >> 16));
    SD_SPI((uint8_t)(Addr >> 8));
    SD_SPI((uint8_t)Addr);
    SD_SPI(crc);
    SD_SPI(0xFF);
}

// ---------------------------------------------------------------------------
// Wait a specific value from the SD card. Return true if the expectedValue
// match, false in the other case
// ---------------------------------------------------------------------------
static u8 SD_wait(char expectedValue)
{
    uint16_t ix;
    char r1 = SD_SPI(0xFF);
    for (ix = 0; ix < SD_WAIT_TIMEOUT; ix++) {
        if (r1 == expectedValue) 
            break;
        r1 = SD_SPI(0xFF);
    }
    SDCARD.error = r1;
    return (r1 == expectedValue) ;
}

// ---------------------------------------------------------------------------
// Macro to deselect the card and return a boolean
// ---------------------------------------------------------------------------
#define SD_RETURN(VALUE)  \
    SD_deselect_chip();  \
	return VALUE;


// ===========================================================================
// PUBLIC FUNCTIONS
// ===========================================================================


// initialization of the SDCARD component
u8 SD_init(void * pt)
{
    SDCARD.error = SD_R1_NOERROR;
    // init spi port

#ifdef SPI_ASYNCHRONOUS
    SDCARD.pt = (pt_t*)pt;
    SPI_init(SPI_MASTER, SPI_ZERO/* SPI_THREE*/, SPI_MSB, SPI_DIV_64);
    SPI_call_back_set(SD_SPI_done_CB, NULL);
#else
    SPI_DDR &= ~_BV(SPI_MISO);   // set port B SPI data (MISO) input to input
    SPI_DDR |= _BV(SPI_CSK);    // set port B SPI clock to output
    SPI_DDR |= _BV(SPI_MOSI);   // set port B SPI data out (MOSI)to output
    SPI_DDR |= _BV(SPI_SS);	  // set port B SPI chip select to output
#endif

    SD_set_slow_clock_speed();
    SD_deselect_chip();

    return OK;
}

// read len byte(s) from SDCARD address addr and copy them in data
u8 SD_read(u16 addr, u8* data, u8 len)
{
    return KO;
}

// write len byte(s) to SDCARD address addr from data
u8 SD_write(u16 addr, u8* data, u8 len)
{
    return KO;
}

// check if SDCARD write has ended
u8 SD_is_fini(void)
{
    return OK;
}

// ===========================================================================
// LOW LEVEL PUBLIC FUNCTIONS
// ===========================================================================

// ---------------------------------------------------------------------------
// Initialize the SD card. Return true if the card is ready 
// for read/write
// ---------------------------------------------------------------------------
u8 SD_init_card(void) 
{ 
    u8 i;
    u16 ix=0;
    SD_set_slow_clock_speed();

    // start SPI mode : 80 clocks pulses while sdcard is not selected
    SD_deselect_chip();
    for(i=0; i < 10; i++) {
        SD_SPI(0xFF); // send 10*8=80 clock pulses
    }

    SD_select_chip();
	// reset the card
	SD_command(SD_CMD0_GO_IDLE_STATE, 0 ,0x95/*CRC7*/);
	i = SD_SPI(0xFF);
    if (i != SD_R1_IDLE) {
        SDCARD.error = i + SD_CARD_ERROR_INIT_1;
        SD_RETURN(KO);
    }

    // wait the end of reset
    ix=0;
    do {
        SD_command(SD_CMD1_SEND_OPCOND, 0, 0xFF/*Dummy CRC*/);
        i = SD_SPI(0xFF);
    } while ((i != SD_R1_NOERROR) && (++ix < SD_WAIT_TIMEOUT));
    if (i != SD_R1_NOERROR) {
        SDCARD.error = i + SD_CARD_ERROR_INIT_2;
        SD_RETURN(KO);
    }

    // increase clock speed for read/write
    SD_set_fast_clock_speed();

    SD_RETURN(OK);
}


// ---------------------------------------------------------------------------
// Write a buffer data at the specified block address
// ---------------------------------------------------------------------------
u8 SD_write_block(sdCardAddr_t block,
                  u8* buffer) 
{ 
    // write RAM sector to MMC
    u8 c;
    u16 i;
    SD_select_chip();
    // 512 byte-write-mode
    SD_command(SD_CMD24_WRITE_BLOCK, block*SD_CARD_BLOCK_SIZE, 0xFF/*dummy crc*/);
    if (!SD_wait(SD_R1_NOERROR)) {
        SDCARD.error += SD_CARD_ERROR_WRITE_1;
        SD_RETURN(KO);
    }

    SD_SPI(0xFF);
    SD_SPI(0xFF);
    SD_SPI(SD_START_TOKEN);
    // write ram sectors to SDCard
    for (i=0; i < SD_CARD_BLOCK_SIZE; i++) {
        SD_SPI(buffer[i]);
    }

    // at the end, send 2 dummy bytes (CRC)
    SD_SPI(0xFF);
    SD_SPI(0xFF);

    // get the data response token
    c = SD_SPI(0xFF);
    c &= 0x1F; 	// 0x1F = 0b.0001.1111;
    if ((c>>1) != 0x02) { // 0x02=data accepted ; 0x05=CRC error; 0x06=write error
        SDCARD.error = c;
        SDCARD.error += SD_CARD_ERROR_WRITE_2;
        SD_RETURN(KO);
    }
	
    // wait until card is not busy anymore
    if (!SD_wait(0xFF)) {
        SDCARD.error += SD_CARD_ERROR_WRITE_3;
        SD_RETURN(KO);
    }
	
    SD_RETURN(OK);
}

// ---------------------------------------------------------------------------
// Read data from the SDcard and fill buffer with it
// ---------------------------------------------------------------------------
u8 SD_read_block(sdCardAddr_t block,
                 u8* buffer)
{ 
    uint16_t i;
    SD_select_chip();
    // 512 byte-read-mode 
    SD_command(SD_CMD17_READ_BLOCK, block*SD_CARD_BLOCK_SIZE, 0xFF/*dummy CRC*/);
    if (!SD_wait(SD_R1_NOERROR)) {
        SDCARD.error += SD_CARD_ERROR_READ_1;
        SD_RETURN(KO);
    }
	
    // wait for 0xFE - start of any transmission
    if (!SD_wait(SD_START_TOKEN)) {
        SDCARD.error += SD_CARD_ERROR_READ_2;
        SD_RETURN(KO);
    }
	
    // proceed with SDCard-read
    for(i=0; i < SD_CARD_BLOCK_SIZE; i++) {
        buffer[i] = SD_SPI(0xFF); 
    }
    // at the end, send 2 dummy bytes
    SD_SPI(0xFF); // actually this returns the CRC/checksum byte
    SD_SPI(0xFF);
	
    // wait until card is not busy anymore
    if (!SD_wait(0xFF)) {
        SDCARD.error += SD_CARD_ERROR_WRITE_3;
        SD_RETURN(KO);
    }

    SD_RETURN(OK);
}

// ===========================================================================
// PUBLIC SDCARD REGISTER FUNCTIONS
// ===========================================================================

#ifdef SD_CARD_REGISTER_COMMANDS

// ---------------------------------------------------------------------------
// Retrieve the 'responseLength' bytes from the SD card after having sent 
// 'command'
// Output: buffer[responseLength]
// ---------------------------------------------------------------------------
static u8 sdGetRegister(u8 command,
                        u8* buffer,
                        u8 responseLength)
{
    u8 i;
    SD_select_chip();

	// send the command
    SD_command(command ,0, 0xFF/*dummy CRC*/);
    if (!SD_wait(SD_R1_NOERROR)) {
        SDCARD.error += SD_CARD_ERROR_REGISTER_1;
        SD_RETURN(KO);
    }

	// wait for 0xFE - start of any transmission
    if (!SD_wait(SD_START_TOKEN)) {
        SDCARD.error += SD_CARD_ERROR_REGISTER_2;
        SD_RETURN(KO);
    }

    for(i=0; i < responseLength; i++) {
        buffer[i] = SD_SPI(0xFF); 
    } 

    // CRC 16
    SD_SPI(0xFF); 
    SD_SPI(0xFF); 

    // wait until card is not busy anymore
    if (!SD_wait(0xFF)) {
        SDCARD.error += SD_CARD_ERROR_REGISTER_3;
        SD_RETURN(KO);
    }

    SD_RETURN(OK);
}

// ---------------------------------------------------------------------------
// Retrieve the 16 bytes Card Identification register
// ---------------------------------------------------------------------------
u8 SD_get_CSD(u8* buffer)
{
    return SD_get_register(SD_CMD9_SEND_CSD, buffer, 16);
}

// ---------------------------------------------------------------------------
// Retrieve the 16 bytes Card Identification register
// ---------------------------------------------------------------------------
u8 SD_get_CID(u8* buffer)
{
    return SD_get_register(SD_CMD10_SEND_CID, buffer, 16);
}

// ---------------------------------------------------------------------------
//  Retrieve the 4 bytes of the OCR register
// ---------------------------------------------------------------------------
u8 SD_get_OCR(u8* buffer)
{
    u8 i, r1;
    SD_select_chip();
    // Read OCR
    SD_command(SD_CMD58_READ_OCR ,0, 0xFF/*dummy CRC*/);
    r1 = SD_SPI(0xFF);

    for(i=0; i < 4; i++) {
        buffer[i] = SD_SPI(0xFF); 
    }
    if ( r1 != SD_R1_NOERROR) {
        SDCARD.error = r1 + SD_CARD_ERROR_OCR_1;
        SD_RETURN(KO);
    }

    SD_RETURN(OK);
}

// ---------------------------------------------------------------------------
// Retrieve the 2 bytes of the status register
// ---------------------------------------------------------------------------
u8 SD_get_status(unsigned char* buffer)
{
    SD_select_chip();
    // Read Status
    SD_command(SD_CMD13_SEND_STATUS ,0, 0xFF/*dummy CRC*/);
    buffer[0] = SD_SPI(0xFF); 
    buffer[1] = SD_SPI(0xFF); 

    SD_RETURN(OK);
}

#endif // SD_CARD_REGISTER_COMMANDS




//---------------------
//  Copyright (C) 2000-2009  <Yann GOUY>
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
//  you can write to me at <yann_gouy@yahoo.fr>
//

#include "drivers/eeprom.h"

#include <avr/io.h>			// EEAR, EEDR
#include <avr/interrupt.h>	// ISR()

#include "utils/fifo.h"


//------------------------------
// private defines
//


//------------------------------
// private variables
//

static struct {
	u16 addr;	// write addr
	u8* data;	// pointer to buffer
	u8 len;		// buffer length
	u8 n;		// index
} eep;


//------------------------------
// private fonctions
//

ISR(EE_READY_vect)
{
	// one more byte written
	// so proceed to next one
	eep.n++;

	// if there is still some place in the buffer
	if ( eep.n < eep.len ) {
		// write next byte
		eep.addr++;
		EEAR = eep.addr;
		eep.data++;
		EEDR = *eep.data;
		EECR |= _BV(EEMPE);
		EECR |= _BV(EEPE);
	}
	// buffer is full
	else {
		// disable eeprom interrupt
		EECR &= ~_BV(EERIE);
	}
}


//------------------------------
// public fonctions
//

// EEPROM driver initialization
void nnk_eep_init(void)
{
	// reset internals
	eep.n = 0;
	eep.data = NULL;

	// erase and write mode
	EECR = 0;
}


// read len byte(s) from EEPROM address addr and copy them in data
u8 nnk_eep_read(u16 addr, u8* data, u8 len)
{
	u8 i;

	// if EEPROM is busy
	if ( EECR & _BV(EEPE) ) {
		// it can't be read
		return KO;
	}

	// read each byte
	for ( i = 0; i < len; i++ ) {
		// set address
		EEAR = addr + i;

		// start read action
		EECR |= _BV(EERE);

		// read result
		*(data + i) = EEDR;
	}

	return OK;
}


// write len byte(s) to EEPROM address addr from data
u8 nnk_eep_write(u16 addr, u8* data, u8 len)
{
	// if EEPROM is busy or a write is running
	if ( (EECR & _BV(EEPE)) || (eep.data != NULL) ) {
		// it can't be written
		return KO;
	}

	// save data buffer address and reset index
	eep.addr = addr;
	eep.data = data;
	eep.len = len;
	eep.n = 0;

	// start first byte write
	EEAR = addr;
	EEDR = *data;
	EECR |= _BV(EEMPE);
	EECR |= _BV(EEPE);

	// enable eeprom interrupt
	EECR |= _BV(EERIE);

	return OK;
}

u8 nnk_eep_is_fini(void)
{
	// while writing
	if ( eep.n < eep.len ) {
		// it is not finished!!!!
		return KO;
	}

	return OK;
}

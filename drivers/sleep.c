//---------------------
//  Copyright (C) 2008  <Yann GOUY>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
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

#include "drivers/sleep.h"

#include <avr/io.h>			// UBRRH, UBBRL, UCSR?
#include <avr/interrupt.h>	// ISR()


//------------------------------
// private variables
//

static struct {
	// mask of the registered clients
	u16 register_mask;

	// mask of the current clients requesting sleep
	u16 current_mask;

	// bit value for the next registering client
	u8 mask_bit;

	// number of times the sleep mode is reached
	u16 stat;
} slp;


//------------------------------
// private fonctions
//


//------------------------------
// public fonctions
//

// setup of the SLEEP
void nnk_slp_init(void)
{
	// reset the whole structure
	slp.register_mask = 0;
	slp.current_mask = 0;
	slp.mask_bit = 0;
	slp.stat = 0;

	// enable idle sleep mode 
	MCUCR |= _BV(SE);
}


// return the sleep mask for the registering client
u16 nnk_slp_register(void)
{
	u16 mask;

	// compute mask for current registering client
	mask = 1 << slp.mask_bit;

	// update the mask for the registered clients
	slp.register_mask |= mask;

	// increment the counter for the next client
	slp.mask_bit++;

	// get to the client its mask
	return mask;
}


// a registered client can request to sleep
u8 nnk_slp_request(u16 mask)
{
	// add client mask to the global mask
	slp.current_mask |= mask;

	// if the mask is complete
	if (slp.current_mask == slp.register_mask) {
		// sleep
		__asm__ __volatile__ ("sleep");

		// on wake-up, update stats
		slp.stat++;

		// and return OK
		return OK;
	}

	// else return KO
	return KO;
}


// a registered client can unrequest to sleep
void nnk_slp_unrequest(u16 mask)
{
	// remove client mask from the global mask
	slp.current_mask &= ~mask;
}

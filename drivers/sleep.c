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

#include <avr/io.h>		// UBRRH, UBBRL, UCSR?
#include <avr/interrupt.h>	// ISR()


//------------------------------
// private variables
//

static struct {
	// mask of the registered clients
	slp_t register_mask;

	// mask of the current clients requesting sleep
	slp_t current_mask;

	// bit value for the next registering client
	u8 mask_bit;
} SLP;


//------------------------------
// private fonctions
//


//------------------------------
// public fonctions
//

// setup of the SLEEP
void SLP_init(void)
{
	// reset the whole structure
	SLP.register_mask = 0;
	SLP.current_mask = 0;
	SLP.mask_bit = 0;
}


// return the sleep mask for the registering client
slp_t SLP_register(void)
{
	slp_t mask;

	// compute mask for current registering client
	mask = 1 << SLP.mask_bit;

	// update the mask for the registered clients
	SLP.register_mask |= mask;

	// increment the counter for the next client
	SLP.mask_bit++;

	// get to the client its mask
	return mask;
}


// a registered client can request to sleep
u8 SLP_request(slp_t mask)
{
	// add client mask to the global mask
	SLP.current_mask |= mask;

	// if the mask is complete
	if (SLP.current_mask == SLP.register_mask) {
		// sleep
		__asm__ __volatile__ ("sleep");

		// on wake-up, clear mask
		SLP.current_mask = 0;

		// and return OK
		return OK;
	}

	// else return KO
	return KO;
}

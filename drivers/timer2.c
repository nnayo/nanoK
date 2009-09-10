//---------------------
//  Copyright (C) 2000-2008  <Yann GOUY>
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

#include "timer2.h"

#include <avr/interrupt.h>		// ISR()
# include <avr/io.h>			// TCNT2


//-----------------------
// private variables
//

static struct {
	u8 prescaler;
	void (*call_back)(void* misc);
	void* misc;
} TMR2;

//-----------------------
// private functions
//

ISR(TIMER2_OVF_vect)
{
	// call the provided call back if any
	if ( TMR2.call_back != NULL )
		(TMR2.call_back)(TMR2.misc);
	else
		// stop by applying 0 prescaler
		TCCR2 = TMR2_STOP;
}

ISR(TIMER2_COMP_vect)
{
	// call the provided call back if any
	if ( TMR2.call_back != NULL )
		(TMR2.call_back)(TMR2.misc);
	else
		// stop by applying 0 prescaler
		TCCR2 = TMR2_STOP;
}


//-----------------------
// public functions
//

void TMR2_init(tmr2_int_mode_t int_mode, tmr2_prescaler_t prescaler, tmr2_wgm_t wgm, u8 compare, void (*call_back)(void* misc), void* misc)
{
	// stop counter
	TCCR2 = TMR2_STOP;

	// save configuration
	TMR2.prescaler = prescaler | wgm;

	// reset counter
	TCNT2 = 0x00;

	// Output Compare Register can be set immediatly
	OCR2 = compare;

	// reset any pending interrupt
	TIMSK |= _BV(OCF2);
	TIMSK |= _BV(TOV2);

	// set interrupt mode
	switch (int_mode) {
		default:
		case TMR2_WITHOUT_INTERRUPT:
			TIMSK &= ~_BV(OCIE2);
			TIMSK &= ~_BV(TOIE2);
			break;

		case TMR2_WITH_OVERFLOW_INT:
			TIMSK &= ~_BV(OCIE2);
			TIMSK |= _BV(TOIE2);
			break;

		case TMR2_WITH_COMPARE_INT:
			TIMSK &= ~_BV(TOIE2);
			TIMSK |= _BV(OCIE2);
			break;
	}

	// set timeout call_back function
	TMR2.call_back = call_back;
	TMR2.misc = misc;
}

void TMR2_reset(void)
{
	// stop counter
	TCCR2 = TMR2_STOP;

	// reset counter
	TCNT2 = 0x00;
}

void TMR2_start(void)
{
	// start by applying configuration
	TCCR2 = TMR2.prescaler;
}

void TMR2_stop(void)
{
	// stop by applying 0 prescaler
	TCCR2 = TMR2_STOP;
}

u8 TMR2_get_value(void)
{
	return TCNT2;
}

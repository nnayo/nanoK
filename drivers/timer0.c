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

#include "timer0.h"

#include <avr/interrupt.h>		// ISR()
#include <avr/io.h>				// TCNT0


//-----------------------
// private variables
//

static volatile struct {
	u8 config;
	void (*call_back)(void* misc);
	void* misc;
} TMR0;

//-----------------------
// private functions
//

ISR(TIMER0_OVF_vect)
{
	// call the provided call back if any
	if ( TMR0.call_back != NULL )
		(TMR0.call_back)(TMR0.misc);
	else
		// stop by applying 0 prescaler
		TCCR0B = TMR0_STOP;

}

ISR(TIMER0_COMPA_vect)
{
	// call the provided call back if any
	if ( TMR0.call_back != NULL )
		(TMR0.call_back)(TMR0.misc);
	else
		// stop by applying 0 prescaler
		TCCR0B = TMR0_STOP;
}


//-----------------------
// public functions
//

void TMR0_init(tmr0_int_mode_t mode, u8 config, u8 compare, void (*call_back)(void* misc), void* misc)
{
	// stop counter
	TCCR0B = TMR0_STOP;

	// save configuration
	TMR0.config = config;

	// reset counter
	TCNT0 = 0x00;

	// Output Compare Register can be set immediatly
	OCR0A = compare;

	// reset any pending interrupt
	TIFR0 |= _BV(OCF0A);
	TIFR0 |= _BV(TOV0);

	// set interrupt mode
	switch (mode) {
		case TMR0_WITHOUT_INTERRUPT:
		default:
			TIMSK0 &= ~_BV(OCIE0A);
			TIMSK0 &= ~_BV(TOIE0);
			break;

		case TMR0_WITH_OVERFLOW_INT:
			TIMSK0 &= ~_BV(OCIE0A);
			TIMSK0 |= _BV(TOIE0);
			break;

		case TMR0_WITH_COMPARE_INT:
			TIMSK0 &= ~_BV(TOIE0);
			TIMSK0 |= _BV(OCIE0A);
			break;
	}

	// set timeout call_back function
	TMR0.call_back = call_back;
	TMR0.misc = misc;
}

void TMR0_reset(void)
{
	// stop counter
	TCCR0B = TMR0_STOP;

	// reset counter
	TCNT0 = 0x00;
}

void TMR0_start(void)
{
	// start by applying configuration
	TCCR0B = TMR0.config;
}

void TMR0_stop(void)
{
	// stop by applying 0 prescaler
	TCCR0B = TMR0_STOP;
}

u8 TMR0_get_value(void)
{
	return TCNT0;
}



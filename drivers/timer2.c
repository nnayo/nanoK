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
} tmr2;

//-----------------------
// private functions
//

ISR(TIMER2_OVF_vect)
{
	// call the provided call back if any
	if ( tmr2.call_back != NULL )
		(tmr2.call_back)(tmr2.misc);
	else
		// stop by applying 0 prescaler
		TCCR2B = TMR2_STOP;
}

ISR(TIMER2_COMPA_vect)
{
	// call the provided call back if any
	if ( tmr2.call_back != NULL )
		(tmr2.call_back)(tmr2.misc);
	else
		// stop by applying 0 prescaler
		TCCR2B = TMR2_STOP;
}


//-----------------------
// public functions
//

void TMR2_init(enum nnk_tmr2_int_mode int_mode, enum nnk_tmr2_prescaler prescaler, enum nnk_tmr2_wgm wgm, u8 compare, void (*call_back)(void* misc), void* misc)
{
	// stop counter
	TCCR2B = TMR2_STOP;

	// save configuration
	tmr2.prescaler = prescaler;

	// reset counter
	TCNT2 = 0x00;

	// set mode
	TCCR2A = wgm;

	// Output Compare Register can be set immediatly
	OCR2A = compare;

	// reset any pending interrupt
	TIFR2 |= _BV(OCF2B);
	TIFR2 |= _BV(OCF2A);
	TIFR2 |= _BV(TOV2);

	// set interrupt mode
	switch (int_mode) {
		default:
		case TMR2_WITHOUT_INTERRUPT:
			TIMSK2 &= ~_BV(OCIE2A);
			TIMSK2 &= ~_BV(TOIE2);
			break;

		case TMR2_WITH_OVERFLOW_INT:
			TIMSK2 &= ~_BV(OCIE2A);
			TIMSK2 |= _BV(TOIE2);
			break;

		case TMR2_WITH_COMPARE_INT:
			TIMSK2 &= ~_BV(TOIE2);
			TIMSK2 |= _BV(OCIE2A);
			break;
	}

	// set timeout call_back function
	tmr2.call_back = call_back;
	tmr2.misc = misc;
}

void TMR2_reset(void)
{
	// stop counter
	TCCR2B = TMR2_STOP;

	// reset counter
	TCNT2 = 0x00;
}

void TMR2_start(void)
{
	// start by applying configuration
	TCCR2B = tmr2.prescaler;
}

void TMR2_stop(void)
{
	// stop by applying 0 prescaler
	TCCR2B = TMR2_STOP;
}

u8 TMR2_get_value(void)
{
	u8 sreg = SREG;
	cli();
	u16 tmp = TCNT2;

	SREG = sreg;
	return tmp;
}

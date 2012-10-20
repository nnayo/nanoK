//---------------------
//  Copyright (C) 2000-2012  <Yann GOUY>
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

#include "timer1.h"

#include <avr/interrupt.h>		// ISR()
#include <avr/io.h>			// TCNT1


//-----------------------
// private variables
//

static struct {
	u8 prescaler;
	void (*call_back)(tmr1_chan_t chan, void* misc);
	void* misc;
} TMR1;

//-----------------------
// private functions
//

ISR(TIMER1_CAPT_vect)
{
	// call the provided call back if any
	if ( TMR1.call_back != NULL )
		(TMR1.call_back)(TMR1_CAPT, TMR1.misc);
}

ISR(TIMER1_COMPA_vect)
{
	// call the provided call back if any
	if ( TMR1.call_back != NULL )
		(TMR1.call_back)(TMR1_A, TMR1.misc);
	else
		// stop by applying 0 prescaler
		TCCR1B = TMR1_STOP;
}


ISR(TIMER1_COMPB_vect)
{
	// call the provided call back if any
	if ( TMR1.call_back != NULL )
		(TMR1.call_back)(TMR1_B, TMR1.misc);
	else
		// stop by applying 0 prescaler
		TCCR1B = TMR1_STOP;
}

ISR(TIMER1_OVF_vect)
{
	// call the provided call back if any
	if ( TMR1.call_back != NULL )
		(TMR1.call_back)(TMR1_OVF, TMR1.misc);
	else
		// stop by applying 0 prescaler
		TCCR1B = TMR1_STOP;
}


//-----------------------
// public functions
//

void TMR1_init(tmr1_int_mode_t int_mode, tmr1_prescaler_t prescaler, tmr1_wgm_t wgm, void (*call_back)(tmr1_chan_t chan, void* misc), void* misc)
{
	// stop counter
	TCCR1B = TMR1_STOP;

	// save configuration
	TMR1.prescaler = prescaler;

	// reset counter
	TCNT1 = 0x00;

	// set mode
	TCCR1A = wgm;

	// reset any pending interrupt
	TIFR1 |= _BV(ICF1) | _BV(OCF1B) | _BV(OCF1A) | _BV(TOV1);

	// set interrupt mode
	TIMSK1 &= ~(_BV(ICIE1) | _BV(OCIE1B) | _BV(OCIE1A) | _BV(TOIE1));
	TIMSK1 |= int_mode;

	// set timeout call_back function
	TMR1.call_back = call_back;
	TMR1.misc = misc;
}

void TMR1_reset(void)
{
	// stop counter
	TCCR1B = TMR1_STOP;

	// reset counter
	TCNT1L = 0x00;
	TCNT1H = 0x00;
}

void TMR1_start(void)
{
	// start by applying configuration
	TCCR1B = TMR1.prescaler;
}

void TMR1_stop(void)
{
	// stop by applying 0 prescaler
	TCCR1B = TMR1_STOP;
}

u16 TMR1_get(void)
{
	return (TCNT1L << 0) | ((u16)TCNT1H << 8);
}

void TMR1_compare_set(tmr1_chan_t chan, u16 val)
{
	switch (chan) {
	case TMR1_A:
		OCR1AH = (val & 0xff00) >> 8;
		OCR1AL = (val & 0x00ff) >> 0;
		break;

	case TMR1_B:
		OCR1BH = (val & 0xff00) >> 8;
		OCR1BL = (val & 0x00ff) >> 0;
		break;

	default:
		break;
	}
}

u16 TMR1_compare_get(tmr1_chan_t chan)
{
	u16 val;

	switch (chan) {
	case TMR1_A:
		val = OCR1AL;
		val |= (u16)OCR1AH << 8;
		break;

	case TMR1_B:
		val = OCR1BL;
		val |= (u16)OCR1BH << 8;
		break;

	default:
		val = 0;
		break;
	}

	return val;
}

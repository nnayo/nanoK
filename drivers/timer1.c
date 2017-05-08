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
	void (*call_back)(enum nnk_tmr1_chan chan, void* misc);
	void* misc;
} tmr1;

//-----------------------
// private functions
//

ISR(TIMER1_CAPT_vect)
{
	// call the provided call back if any
	if ( tmr1.call_back != NULL )
		(tmr1.call_back)(NNK_TMR1_CAPT, tmr1.misc);
}

ISR(TIMER1_COMPA_vect)
{
	// call the provided call back if any
	if ( tmr1.call_back != NULL )
		(tmr1.call_back)(NNK_TMR1_A, tmr1.misc);
	else
		// stop by applying 0 prescaler
		TCCR1B = NNK_TMR1_STOP;
}


ISR(TIMER1_COMPB_vect)
{
	// call the provided call back if any
	if ( tmr1.call_back != NULL )
		(tmr1.call_back)(NNK_TMR1_B, tmr1.misc);
	else
		// stop by applying 0 prescaler
		TCCR1B = NNK_TMR1_STOP;
}

ISR(TIMER1_OVF_vect)
{
	// call the provided call back if any
	if ( tmr1.call_back != NULL )
		(tmr1.call_back)(NNK_TMR1_OVF, tmr1.misc);
	else
		// stop by applying 0 prescaler
		TCCR1B = NNK_TMR1_STOP;
}


//-----------------------
// public functions
//

void nnk_tmr1_init(enum nnk_tmr1_int_mode int_mode, enum nnk_tmr1_prescaler prescaler, enum nnk_tmr1_wgm wgm, enum nnk_tmr1_cmp_out_md cmp_md, void (*call_back)(enum nnk_tmr1_chan chan, void* misc), void* misc)
{
	// stop counter
	TCCR1B = NNK_TMR1_STOP;

	// save prescaler configuration
	tmr1.prescaler = prescaler;

	// reset counter
	TCNT1 = 0x0000;

	// set mode
	TCCR1A = (cmp_md << 4) | (wgm & (_BV(WGM11) | _BV(WGM10)));
	TCCR1B = (TCCR1B & (_BV(WGM13) | _BV(WGM12))) | ((wgm & 0x0c) << 1);

	// reset any pending interrupt
	TIFR1 |= _BV(ICF1) | _BV(OCF1B) | _BV(OCF1A) | _BV(TOV1);

	// set interrupt mode
	TIMSK1 &= ~(_BV(ICIE1) | _BV(OCIE1B) | _BV(OCIE1A) | _BV(TOIE1));
	TIMSK1 |= int_mode;

	// set timeout call_back function
	tmr1.call_back = call_back;
	tmr1.misc = misc;
}


void nnk_tmr1_reset(void)
{
	// stop counter
	nnk_tmr1_stop();

	// reset counter
	TCNT1 = 0x0000;
}


void nnk_tmr1_start(void)
{
	// start by applying prescaler configuration
	TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11) | _BV(CS10))) | (tmr1.prescaler & (_BV(CS12) | _BV(CS11) | _BV(CS10)));
}


void nnk_tmr1_stop(void)
{
	// stop by applying 0 prescaler
	TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11) | _BV(CS10))) | (NNK_TMR1_STOP & (_BV(CS12) | _BV(CS11) | _BV(CS10)));
}


u16 nnk_tmr1_get(void)
{
	u8 sreg = SREG;
	cli();
	u16 cnt = TCNT1;

	SREG = sreg;
	return cnt;
}


void nnk_tmr1_compare_set(enum nnk_tmr1_chan chan, u16 val)
{
	switch (chan) {
	case NNK_TMR1_CAPT:
		ICR1 = val;
		break;

	case NNK_TMR1_A:
		OCR1A = val;
		break;

	case NNK_TMR1_B:
		OCR1B = val;
		break;

	default:
		break;
	}
}


u16 nnk_tmr1_compare_get(enum nnk_tmr1_chan chan)
{
	u16 val;

	switch (chan) {
	case NNK_TMR1_CAPT:
		val = ICR1;
		break;

	case NNK_TMR1_A:
		val = OCR1A;
		break;

	case NNK_TMR1_B:
		val = OCR1B;
		break;

	default:
		val = 0;
		break;
	}

	return val;
}

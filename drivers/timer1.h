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

#ifndef __TIMER1_H__
# define __TIMER1_H__

# include "type_def.h"	// u8 
# include <avr/io.h>			// TCNT1

// interrupt mode
//
typedef enum {
	TMR1_WITHOUT_INTERRUPT	= 0,
	TMR1_WITH_OVERFLOW_INT	= _BV(TOIE1),
	TMR1_WITH_COMPARE_INT	= _BV(ICIE1),
	TMR1_WITH_INT_COMP_A	= _BV(OCIE1A),
	TMR1_WITH_INT_COMP_B	= _BV(OCIE1B),
} tmr1_int_mode_t;

// timer channel
//
typedef enum {
	TMR1_CAPT,
	TMR1_A,
	TMR1_B,
	TMR1_OVF,
} tmr1_chan_t;

// waveform generation mode
//
typedef enum {
	TMR1_WGM_NORMAL,	// normal overflow on 0xff
	TMR1_WGM_PWM,		// pwm
	TMR1_WGM_CTC,		// clear timer on compare, reset on compare value
	TMR1_WGM_FASTPWM,	// fast pwm
} tmr1_wgm_t;

// prescaler values
// TODO
//
//  internal | timer1    | overflow
//  clock (*)| prescaler | time
// ----------+-----------+----------
//   125 ns  |    /1     |  0.032 ms
//   125 ns  |    /8     |  0.256 ms
//   125 ns  |    /32    |  1.024 ms
//   125 ns  |    /64    |  2.048 ms
//   125 ns  |    /128   |  4.096 ms
//   125 ns  |    /256   |  8.192 ms
//   125 ns  |    /1024  | 32.768 ms
// (*) 125 ns <=> 8MHz

typedef enum {
	TMR1_STOP,
	TMR1_PRESCALER_1,
	TMR1_PRESCALER_8,
	TMR1_PRESCALER_32,
	TMR1_PRESCALER_64,
	TMR1_PRESCALER_128,
	TMR1_PRESCALER_256,
	TMR1_PRESCALER_1024
} tmr1_prescaler_t;

// init the timer:
// - to generate or not an overflow interrupt,
// - with a resolution,
// - a comparaison value (let it 0 if useless),
// - and to call a function if provided on oveflow
//
// only OCRA is used right now
extern void TMR1_init(tmr1_int_mode_t int_mode, tmr1_prescaler_t prescaler, tmr1_wgm_t wgm, void (*call_back)(tmr1_chan_t chan, void* misc), void* misc);

// reset the timer to 0
//
extern void TMR1_reset(void);

// start the timer from its current value
//
extern void TMR1_start(void);

// stop the timer at its current value
//
extern void TMR1_stop(void);

// return the current value of the timer
//
extern u16 TMR1_get(void);

// set the compare value of the given channel
//
extern void TMR1_compare_set(tmr1_chan_t chan, u16 val);

// get the compare value of the given channel
//
extern u16 TMR1_compare_get(tmr1_chan_t chan);

#endif

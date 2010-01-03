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

#ifndef __TIMER2_H__
# define __TIMER2_H__

# include "type_def.h"	// u8

// interrupt mode
//
typedef enum {
	TMR2_WITHOUT_INTERRUPT,
	TMR2_WITH_OVERFLOW_INT,
	TMR2_WITH_COMPARE_INT,
} tmr2_int_mode_t;

// waveform generation mode
//
typedef enum {
	TMR2_WGM_NORMAL,	// normal overflow on 0xff
	TMR2_WGM_PWM,		// pwm
	TMR2_WGM_CTC,		// clear timer on compare, reset on compare value
	TMR2_WGM_FASTPWM,	// fast pwm
} tmr2_wgm_t;

// prescaler values
//
//  internal | timer2    | overflow
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
	TMR2_STOP,
	TMR2_PRESCALER_1,
	TMR2_PRESCALER_8,
	TMR2_PRESCALER_32,
	TMR2_PRESCALER_64,
	TMR2_PRESCALER_128,
	TMR2_PRESCALER_256,
	TMR2_PRESCALER_1024
} tmr2_prescaler_t;

// init the timer:
// - to generate or not an overflow interrupt,
// - with a resolution,
// - a comparaison value (let it 0 if useless),
// - and to call a function if provided on oveflow
//
// only OCRA is used right now
extern void TMR2_init(tmr2_int_mode_t int_mode, tmr2_prescaler_t prescaler, tmr2_wgm_t wgm, u8 compare, void (*call_back)(void* misc), void* misc);

// reset the timer to 0
//
extern void TMR2_reset(void);

// start the timer from its current value
//
extern void TMR2_start(void);

// stop the timer at its current value
//
extern void TMR2_stop(void);

// return the current value of the timer
//
extern u8 TMR2_get_value(void);

#endif

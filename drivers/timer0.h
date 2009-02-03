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

#ifndef __TIMER0_H__
# define __TIMER0_H__

# include "type_def.h"	// u8

// mode
// for TMR0_init()
//
typedef enum {
	TMR0_WITHOUT_INTERRUPT,
	TMR0_WITH_OVERFLOW_INT,
	TMR0_WITH_COMPARE_INT,
} tmr0_int_mode_t;


// Waveform Generation Mode
// WGM01 : WGM00
//   0       0		normal
//   0       1		PWM phase correct
//   1       0		CTC (Clear Timer on Compare Match)
//   1       1		fast PWM
typedef enum {
	TMR0_WGM_NORMAL		= 0x00,
	TMR0_WGM_PHASE_CORRECT	= 0x40,
	TMR0_WGM_CTC		= 0x08,
	TMR0_WGM_FAST_PWM	= 0x48,
} tmr0_wgm_t;


// Compare Match Output Mode
// COM01:0: Compare Match Output Mode
// 					normal		phase correct	CTC		fast PWM
typedef enum {
	TMR0_COM_00	= 0x00,	//	OC0 disconnect	OC0 disconnect	OC0 disconnect	OC0 disconnect
	TMR0_COM_01	= 0x10,	//	toggle OC0	reserved	toggle OC0	reserved
	TMR0_COM_10	= 0x20,	//	clear OC0	clear OC0 up	clear OC0	non inverting
	TMR0_COM_11	= 0x30,	//	set OC0		set OC0 up	set OC0		inverting
} tmr0_com_t;


// prescaler values
// for TMR0_init()
//
//  internal | timer0    | overflow
//  clock (*)| prescaler | time
// ----------+-----------+----------
//   125 ns  |    /1     |  0.032 ms
//   125 ns  |    /8     |  0.256 ms
//   125 ns  |    /64    |  2.048 ms
//   125 ns  |    /256   |  8.192 ms
//   125 ns  |    /1024  | 32.768 ms
// (*) 125 ns <=> 8MHz

typedef enum {
	TMR0_STOP,
	TMR0_PRESCALER_1,
	TMR0_PRESCALER_8,
	TMR0_PRESCALER_64,
	TMR0_PRESCALER_256,
	TMR0_PRESCALER_1024,
} tmr0_prescaler_t;


// init the timer:
// - to generate or not an overflow interrupt,
// - with a resolution,
// - and to call a function if provided on oveflow
//
// the timer is stopped out of the call of this function
// 
extern void TMR0_init(tmr0_int_mode_t mode, u8 config, u8 compare, void (*call_back)(void* misc), void* misc);

// reset the timer to 0
//
extern void TMR0_reset(void);

// start the timer from its current value
//
extern void TMR0_start(void);

// stop the timer at its current value
//
extern void TMR0_stop(void);

// return the current value of the timer
//
extern u8 TMR0_get_value(void);

#endif

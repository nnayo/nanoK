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
	TMR1_WGM_NORMAL,		// normal overflow on 0xffff
	TMR1_WGM_PWM_PC8,		// PWM, phase correct 8-bit
	TMR1_WGM_PWM_PC9,		// PWM, phase correct 9-bit
	TMR1_WGM_PWM_PC10,		// PWM, phase correct 10-bit
	TMR1_WGM_CTC_OCR1A,		// clear timer on compare, reset on compare value
	TMR1_WGM_FAST_PWM8,		// fast PWM 8-bit
	TMR1_WGM_FAST_PWM9,		// fast PWM 9-bit
	TMR1_WGM_FAST_PWM10,	// fast PWM 10-bit
	TMR1_WGM_PWM_PFC_ICR,	// PWM, phase and frequency correct TOP = ICR1
	TMR1_WGM_PWM_PFC_OCR,	// PWM, phase and frequency correct TOP = OCR1A
	TMR1_WGM_PWM_PC_ICR,	// PWM, phase correct TOP = ICR1
	TMR1_WGM_PWM_PC_OCR,	// PWM, phase correct TOP = OCR1A
	TMR1_WGM_CTC_ICR1,		// clear timer on compare, reset on compare value
	TMR1_WGM_RESERVED,
	TMR1_WGM_FAST_PWM_ICR1,	// fast PWM TOP = ICR1
	TMR1_WGM_FAST_PWM_OCR1A,// fast PWM TOP = OCR1A
} tmr1_wgm_t;

// compare output mode
//
typedef enum {
	COM1AB_0000,
	COM1AB_0001,
	COM1AB_0010,
	COM1AB_0011,
	COM1AB_0100,
	COM1AB_0101,
	COM1AB_0110,
	COM1AB_0111,
	COM1AB_1000,
	COM1AB_1001,
	COM1AB_1010,
	COM1AB_1011,
	COM1AB_1100,
	COM1AB_1101,
	COM1AB_1110,
	COM1AB_1111,
} tmr1_cmp_out_md_t;

// prescaler values
// TODO
//
//  timer1    | overflow   | overflow
//  prescaler | time 8MHz  | time 16MHz
// -----------+------------+-----------
//     /1     |   2.048 ms |   4.096 ms
//     /8     |  16.384 ms |  32.768 ms
//     /64    | 131.072 ms | 262.144 ms
//     /256   | 524.288 ms |   1.048  s
//     /1024  |   2.097  s |   4.192  s
// MHz => T = 125 ns
// 16MHz => T = 62.5 ns

typedef enum {
	TMR1_STOP,
	TMR1_PRESCALER_1,
	TMR1_PRESCALER_8,
	TMR1_PRESCALER_64,
	TMR1_PRESCALER_256,
	TMR1_PRESCALER_1024,
	TMR1_PRESCALER_T1_FALL,
	TMR1_PRESCALER_T1_RISE,
} tmr1_prescaler_t;

// init the timer:
// - to generate or not an overflow interrupt,
// - with a resolution,
// - in a mode
// - and to call a function if provided on overflow
//
extern void TMR1_init(tmr1_int_mode_t int_mode, tmr1_prescaler_t prescaler, tmr1_wgm_t wgm, tmr1_cmp_out_md_t cmp_md, void (*call_back)(tmr1_chan_t chan, void* misc), void* misc);

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

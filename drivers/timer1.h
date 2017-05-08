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
enum nnk_tmr1_int_mode {
	NNK_TMR1_WITHOUT_INTERRUPT	= 0,
	NNK_TMR1_WITH_OVERFLOW_INT	= _BV(TOIE1),
	NNK_TMR1_WITH_COMPARE_INT	= _BV(ICIE1),
	NNK_TMR1_WITH_INT_COMP_A	= _BV(OCIE1A),
	NNK_TMR1_WITH_INT_COMP_B	= _BV(OCIE1B),
};

// timer channel
//
enum nnk_tmr1_chan {
	NNK_TMR1_CAPT,
	NNK_TMR1_A,
	NNK_TMR1_B,
	NNK_TMR1_OVF,
};

// waveform generation mode
//
enum nnk_tmr1_wgm {
	NNK_TMR1_WGM_NORMAL,		// normal overflow on 0xffff
	NNK_TMR1_WGM_PWM_PC8,		// PWM, phase correct 8-bit
	NNK_TMR1_WGM_PWM_PC9,		// PWM, phase correct 9-bit
	NNK_TMR1_WGM_PWM_PC10,		// PWM, phase correct 10-bit
	NNK_TMR1_WGM_CTC_OCR1A,		// clear timer on compare, reset on compare value
	NNK_TMR1_WGM_FAST_PWM8,		// fast PWM 8-bit
	NNK_TMR1_WGM_FAST_PWM9,		// fast PWM 9-bit
	NNK_TMR1_WGM_FAST_PWM10,	// fast PWM 10-bit
	NNK_TMR1_WGM_PWM_PFC_ICR,	// PWM, phase and frequency correct TOP = ICR1
	NNK_TMR1_WGM_PWM_PFC_OCR,	// PWM, phase and frequency correct TOP = OCR1A
	NNK_TMR1_WGM_PWM_PC_ICR,	// PWM, phase correct TOP = ICR1
	NNK_TMR1_WGM_PWM_PC_OCR,	// PWM, phase correct TOP = OCR1A
	NNK_TMR1_WGM_CTC_ICR1,		// clear timer on compare, reset on compare value
	NNK_TMR1_WGM_RESERVED,
	NNK_TMR1_WGM_FAST_PWM_ICR1,	// fast PWM TOP = ICR1
	NNK_TMR1_WGM_FAST_PWM_OCR1A,	// fast PWM TOP = OCR1A
};

// compare output mode
//
enum nnk_tmr1_cmp_out_md {
	NNK_COM1AB_0000,
	NNK_COM1AB_0001,
	NNK_COM1AB_0010,
	NNK_COM1AB_0011,
	NNK_COM1AB_0100,
	NNK_COM1AB_0101,
	NNK_COM1AB_0110,
	NNK_COM1AB_0111,
	NNK_COM1AB_1000,
	NNK_COM1AB_1001,
	NNK_COM1AB_1010,
	NNK_COM1AB_1011,
	NNK_COM1AB_1100,
	NNK_COM1AB_1101,
	NNK_COM1AB_1110,
	NNK_COM1AB_1111,
};

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

enum nnk_tmr1_prescaler {
	NNK_TMR1_STOP,
	NNK_TMR1_PRESCALER_1,
	NNK_TMR1_PRESCALER_8,
	NNK_TMR1_PRESCALER_64,
	NNK_TMR1_PRESCALER_256,
	NNK_TMR1_PRESCALER_1024,
	NNK_TMR1_PRESCALER_T1_FALL,
	NNK_TMR1_PRESCALER_T1_RISE,
};

// init the timer:
// - to generate or not an overflow interrupt,
// - with a resolution,
// - in a mode
// - and to call a function if provided on overflow
//
extern void nnk_tmr1_init(enum nnk_tmr1_int_mode int_mode, enum nnk_tmr1_prescaler prescaler, enum nnk_tmr1_wgm wgm, enum nnk_tmr1_cmp_out_md cmp_md, void (*call_back)(enum nnk_tmr1_chan chan, void* misc), void* misc);

// reset the timer to 0
//
extern void nnk_tmr1_reset(void);

// start the timer from its current value
//
extern void nnk_tmr1_start(void);

// stop the timer at its current value
//
extern void nnk_tmr1_stop(void);

// return the current value of the timer
//
extern u16 nnk_tmr1_get(void);

// set the compare value of the given channel
//
extern void nnk_tmr1_compare_set(enum nnk_tmr1_chan chan, u16 val);

// get the compare value of the given channel
//
extern u16 nnk_tmr1_compare_get(enum nnk_tmr1_chan chan);

#endif

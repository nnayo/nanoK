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

#ifndef __RS_H__
# define __RS_H__

# include <avr/pgmspace.h>

# include <stdio.h>	// that way, including rs.h will also bring printf() and associated prototypes

# include "type_def.h"

// Rx and Tx buffer lengths
# define RS_RX_LEN	10
# define RS_TX_LEN	70

// baud rates for AVR @ 8 MHz
#if 0	// a nice macro, should serve as an example for creating baud macroes
# if F_CPU < 2000000UL && defined(U2X)
	UCSRA = _BV(U2X);             /* improve baud rate error by using 2x clk */
	UBRRL = (F_CPU / (8UL * UART_BAUD)) - 1;
# else
	UBRRL = (F_CPU / (16UL * UART_BAUD)) - 1;
# endif
#endif

# define B2400		207
# define B4800		103
# define B9600		51
# define B19200		25
# define B38400		12
# define B57600		8
# define B76800		6
# define B115200	3
# define B250000	1
# define B500000	0

extern void RS_init(u8 baud);			// setup for USART
						// baud given using provided macro
						// interrupt mode

extern void RS_cnt(u8* FE_cnt,			// return the values of Frame Error
			u8* DOR_cnt,		// Data OverRun
			u8* PE_cnt,		// Parity Error counters and
			u8* rx_ovfl_cnt);	// rx fifo overflow counter

//extern u8 RS_lock(void* rs, void);		// lock the RS for exclusive use
//extern u8 RS_unlock(void* rs, void);		// unlock the RS

#if 0
// all following functions are blocking
//
extern void RS_get(u8* data);			// read one byte 
extern void RS_put(u8 data);			// write one byte

extern void RS_put_hex8(u8 data);		// write data as 0xYY without 0x
extern void RS_put_hex16(u16 data);		// write data as 0xYYYY without 0x
extern void RS_put_hex32(u32 data);		// write data as 0xYYYYYYYY without 0x

extern void RS_str(u8* data);			// write a string 
extern void RS_str_P(PGM_P data);		// write a string from program space

extern u8 RS_can_get(void);			// report how many bytes can be read
extern u8 RS_can_put(void);			// report how many bytes can be written

extern void RS_flush(void);			// send every present characters
#endif

#endif

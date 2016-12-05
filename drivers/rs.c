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

#include "drivers/rs.h"

#include <stdio.h>		// printf() and associated functions
#include <avr/io.h>		// UBRRH, UBBRL, UCSR?
#include <avr/interrupt.h>	// ISR()

#include "utils/fifo.h"

//static const u8 dec2hex[] PROGMEM = "0123456789abcdef";

//------------------------------
// private variables
//

#define FP_BUF_LEN	10

static struct {
	FILE file;

	// fifoes and their associated buffers
	u8 rx_buf[RS_RX_LEN];
	struct nnk_fifo rx_fifo;
	u8 tx_buf[RS_TX_LEN];
	struct nnk_fifo tx_fifo;

	// buffer for floating point operations
	u8 fp_buf[FP_BUF_LEN];

	// hardware failures
	u8 FE_cnt;			// Frame Error counter
	u8 DOR_cnt;			// Data OverRun counter
	u8 PE_cnt;			// Parity Error counter

	// fifo failures
	u8 rx_ovfl_cnt;
} RS;


//------------------------------
// private fonctions
//

ISR(USART_RX_vect)
{
	u8 buf;

	// check for Frame Error
	if ( UCSR0A & _BV(FE0) ) {
		RS.FE_cnt++;

		buf = UDR0;		// received data read but unused

		return;
	}

	// check for Data OverRun
	if ( UCSR0A & _BV(DOR0) ) {
		RS.DOR_cnt++;

		buf = UDR0;
		if (nnk_fifo_put(&RS.rx_fifo, &buf) == KO)	// put rxed char in fifo
			RS.rx_ovfl_cnt++;		// on error, inc counter
	}

	// check for Parity Error
	if ( UCSR0A & _BV(UPE0) ) {
		RS.PE_cnt++;

		buf = UDR0;		// received data read but unused

		return;
	}

	buf = UDR0;
	if (nnk_fifo_put(&RS.rx_fifo, &buf) == KO)	// put rxed char in fifo
		RS.rx_ovfl_cnt++;		// on error, inc counter
}


ISR(USART_UDRE_vect)
{
	u8 buf;

	if ( nnk_fifo_get(&RS.tx_fifo, &buf) == OK)
		UDR0 = buf;		// get a char from fifo is available
	else
		UCSR0B &= ~_BV(UDRIE0);	// no more data to send, stop Tx interrupt
}


// write one byte
static int nnk_rs_put(char data, FILE* f)
{
	(void)f;

	// if there is some empty space in the Tx fifo
	if ( nnk_fifo_free(&RS.tx_fifo) ) {
		// add the new character to the Tx fifo
		nnk_fifo_put(&RS.tx_fifo, &data);

		// (re-)enable UDRE interrupt
		UCSR0B |= _BV(UDRIE0);
	}
	else {
		// (re-)enable UDRE interrupt
		UCSR0B |= _BV(UDRIE0);

		// and block as long as there is no empty space in the Tx fifo
		while ( nnk_fifo_put(&RS.tx_fifo, &data) != OK ) {
		}
	}

	return 0;
}


// read one byte
static int nnk_rs_get(FILE* f)
{
	u8 buf;

	(void)f;

	// if no character in fifo, 
	if ( nnk_fifo_get(&(RS.rx_fifo), &buf) != OK )
		return _FDEV_EOF;	// return End Of File
	else
		return (int)buf;	// return read data
}


//------------------------------
// public fonctions
//

// setup of the USART
void nnk_rs_init(u8 baud)
{
	// set transmission speed
	UBRR0H = 0;
	UBRR0L = baud;

	// enable interrupt on Rx, Rx and Tx
	UCSR0B = _BV(RXCIE0) | _BV(TXEN0) | _BV(RXEN0);

	// 8N1
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);

	// fifoes init
	nnk_fifo_init(&RS.rx_fifo, RS.rx_buf, RS_RX_LEN, sizeof(u8));
	nnk_fifo_init(&RS.tx_fifo, RS.tx_buf, RS_TX_LEN, sizeof(u8));

	// create the file
	fdev_setup_stream(&RS.file, nnk_rs_put, nnk_rs_get, _FDEV_SETUP_RW);
	// manually add informations about floating point buffer
	RS.file.buf = (char*) RS.fp_buf;
	RS.file.size = FP_BUF_LEN;

	// manually assign standard streams 
	stdout = &RS.file;
	stdin = &RS.file;
}

// return the values of Frame Error, Data OverRun and Parity Error counters
void nnk_rs_cnt(u8* FE_cnt,	u8* DOR_cnt, u8* PE_cnt, u8* rx_ovfl_cnt)
{
	*FE_cnt = RS.FE_cnt;
	*DOR_cnt = RS.DOR_cnt;
	*PE_cnt = RS.PE_cnt;
	*rx_ovfl_cnt = RS.rx_ovfl_cnt;
}

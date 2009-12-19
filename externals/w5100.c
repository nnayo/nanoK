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

// the driver is intended for W5100 chip from Wiznet
//


#include "externals/w5100.h"

#include "externals/w5100_internals.h"

#include "drivers/spi.h"

#include <string.h>		// memcpy()


//-----------------------------------------------------
// private variables
//

typedef struct {
	u16 start_addr;
	u16 end_addr;
	socket_state_t state;
} socket_t;

const u16 S_OFFSET[NB_SOCKETS] = { S0_OFFSET, S1_OFFSET, S2_OFFSET, S3_OFFSET };

static struct {
	socket_t socket[NB_SOCKETS];
} W5100;


//-----------------------------------------------------
// private functions
//

static void W5100_write(u16 addr, u8 data)
{
	w5100_stream_t w_tx;
	w5100_stream_t w_rx;

	w_tx.opcode = W5100_WRITE;
	w_tx.addr = addr;
	w_tx.data = data;

	SPI_master_blocking((u8*)&w_tx, sizeof(w_tx), (u8*)&w_rx, sizeof(w_rx));
}


static u8 W5100_read(u16 addr)
{
	w5100_stream_t w_tx;
	w5100_stream_t w_rx;

	w_tx.opcode = W5100_WRITE;
	w_tx.addr = addr;
	w_tx.data = 0xff;

	SPI_master_blocking((u8*)&w_tx, sizeof(w_tx), (u8*)&w_rx, sizeof(w_rx));

	return w_rx.data;
}


static socket_state_t W5100_get_state(u8 socket)
{
	// compute address of status reg to retrieve its value
	return (socket_state_t)W5100_read(Sn_SR + S_OFFSET[socket]);
}


//-----------------------------------------------------
// public functions
//

// initialization of the W5100 component
u8 W5100_init(void)
{
	// SPI link configuration (W5100 : spi mode 0, MSB first, min SCL period 70 ns (=> 14 MHz))
	SPI_init(SPI_MASTER, SPI_ZERO, SPI_MSB, SPI_DIV_2);

	// basic settings
	//
	// set Mode Register
	W5100_write(MR, 0x00);

	// Interrupt Mask Register
	W5100_write(IMR, 0x00);

	// Retry Time-value Register
	//	-> keep default : 200 ms

	// Retry Count Register
	//	-> keep default : 8

	// network settings
	//
	// Gateway Address Register '192.168.7.255'
	W5100_write(GAR0, 192);
	W5100_write(GAR1, 168);
	W5100_write(GAR2, 7);
	W5100_write(GAR3, 255);

	// Source Hardware Address Register '0.T.R.o.l.l' = 
	W5100_write(SHAR0, 0);
	W5100_write(SHAR1, 'T');
	W5100_write(SHAR2, 'R');
	W5100_write(SHAR3, 'o');
	W5100_write(SHAR4, 'l');
	W5100_write(SHAR5, 'l');

	// Subnet Mask Register '255.255.255.0'
	W5100_write(SUBR0, 255);
	W5100_write(SUBR1, 255);
	W5100_write(SUBR2, 255);
	W5100_write(SUBR3, 0);

	// Source IP Address Register (SIPR)
	W5100_write(SIPR0, 192);
	W5100_write(SIPR1, 168);
	W5100_write(SIPR2, 7);
	W5100_write(SIPR3, 77);

	// socket memory settings
	//
	// RMSR	: 2Ko per rx socket
	W5100_write(RMSR, 0x55);
	// TMSR	: 2Ko per tx socket
	W5100_write(TMSR, 0x55);

	return OK;
}


// send a data block to destination IP and port 
u8 W5100_tx(u32 ip, u16 port, u8* data, u8 len)
{
	return KO;
}


// read a data block received on port
u8 W5100_rx(u16 port, const u8* data, u8 len)
{
	return KO;
}

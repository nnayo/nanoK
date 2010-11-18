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

// socket descriptor
typedef struct {
	W5100_prot_t prot;
	u16 tx_start_addr;
	u16 tx_end_addr;
	u16 rx_start_addr;
	u16 rx_end_addr;
} socket_t;


// component instance
static struct {
	socket_t socket[NB_SOCKETS];
} W5100;


//-----------------------------------------------------
// private functions
//

// send a single byte to given address
static void W5100_write(u16 addr, u8 data)
{
	w5100_stream_t w_tx;
	w5100_stream_t w_rx;

	w_tx.opcode = W5100_WRITE;
	w_tx.addr = addr;
	w_tx.data = data;

	SPI_master_blocking((u8*)&w_tx, sizeof(w_tx), (u8*)&w_rx, sizeof(w_rx));
	// if transfer is successful, w_rx shall hold {0x00, 0x01, 0x02, 0x03}
}


// send a data block of len bytes starting at given address
static void W5100_write_block(u16 addr, u8* data, u8 len)
{
	u8 i;

	// send data byte by byte
	for (i = 0; i < len; i++) {
		W5100_write(addr + i, data[i]);
	}
}


// read a single byte from given address
static u8 W5100_read(u16 addr)
{
	w5100_stream_t w_tx;
	w5100_stream_t w_rx;

	w_tx.opcode = W5100_WRITE;
	w_tx.addr = addr;
	w_tx.data = 0xff;

	SPI_master_blocking((u8*)&w_tx, sizeof(w_tx), (u8*)&w_rx, sizeof(w_rx));
	// if transfer is successful, w_rx shall hold {0x00, 0x01, 0x02, 'read data'}

	return w_rx.data;
}


// retrieve a data block of len bytes starting at given address
static void W5100_read_block(u16 addr, u8* data, u8 len)
{
	u8 i;

	// retrieve data byte by byte
	for (i = 0; i < len; i++) {
		data[i] = W5100_read(addr + i);
	}
}


//-----------------------------------------------------
// public functions
//


// initialization of the W5100 component
u8 W5100_init(const W5100_mode_t mode, const MAC_address_t mac_addr)
{
	u8 i;

	// soft init
	//
	// reset sockets
	for (i = 0; i < NB_SOCKETS; i++) {
		W5100.socket[i].prot = SOCK_NONE;
	}

	// hardware settings
	//
	// SPI link configuration (W5100 : spi mode 0, MSB first, min SCL period 70 ns (=> 14 MHz))
	SPI_init(SPI_MASTER, SPI_ZERO, SPI_MSB, SPI_DIV_2);

	// set Mode Register (S/W reset, disable ping block, disable PPPoE mode, disable indirect bus) 
	W5100_write(MR, 0x00);

	// Interrupt Mask Register (mask all interrupts)
	W5100_write(IMR, 0x00);

	// Retry Time-value Register
	//	-> keep default : 200 ms

	// Retry Count Register
	//	-> keep default : 8

	// Source Hardware Address Register '0.T.R.o.l.l' = 
	W5100_write(SHAR0, 0);
	W5100_write(SHAR1, 'T');
	W5100_write(SHAR2, 'R');
	W5100_write(SHAR3, 'o');
	W5100_write(SHAR4, 'l');
	W5100_write(SHAR5, 'l');

	// socket memory settings
	//
	// RMSR	: 2Ko per rx socket
	W5100_write(RMSR, 0x55);
	for (i = 0; i < NB_SOCKETS; i++) {
		W5100.socket[i].tx_start_addr = 0x4000 + i * 0x0800;
		W5100.socket[i].tx_end_addr = 0x4800 - 1 + i * 0x0800;
	}

	// TMSR	: 2Ko per tx socket
	W5100_write(TMSR, 0x55);
	for (i = 0; i < NB_SOCKETS; i++) {
		W5100.socket[i].rx_start_addr = 0x6000 + i * 0x0800;
		W5100.socket[i].rx_end_addr = 0x6800 - 1 + i * 0x0800;
	}

	return OK;
}


// pseudo-thread for DHCP mode
void W5100_run(void)
{
	// if the DHCP mode is enable
	// handle it
	// then continue in normal mode
	//
	// normal mode
	// wait until IP address is set
	// then poll sockets status for received packets
}


// node IP address handling functions
void W5100_IP_address_set(const IP_address_t ip_addr)
{
	// network settings
	//
	// Gateway Address Register: IP_address & 'x.x.x.255'
	W5100_write(GAR0, ip_addr[0]);
	W5100_write(GAR1, ip_addr[1]);
	W5100_write(GAR2, ip_addr[2]);
	W5100_write(GAR3, 255);

	// Subnet Mask Register '255.255.255.0'
	W5100_write(SUBR0, 255);
	W5100_write(SUBR1, 255);
	W5100_write(SUBR2, 255);
	W5100_write(SUBR3, 0);

	// Source IP Address Register (SIPR)
	W5100_write(SIPR0, ip_addr[0]);
	W5100_write(SIPR1, ip_addr[1]);
	W5100_write(SIPR2, ip_addr[2]);
	W5100_write(SIPR3, ip_addr[3]);
}


void W5100_IP_address_get(IP_address_t ip_addr)
{
	// Source IP Address Register (SIPR)
	ip_addr[0] = W5100_read(SIPR0);
	ip_addr[1] = W5100_read(SIPR1);
	ip_addr[2] = W5100_read(SIPR2);
	ip_addr[3] = W5100_read(SIPR3);
}


// create a new socket
u8 W5100_socket(W5100_prot_t prot)
{
	socket_state_t state;
	u8 i;

	// search for available socket
	for (i = 0; i < NB_SOCKETS; i++) {
		// retrieve socket state
		state = (socket_state_t)W5100_read(S_OFFSET(i) + Sn_SR);

		// if socket is closed then it is available
		if ( (state == SOCKET_CLOSED) && (W5100.socket[i].prot == SOCK_NONE) ) {
			W5100.socket[i].prot = prot;
			return i;
		}
	}

	// no free socket found
	return -1;
}


// close a socket
u8 W5100_close(u8 sock)
{
	socket_state_t state;

	// retrieve socket state
	state = (socket_state_t)W5100_read(S_OFFSET(sock) + Sn_SR);

	// if socket is already closed
	if ( (state == SOCKET_CLOSED) && (W5100.socket[sock].prot == SOCK_NONE) )
		return KO;

	// send close command
	W5100_write(S_OFFSET(sock) + Sn_CR, CMD_CLOSE);

	// update socket protocol
	W5100.socket[sock].prot = SOCK_NONE;
	return OK;
}


// bind socket for incoming packets from ip_addr:port
u8 W5100_bind(u8 sock, IP_address_t ip_addr, u16 port)
{
	return KO;
}


// connect socket for outgoing packets to ip_addr:port
u8 W5100_connect(u8 sock, IP_address_t ip_addr, u16 port)
{
	u8 buf;

	switch (W5100.socket[sock].prot) {
		case SOCK_STREAM:	// TCP
			// set UDP mode
			W5100_write(S_OFFSET(sock) + Sn_MR, MODE_TCP);

			// set protocol value
			W5100_write(S_OFFSET(sock) + Sn_PORT0, (port & 0xff00) >> 8);
			W5100_write(S_OFFSET(sock) + Sn_PORT1, (port & 0x00ff) >> 0);

			// issue OPEN command
			W5100_write(S_OFFSET(sock) + Sn_CR, CMD_OPEN);

			// check if mode has correctly changed
			buf = W5100_read(S_OFFSET(sock) + Sn_SR);
			if (buf != SOCKET_INIT) {
				// issue CLOSE command
				W5100_write(S_OFFSET(sock) + Sn_CR, CMD_CLOSE);

				return KO;
			}

			break;

		case SOCK_DGRAM:	// UDP
			// set UDP mode
			W5100_write(S_OFFSET(sock) + Sn_MR, MODE_UDP);

			// set protocol value
			W5100_write(S_OFFSET(sock) + Sn_PORT0, (port & 0xff00) >> 8);
			W5100_write(S_OFFSET(sock) + Sn_PORT1, (port & 0x00ff) >> 0);

			// issue OPEN command
			W5100_write(S_OFFSET(sock) + Sn_CR, CMD_OPEN);

			// check if mode has correctly changed
			buf = W5100_read(S_OFFSET(sock) + Sn_SR);
			if (buf != SOCKET_UDP) {
				// issue CLOSE command
				W5100_write(S_OFFSET(sock) + Sn_CR, CMD_CLOSE);

				return KO;
			}

			// set destination address and port
			W5100_write_block(S_OFFSET(sock) + Sn_DIPR0, (u8*)&ip_addr, sizeof(IP_address_t));
			W5100_write_block(S_OFFSET(sock) + Sn_DPORT0, (u8*)&port, sizeof(u16));

			break;

		case SOCK_RAW:		// IP RAW
			// set IP RAW mode
			W5100_write(S_OFFSET(sock) + Sn_MR, MODE_IPRAW);

			// set protocol value
			W5100_write(S_OFFSET(sock) + Sn_PROTO, port & 0xff);

			// issue OPEN command
			W5100_write(S_OFFSET(sock) + Sn_CR, CMD_OPEN);

			// check if mode has correctly changed
			buf = W5100_read(S_OFFSET(sock) + Sn_SR);
			if (buf != SOCKET_IPRAW) {
				// issue CLOSE command
				W5100_write(S_OFFSET(sock) + Sn_CR, CMD_CLOSE);

				return KO;
			}

			break;

		default:
			return KO;
	}

	return OK;
}


// send data
u8 W5100_send(u8 sock, u8* data, u8 len)
{
	u16 free_size;
	u16 start_addr;
	u8 addr;

	// check if enough empty room
	// copy data block
	// start transmittion
	switch (W5100.socket[sock].prot) {
		case SOCK_STREAM:	// TCP
			break;

		case SOCK_DGRAM:	// UDP
			// check if enough empty room
			W5100_read_block(S_OFFSET(sock) + Sn_TX_FSR0, (u8*)&free_size, sizeof(free_size));
			if (free_size < len) {
				return KO;
			}

			// copy data block
			W5100_write_block(S_OFFSET(sock) + Sn_TX_WR0, (u8*)&start_addr, sizeof(start_addr));

			// start transmittion
			break;

		case SOCK_RAW:	// IP RAW
			break;

		default:
			break;
	}

	return KO;
}


// read received data if any
u8 W5100_recv(u8 sock, u8* data, u8* len)
{
	u8 buf;

	// check if data is received
	buf = W5100_read(S_OFFSET(sock) + Sn_IR);
	if ( ~(buf & Sn_IR_RECV) ) {
		// no data received
		return KO;
	}

	switch (W5100.socket[sock].prot) {
		case SOCK_STREAM:	// TCP
			break;

		case SOCK_DGRAM:	// UDP
			break;

		case SOCK_RAW:	// IP RAW
			break;

		default:
			break;
	}
	return KO;
}

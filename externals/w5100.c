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

// design
//
// first, SW reset and basic configuration
//
// then if DHCP is required, 
// all other activities are suspended up to IP address acquisition
//
// every socket have its dedicated thread
// transmission duration and time-out are handled in this thread
//

#include "externals/w5100.h"

#include "externals/w5100_internals.h"

#include "drivers/spi.h"

#include "utils/pt.h"

#include <string.h>		// memcpy()


//-----------------------------------------------------
// private defines
//

#define UDP_HEADER_SIZE	8
#define RAW_HEADER_SIZE	6


//-----------------------------------------------------
// private variables
//

// socket descriptor
typedef struct {
	W5100_prot_t prot;
	u16 tx_down_limit;
	u16 tx_up_limit;
	u16 rx_down_limit;
	u16 rx_up_limit;
	pt_t pt;
} socket_t;


// component instance
static struct {
	W5100_mode_t mode;

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
static void W5100_write_block(u16 addr, u8* data, u8 len, u16 down_limit, u16 up_limit)
{
	u8 i;

	// send data byte by byte
	for (i = 0; i < len; i++) {
		if ( (addr + i) < up_limit ) {
			W5100_write(addr + i, data[i]);
		} 
		else {
			W5100_write(addr + i - up_limit + down_limit, data[i]);
		}
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
static void W5100_read_block(u16 addr, u8* data, u8 len, u16 down_limit, u16 up_limit)
{
	u8 i;

	// retrieve data byte by byte
	for (i = 0; i < len; i++) {
		if ( (addr + i) < up_limit ) {
			data[i] = W5100_read(addr + i);
		} 
		else {
			data[i] = W5100_read(addr + i - up_limit + down_limit);
		}
	}
}


static PT_THREAD( W5100_tx(u8 sock, u8* data, u8 len, IP_address_t ip_addr, u16 port, W5100_status_t* status) )
{
	socket_t* socket = &W5100.socket[sock];
	pt_t* pt = &socket->pt;
	u16 sock_offset = S_OFFSET(sock);
	u16 free_size;
	u16 block_addr;
	u8 ir;

	PT_BEGIN(pt);

	// algo is the same for TCP, UDP and RAW protocols
	//

	// if exiting too soon, it will be on error
	*status = W5100_ERROR;

	// check if enough empty room
	free_size  = W5100_read(sock_offset + Sn_TX_FSR0) << 8;
	free_size += W5100_read(sock_offset + Sn_TX_FSR0) << 0;
	if (free_size < len) {
		PT_EXIT(pt);
	}

	switch (socket->prot) {
		case SOCK_STREAM:
			// IP and port already set
			break;

		case SOCK_DGRAM:
			// set destination IP
			W5100_write(S_OFFSET(sock) + Sn_DIPR0, ip_addr[3]);
			W5100_write(S_OFFSET(sock) + Sn_DIPR1, ip_addr[2]);
			W5100_write(S_OFFSET(sock) + Sn_DIPR2, ip_addr[1]);
			W5100_write(S_OFFSET(sock) + Sn_DIPR3, ip_addr[0]);

			// set destination port
			W5100_write(S_OFFSET(sock) + Sn_PORT0, (port & 0xff00) >> 8);
			W5100_write(S_OFFSET(sock) + Sn_PORT1, (port & 0x00ff) >> 0);

			// set IP and port
			break;

		case SOCK_RAW:
			// TODO
			break;

		case SOCK_NONE:
			PT_EXIT(pt);
			break;
	}

	// service is now running
	*status = W5100_RUNNING;

	// retrieve current data block write address
	block_addr  = W5100_read(sock_offset + Sn_TX_WR0) << 8;
	block_addr += W5100_read(sock_offset + Sn_TX_WR1) << 0;

	// copy data block
	W5100_write_block(block_addr, data, len, socket->tx_down_limit, socket->tx_up_limit);

	// update TX write pointer
	block_addr += len;
	if ( block_addr > socket->tx_up_limit ) {
		block_addr = socket->tx_down_limit;
	}
	W5100_write(sock_offset + Sn_TX_WR0, (block_addr & 0xff00) >> 8);
	W5100_write(sock_offset + Sn_TX_WR1, (block_addr & 0x00ff) >> 0);

	// start transmittion
	W5100_write(sock_offset + Sn_CR, CMD_SEND);

	// transmission will be finished when Sn_IR == SEND_OK | TIMEOUT
	PT_WAIT_UNTIL(pt, (ir = W5100_read(sock_offset + Sn_IR)) & (Sn_IR_SEND_OK | Sn_IR_TIMEOUT));

	if (ir & Sn_IR_SEND_OK) {
		*status = W5100_SUCCESS;
	}
	else if (ir & Sn_IR_TIMEOUT) {
		*status = W5100_ERROR;
	}

	// reset Sn_IR flags
	W5100_write(sock_offset + Sn_IR, Sn_IR_SEND_OK | Sn_IR_TIMEOUT);

	PT_END(pt);
}


static PT_THREAD( W5100_rx(pt_t* pt, W5100_status_t* status) )
{
	PT_BEGIN(pt);
	PT_END(pt);
}


// DHCP algo
//
// client port : 68
// server port : 67
static PT_THREAD( W5100_dhcp(pt_t* pt) )
{
	typedef struct {
		u8 op;				// 1 : BOOTREQUEST, 2 : BOOTREPLY
		u8 htype;			// hardware adress type : 1
		u8 hlen;			// hardware address length : 6
		u8 hops;			// 0
		u32 xid;			// transaction id : random
		u16 secs;			// seconds passed sinc client began the request process
		u16 flags;			//
		u32 ciaddr;			// client IP address : filled in by client
		u32 yiaddr;			// your client IP address : server response
		u32 siaddr;			// server IP address
		u32 giaddr;			// relay agent IP address
		u8 chaddr[16];		// client hardware agent
		u8 sname[64];		// server host name, null terminated string
		u8 file[128];		// boot file name, null terminated
		u8 options[312];	// DHCPDISCOVER, DHCPOFFER, DHCPREQUEST,...
	} DHCP_packet_t;		// due to its size, this packet has to be memory-mapped in W5100 RAM

	typedef enum {
		IP_ADDRESS_LEASE_TIME = 51,
		DHCP_MESSAGE_TYPE = 53,
	} option_t;

	typedef enum {
		DHCPDISCOVER,
		DHCPOFFER
	} DHCP_message_type_t;

	IP_address_t ip_addr = {0, 0, 0, 0};

	PT_BEGIN(pt);

	// socket #0 is already opened in UDP mode

	// set IP address to 0.0.0.0
	W5100_IP_address_set(ip_addr);

	// send DHCP_DISCOVER frame

	// wait DHCP_OFFER frame

	// reply DHCP_REQUEST frame

	// wait DHCP_ACK frame

	PT_END(pt);
}


// TCP socket handling
static PT_THREAD( W5100_tcp(pt_t* pt, socket_t* socket) )
{
	PT_BEGIN(pt);
	PT_END(pt);
}


//-----------------------------------------------------
// public functions
//


// initialization of the W5100 component
u8 W5100_init(const W5100_mode_t mode)
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
		W5100.socket[i].tx_down_limit = 0x4000 + i * 0x0800;
		W5100.socket[i].tx_up_limit = 0x4800 - 1 + i * 0x0800;
	}

	// TMSR	: 2Ko per tx socket
	W5100_write(TMSR, 0x55);
	for (i = 0; i < NB_SOCKETS; i++) {
		W5100.socket[i].rx_down_limit = 0x6000 + i * 0x0800;
		W5100.socket[i].rx_up_limit = 0x6800 - 1 + i * 0x0800;
	}

	// save mode
	W5100.mode = mode;

	// if DHCP is asked
	if (mode == W5100_DHCP) {
		// then lock every socket in UDP mode
		for (i = 0; i < NB_SOCKETS; i++) {
			(void)W5100_socket(SOCK_DGRAM);
		}
		// they will be unlocked after IP address attribution
	}

	return OK;
}


// pseudo-thread for DHCP mode
void W5100_run(void)
{
	// if the DHCP mode is enable
	if (W5100.mode == W5100_DHCP) {
		// handle it
		W5100_dhcp(&W5100.socket[0].pt);
	}
	// then continue in normal mode

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
	// except if IP address is 0.0.0.0 then gateway is also 0.0.0.0
	W5100_write(GAR0, ip_addr[0]);
	W5100_write(GAR1, ip_addr[1]);
	W5100_write(GAR2, ip_addr[2]);
	W5100_write(GAR3, ip_addr[3] ? 255 : 0);

	// Subnet Mask Register '255.255.255.0'
	// except if IP address is 0.0.0.0 then subnet is also 0.0.0.0
	W5100_write(SUBR0, ip_addr[0] ? 255 : 0);
	W5100_write(SUBR1, ip_addr[1] ? 255 : 0);
	W5100_write(SUBR2, ip_addr[2] ? 255 : 0);
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
	socket_t* socket;
	socket_state_t state;
	u8 i;

	// search for available socket
	for (i = 0; i < NB_SOCKETS; i++) {
		socket = &W5100.socket[i];

		// only a free socket can be open
		if (socket->prot != SOCK_NONE)
			continue;

		// retrieve socket state
		state = (socket_state_t)W5100_read(S_OFFSET(i) + Sn_SR);

		// if socket is closed then it is available
		if (state == SOCKET_CLOSED) {
			socket->prot = prot;
			PT_INIT(&socket->pt);
			return i;
		}
	}

	// no free socket found
	return -1;
}


// close a socket
void W5100_close(u8 sock)
{
	socket_t* socket;
	socket_state_t state;

	// retrieve socket state
	state = (socket_state_t)W5100_read(S_OFFSET(sock) + Sn_SR);

	// if socket is already closed
	socket = &W5100.socket[sock];
	if ( (state == SOCKET_CLOSED) && (socket->prot == SOCK_NONE) )
		return;

	// send close command
	W5100_write(S_OFFSET(sock) + Sn_CR, CMD_CLOSE);

	// update socket protocol
	socket->prot = SOCK_NONE;
}


// bind socket for incoming packets from ip_addr:port
u8 W5100_bind(u8 sock, IP_address_t ip_addr, u16 port)
{
	u8 buf;

	switch (W5100.socket[sock].prot) {
		case SOCK_STREAM:	// TCP (server mode)
			// set TCP mode
			W5100_write(S_OFFSET(sock) + Sn_MR, MODE_TCP);

			// set port
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

			// set port
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
			W5100_write_block(S_OFFSET(sock) + Sn_DIPR0, (u8*)&ip_addr, sizeof(IP_address_t), REGS_DOWN_LIMIT, REGS_UP_LIMIT);
			W5100_write_block(S_OFFSET(sock) + Sn_DPORT0, (u8*)&port, sizeof(u16), REGS_DOWN_LIMIT, REGS_UP_LIMIT);

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


// connect socket to host at ip_addr:port
u8 W5100_connect(u8 sock, IP_address_t ip_addr, u16 port)
{
	u8 buf;

	switch (W5100.socket[sock].prot) {
		case SOCK_STREAM:	// TCP (client mode)
			// set TCP mode
			W5100_write(S_OFFSET(sock) + Sn_MR, MODE_TCP);

			// set destination IP
			W5100_write(S_OFFSET(sock) + Sn_DIPR0, ip_addr[3]);
			W5100_write(S_OFFSET(sock) + Sn_DIPR1, ip_addr[2]);
			W5100_write(S_OFFSET(sock) + Sn_DIPR2, ip_addr[1]);
			W5100_write(S_OFFSET(sock) + Sn_DIPR3, ip_addr[0]);

			// set destination port
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
		case SOCK_RAW:		// IP RAW
		default:
			return KO;
	}

	return OK;
}


// send data
W5100_status_t W5100_sendto(u8 sock, u8* data, u8 len, IP_address_t ip_addr, u16 port)
{
	W5100_status_t status;

	W5100_tx(sock, data, len, ip_addr, port, &status);

	return status;
}


// UDP read received data if any
W5100_status_t W5100_recvfrom(u8 sock, u8* data, u8* len, IP_address_t ip_addr, u16 port)
{
	socket_t socket;
	u16 rx_size;
	u16 block_addr;
	u8 header[(UDP_HEADER_SIZE > RAW_HEADER_SIZE) ? UDP_HEADER_SIZE : RAW_HEADER_SIZE];
	u8 buf;

	socket = W5100.socket[sock];

	// check if socket is open

	// check if data is received
	buf = W5100_read(S_OFFSET(sock) + Sn_IR);
	if ( ~(buf & Sn_IR_RECV) ) {
		// no data received
		return W5100_RUNNING;
	}

	// retrieve received ddata block size
	rx_size  = W5100_read(S_OFFSET(sock) + Sn_RX_RSR0) << 8;
	rx_size += W5100_read(S_OFFSET(sock) + Sn_RX_RSR1) << 0;

	// retrieve current data block read address
	block_addr  = W5100_read(S_OFFSET(sock) + Sn_RX_RD0) << 8;
	block_addr += W5100_read(S_OFFSET(sock) + Sn_RX_RD1) << 0;

	switch (socket.prot) {
		case SOCK_STREAM:	// TCP
			// there's no specific header
			break;

		case SOCK_DGRAM:	// UDP
			// retrieve specific UDP header
			W5100_read_block(block_addr, header, UDP_HEADER_SIZE, socket.rx_down_limit, socket.rx_up_limit);

			// get received size
			*len = (header[4] << 8) + header[5];

			// update data block address
			block_addr += *len;
			break;

		case SOCK_RAW:	// IP RAW
			// retrieve specific RAW header
			W5100_read_block(block_addr, header, RAW_HEADER_SIZE, socket.rx_down_limit, socket.rx_up_limit);

			// get received size
			*len = (header[6] << 8) + header[7];

			// update data block address
			block_addr += *len;
			break;

		default:
			break;
	}

	// compute length
	*len = (*len > rx_size) ? rx_size : *len;

	// copy data block
	W5100_read_block(block_addr, data, *len, socket.rx_down_limit, socket.rx_up_limit);

	// update RX read pointer
	block_addr += *len;
	if ( block_addr > socket.rx_up_limit ) {
		block_addr = socket.rx_down_limit;
	}
	W5100_write(S_OFFSET(sock) + Sn_RX_RD0, (block_addr & 0xff00) >> 8);
	W5100_write(S_OFFSET(sock) + Sn_RX_RD1, (block_addr & 0x00ff) >> 0);

	// confirm correct reception
	W5100_write(S_OFFSET(sock) + Sn_CR, CMD_RECV);
	
	return KO;
}

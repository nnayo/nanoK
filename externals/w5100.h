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

// usage
//
// UDP client:
// 	s = socket(UDP)
// 	sendto(s, data, len, ip, port)
//
// UDP server:
// 	s = socket(UDP)
// 	recvfrom(s, data, len, ip, port)
//
// TCP client:
//  s = socket(TCP)
// 	bind(s, ip, port)
//  send(s) | recv(s)
//
// TCP server:
// 	s = socket(TCP)
// 	bind(s, ip, port)
// 	send(s, data, len) | recv(s, data, len)
//

#ifndef __W5100_H__
# define __W5100_H__

# include "type_def.h"

//-----------------------------------------------------
// public types
//

// MAC and IP address formats
typedef u8 MAC_address_t[6];
typedef u8 IP_address_t[4];


// initialization mode for IP mode choice
typedef enum {
	W5100_STATIC,
	W5100_DHCP,
} W5100_mode_t;


// socket protocol
typedef enum {
	SOCK_NONE,		// no protocol (socket is closed)
	SOCK_STREAM,	// TCP
	SOCK_DGRAM,		// UDP
	SOCK_RAW,		// RAW (ICMP)
} W5100_prot_t;


// socket action status
typedef enum {
	W5100_ERROR,
	W5100_RUNNING,
	W5100_SUCCESS,
} W5100_status_t;


//-----------------------------------------------------
// public functions
//

// initialization of the W5100 component
extern u8 W5100_init(const W5100_mode_t mode);

// pseudo-thread for DHCP mode and TCP sockets
extern void W5100_run(void);


// node IP address handling functions
extern void W5100_IP_address_set(const IP_address_t ip_addr);
extern void W5100_IP_address_get(IP_address_t ip_addr);


// create a new socket
// return its id
extern u8 W5100_socket(W5100_prot_t prot);

// close a socket
extern void W5100_close(u8 sock);


// bind socket for incoming packets from ip_addr:port
extern W5100_status_t W5100_bind(u8 sock, IP_address_t ip_addr, u16 port);

// connect socket for outgoing packets to ip_addr:port
extern W5100_status_t W5100_connect(u8 sock, IP_address_t ip_addr, u16 port);


// UDP mode: read received data if any
extern W5100_status_t W5100_recvfrom(u8 sock, u8* data, u8* len, IP_address_t ip_addr, u16 port);

// UDP mode: send data
extern W5100_status_t W5100_sendto(u8 sock, u8* data, u8 len, IP_address_t ip_addr, u16 port);


#endif	// __W5100_H__

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


#ifndef __W5100_H__
# define __W5100_H__

# include "type_def.h"

//-----------------------------------------------------
// public types
//

// MAc and IP address formats
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


//-----------------------------------------------------
// public functions
//

// initialization of the W5100 component
extern u8 W5100_init(const W5100_mode_t mode, const MAC_address_t mac_addr);

// pseudo-thread for DHCP mode
extern void W5100_run(void);


// node IP address handling functions
extern void W5100_IP_address_set(const IP_address_t ip_addr);
extern void W5100_IP_address_get(IP_address_t ip_addr);


// create a new socket
extern u8 W5100_socket(W5100_prot_t prot);

// close a socket
extern u8 W5100_close(u8 sock);


// bind socket for incoming packets from ip_addr:port
extern u8 W5100_bind(u8 sock, IP_address_t ip_addr, u16 port);

// read received data if any
extern u8 W5100_recv(u8 sock, u8* data, u8* len);


// connect socket for outgoing packets to ip_addr:port
extern u8 W5100_connect(u8 sock, IP_address_t ip_addr, u16 port);

// send data
extern u8 W5100_send(u8 sock, u8* data, u8 len);


#endif	// __W5100_H__

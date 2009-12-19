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

// W5100 chip register description
//
// this chip is from Wiznet
//
// using the SPI interface, the protocol uses a 32-bit stream
// first byte is an opcode (read or write)
// two next bytes are the address (MSB first)
// last byte is the data field (read or written)
//

#ifndef __W5100_INTERNALS_H__
# define __W5100_INTERNALS_H__

# include "type_def.h"

//------------------------------------------
// private defines
//

# define NB_SOCKETS	4


// common registers
//

// mode
# define	MR		((u16)0x0000)

// gateway address
# define	GAR0	((u16)0x0001)
# define	GAR1	((u16)0x0002)
# define	GAR2	((u16)0x0003)
# define	GAR3	((u16)0x0004)

// subnet mask address
# define	SUBR0	((u16)0x0005)
# define	SUBR1	((u16)0x0006)
# define	SUBR2	((u16)0x0007)
# define	SUBR3	((u16)0x0008)

// source hardware address
# define	SHAR0	((u16)0x0009)
# define	SHAR1	((u16)0x000a)
# define	SHAR2	((u16)0x000b)
# define	SHAR3	((u16)0x000c)
# define	SHAR4	((u16)0x000d)
# define	SHAR5	((u16)0x000e)

// source IP address
# define	SIPR0	((u16)0x000f)
# define	SIPR1	((u16)0x0010)
# define	SIPR2	((u16)0x0011)
# define	SIPR3	((u16)0x0012)

// reserved
// 0x0013 - 0x0014

// interrupt
# define	IR		((u16)0x0015)

// interrupt mask
# define	IMR		((u16)0x0016)

// retry time
# define	RTR0	((u16)0x0017)
# define	RTR1	((u16)0x0018)

// retry counter
# define	RCR		((u16)0x0019)

// rx memory size
# define	RMSR	((u16)0x001a)

// tx memory size
# define	TMSR	((u16)0x001b)

// authentication type in PPPoE
# define	PATR0	((u16)0x001c)
# define	PATR1	((u16)0x001d)

// reserved
// 0x001e - 0x0027

// PPP LCP request timer
# define	PTIMER	((u16)0x0028)

// PPP LCP magic number
# define	PMAGIC	((u16)0x0029)

// unreachable IP address
# define	UIPR0	((u16)0x002a)
# define	UIPR1	((u16)0x002b)
# define	UIPR2	((u16)0x002c)
# define	UIPR3	((u16)0x002d)

// unreachable port
# define	UPORT0	((u16)0x002e)
# define	UPORT1	((u16)0x002f)

// reserved
// 0x0030 - 0x03ff

// socket offset
#define S0_OFFSET	((u16)0x0400)
#define S1_OFFSET	((u16)0x0500)
#define S2_OFFSET	((u16)0x0600)
#define S3_OFFSET	((u16)0x0700)


// socket registers (without offset)
//

// socket n mode
# define	Sn_MR		((u16)0x0000)

// socket n command
# define	Sn_CR		((u16)0x0001)

// socket n interrupt
# define	Sn_IR		((u16)0x0002)

// socket n status
# define	Sn_SR		((u16)0x0003)

// socket n source port
# define	Sn_PORT0	((u16)0x0004)
# define	Sn_PORT1	((u16)0x0005)

// socket n destination hardware address
# define	Sn_DHAR0	((u16)0x0006)
# define	Sn_DHAR1	((u16)0x0007)
# define	Sn_DHAR2	((u16)0x0008)
# define	Sn_DHAR3	((u16)0x0009)
# define	Sn_DHAR4	((u16)0x000a)
# define	Sn_DHAR5	((u16)0x000b)

// socket n destination IP address
# define	Sn_DIPR0	((u16)0x000c)
# define	Sn_DIPR1	((u16)0x000d)
# define	Sn_DIPR2	((u16)0x000e)
# define	Sn_DIPR3	((u16)0x000f)

// socket n destination port
# define	Sn_DPORT0	((u16)0x0010)
# define	Sn_DPORT1	((u16)0x0011)

// socket n maximum segment size
# define	Sn_MSSR0	((u16)0x0012)
# define	Sn_MSSR1	((u16)0x0013)

// socket n protocol in IP raw mode
# define	Sn_PROTO	((u16)0x0014)

// socket n IP TOS
# define	Sn_TOS		((u16)0x0015)

// socket n IP TTL
# define	Sn_TTL		((u16)0x0016)

// reserved
// 0x0017 - 0x001f

// socket n TX free size
# define	Sn_TX_FSR0	((u16)0x0020)
# define	Sn_TX_FSR1	((u16)0x0021)

// socket n TX read pointer
# define	Sn_TX_RD0	((u16)0x0022)
# define	Sn_TX_RD1	((u16)0x0023)

// socket n TX write pointer
# define	Sn_TX_WR0	((u16)0x0024)
# define	Sn_TX_WR1	((u16)0x0025)

// socket n RX received size
# define	Sn_RX_RSR0	((u16)0x0026)
# define	Sn_RX_RSR1	((u16)0x0027)

// socket n RX read pointer
# define	Sn_RX_RD0	((u16)0x0028)
# define	Sn_RX_RD1	((u16)0x0029)

// reserved
// 0x002a - 0x00ff


//------------------------------------------
// private types
//

typedef enum {
	W5100_WRITE	= 0xf0,
	W5100_READ	= 0x0f,
} w5100_opcode_t;

typedef struct {
	w5100_opcode_t opcode;
	u16 addr;
	u8 data;
} w5100_stream_t;

typedef enum {
	W5100_CLOSED,
} socket_state_t;

#endif	// __W5100_INTERNALS_H__

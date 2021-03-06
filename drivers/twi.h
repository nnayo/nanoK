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

#ifndef __NNK_TWI_H__
# define __NNK_TWI_H__

# include "type_def.h"

# define NNK_TWI_BROADCAST_ADDR	0x00
# define NNK_TWI_FIRST_ADDR		0x01
# define NNK_TWI_LAST_ADDR		0x7f

enum nnk_twi_state {
	NNK_TWI_IDLE,		// just idle
	NNK_TWI_NO_SL,		// no slave at the given address
	NNK_TWI_MS_RX_BEGIN,	// master mode, reception from slave beginning
	NNK_TWI_MS_RX_END,		// master mode, reception from slave finished
	NNK_TWI_MS_TX_BEGIN,	// master mode, transmission to slave beginning
	NNK_TWI_MS_TX_END,		// master mode, transmission to slave finished
	NNK_TWI_SL_RX_BEGIN,	// slave mode, reception from master beginning
	NNK_TWI_SL_RX_END,		// slave mode, reception from master finished
	NNK_TWI_SL_TX_BEGIN,	// slave mode, transmission to master beginning
	NNK_TWI_SL_TX_END,		// slave mode, transmission to master finished
	NNK_TWI_GENCALL_BEGIN,	// general call, reception from master beginning
	NNK_TWI_GENCALL_END,	// general call, reception from master finished
	NNK_TWI_ERROR		// error in the protocol or in the state machine
};	// automata states

// init the TWI component
//
// by default, address recognition is disabled
// this means the TWI is able to send data
// but not to receive
// general call answer shall be enable
// and/or a slave address shall be provided
//
// its parameters are:
// - call_back : a function pointer (see below for for details)
// - misc : the misc pointer allows to pass an extra parameter to the callback function
//          if it is useless, set it to NULL.
//
// the call_back function will be called to signal:
// - end of transmission,
// - end of reception,
// - error in protocol (such as slave not responding,...).
//
// the responsability of the call_back function is to:
// - stop the communication,
// - restart an other communication,
// - handle the errors.
// (see at end of file for more details)
//
// its parameters are:
// - nb_data : number of data effectively read or sent
// - state : current com state
// - misc : extra parameter (up to your needs)
//
extern void nnk_twi_init(void(*call_back)(enum nnk_twi_state state, u8 nb_data, void* misc), void* misc);

// set the I2C address
// setting the address to zero means setting the TWI as master
// as it will respond only to general call if enabled.
//
extern void nnk_twi_sl_addr_set(u8 sl_addr);

// get the I2C address
//
extern u8 nnk_twi_sl_addr_get(void);

// enable (TRUE) or disable (FALSE) the general call address recognition
//
extern void nnk_twi_gen_call(u8 gen_call);

// shall be used only in the call_back function to stop the communication
//
extern void nnk_twi_stop(void);

// master comm
// 
// send data to or receive data from slave.
// the transmission immediatly starts.
// 
// adr : slave I2C address
// len : length of the data buffer to send or of the number of data to read
// data : data buffer belonging to the caller
//
// if a master comm failed due to a lost arbitration,
// this comm will be resumed when the bus will become free
//
// if comm is impossible, it returns KO else OK
//
extern u8 nnk_twi_ms_tx(u8 adr, u8 len, u8* data);
extern u8 nnk_twi_ms_rx(u8 adr, u8 len, u8* data);

// slave comm
//
// when being addressed as slave, those functions are to be
// used for sending data to or reading data from the bus master.
//
// len : length of the data buffer
// data : data buffer belonging to the caller
//
// if comm is impossible, it returns KO else OK
//
extern u8 nnk_twi_sl_tx(u8 len, u8* data);
extern u8 nnk_twi_sl_rx(u8 len, u8* data);

// general call
//
// when addressed in general call,
// it is only possible to receive data
// so use nnk_twi_sl_rx with same parameters
//

// more details on the requirements for the call back function
//
// what to do in the call_back is called upon the nnk_twi_state and your will.
//
// here are descripted all the possible cases depending on the nnk_twi_state
//
// - NNK_TWI_IDLE:
// 	will never happened, it is just an internal state
// 	-> nothing to do then!!!
//
// - NNK_TWI_NO_SL:
// 	no slave at given address or no slave able to answer general call
// 	-> stop communication:	nnk_twi_stop()
// 	-> retry sending data:	nnk_twi_ms_tx()
// 	-> retry reading data:	nnk_twi_ms_rx()
//
// - NNK_TWI_MS_RX_END:
// 	reception as master finished
// 	-> stop communication:	nnk_twi_stop()
// 	-> try sending data:	nnk_twi_ms_tx()
// 	-> try reading data:	nnk_twi_ms_rx()
//
// - NNK_TWI_MS_TX_END:
// 	transmission as master finished
// 	-> stop communication:	nnk_twi_stop()
// 	-> try sending data:	nnk_twi_ms_tx()
// 	-> try reading data:	nnk_twi_ms_rx()
//
// - NNK_TWI_SL_RX_BEGIN:
// 	reception as slave beginning (may be due to an arbitration lost)
// 	-> stop communication:	nnk_twi_stop()
// 	-> accept reading data:	nnk_twi_sl_rx()
//
// - NNK_TWI_SL_RX_END:
// 	reception as slave finished
// 	-> stop communication:	nnk_twi_stop()
// 	-> try sending data:	nnk_twi_ms_tx()
// 	-> try reading data:	nnk_twi_ms_rx()
//
// - NNK_TWI_SL_TX_BEGIN:
// 	transmission as slave beginning (may be due to an arbitration lost)
// 	-> stop communication:	nnk_twi_stop()
// 	-> accept reading data:	nnk_twi_sl_rx()
//
// - NNK_TWI_SL_TX_END:
// 	transmission as slave finished
// 	-> try sending data:	nnk_twi_ms_tx()
// 	-> try reading data:	nnk_twi_ms_rx()
//
// - NNK_TWI_GENCALL_BEGIN:
// 	reception as slave (in the context of a general call) beginning (may be due to an arbitration lost)
// 	-> stop communication:	nnk_twi_stop()
// 	-> accept reading data:	nnk_twi_sl_rx()
//
// - NNK_TWI_GENCALL_END:
// 	reception as slave (in the context of a general call) finished
// 	-> stop communication:	nnk_twi_stop()
// 	-> try sending data:	nnk_twi_ms_tx()
// 	-> try reading data:	nnk_twi_ms_rx()
//
// - NNK_TWI_ERROR:
// 	error in protocol at hardware level (probably)
// 	nothing to do except trying to stop the current transfert
// 	-> stop communication:	nnk_twi_stop()
//

#endif

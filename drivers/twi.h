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

#ifndef __TWI_H__
# define __TWI_H__

# include "type_def.h"

# define TWI_BROADCAST_ADDR	0x00
# define TWI_FIRST_ADDR		0x01
# define TWI_LAST_ADDR		0x7f

typedef enum {
	TWI_IDLE,		// just idle
	TWI_NO_SL,		// no slave at the given address
	TWI_MS_RX_BEGIN,	// master mode, reception from slave beginning
	TWI_MS_RX_END,		// master mode, reception from slave finished
	TWI_MS_TX_BEGIN,	// master mode, transmission to slave beginning
	TWI_MS_TX_END,		// master mode, transmission to slave finished
	TWI_SL_RX_BEGIN,	// slave mode, reception from master beginning
	TWI_SL_RX_END,		// slave mode, reception from master finished
	TWI_SL_TX_BEGIN,	// slave mode, transmission to master beginning
	TWI_SL_TX_END,		// slave mode, transmission to master finished
	TWI_GENCALL_BEGIN,	// general call, reception from master beginning
	TWI_GENCALL_END,	// general call, reception from master finished
	TWI_ERROR		// error in the protocol or in the state machine
} twi_state_t;	// automata states

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
extern void TWI_init(void(*call_back)(twi_state_t state, u8 nb_data, void* misc), void* misc);

// set the I2C address
// setting the address to zero means setting the TWI as master
// as it will respond only to general call if enabled.
//
extern void TWI_set_sl_addr(u8 sl_addr);

// get the I2C address
//
extern u8 TWI_get_sl_addr(void);

// enable (TRUE) or disable (FALSE) the general call address recognition
//
extern void TWI_gen_call(u8 gen_call);

// shall be used only in the call_back function to stop the communication
//
extern void TWI_stop(void);

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
extern u8 TWI_ms_tx(u8 adr, u8 len, u8* data);
extern u8 TWI_ms_rx(u8 adr, u8 len, u8* data);

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
extern u8 TWI_sl_tx(u8 len, u8* data);
extern u8 TWI_sl_rx(u8 len, u8* data);

// general call
//
// when addressed in general call,
// it is only possible to receive data
// so use TWI_sl_rx with same parameters
//

// more details on the requirements for the call back function
//
// what to do in the call_back is called upon the twi_state and your will.
//
// here are descripted all the possible cases depending on the twi_state
//
// - twi_idle:
// 	will never happened, it is just an internal state
// 	-> nothing to do then!!!
//
// - twi_no_sl:
// 	no slave at given address or no slave able to answer general call
// 	-> stop communication:	TWI_stop()
// 	-> retry sending data:	TWI_ms_tx()
// 	-> retry reading data:	TWI_ms_rx()
//
// - twi_ms_rx_end:
// 	reception as master finished
// 	-> stop communication:	TWI_stop()
// 	-> try sending data:	TWI_ms_tx()
// 	-> try reading data:	TWI_ms_rx()
//
// - twi_ms_tx_end:
// 	transmission as master finished
// 	-> stop communication:	TWI_stop()
// 	-> try sending data:	TWI_ms_tx()
// 	-> try reading data:	TWI_ms_rx()
//
// - twi_sl_rx_begin:
// 	reception as slave beginning (may be due to an arbitration lost)
// 	-> stop communication:	TWI_stop()
// 	-> accept reading data:	TWI_sl_rx()
//
// - twi_sl_rx_end:
// 	reception as slave finished
// 	-> stop communication:	TWI_stop()
// 	-> try sending data:	TWI_ms_tx()
// 	-> try reading data:	TWI_ms_rx()
//
// - twi_sl_tx_begin:
// 	transmission as slave beginning (may be due to an arbitration lost)
// 	-> stop communication:	TWI_stop()
// 	-> accept reading data:	TWI_sl_rx()
//
// - twi_sl_tx_end:
// 	transmission as slave finished
// 	-> try sending data:	TWI_ms_tx()
// 	-> try reading data:	TWI_ms_rx()
//
// - twi_gencall_begin:
// 	reception as slave (in the context of a general call) beginning (may be due to an arbitration lost)
// 	-> stop communication:	TWI_stop()
// 	-> accept reading data:	TWI_sl_rx()
//
// - twi_gencall_end:
// 	reception as slave (in the context of a general call) finished
// 	-> stop communication:	TWI_stop()
// 	-> try sending data:	TWI_ms_tx()
// 	-> try reading data:	TWI_ms_rx()
//
// - twi_error:
// 	error in protocol at hardware level (probably)
// 	nothing to do except trying to stop the current transfert
// 	-> stop communication:	TWI_stop()
//

#endif

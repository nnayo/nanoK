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

#include "twi.h"

#include <compat/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>	// ISR()


#define TWI_PRESCALER_1		0x00
#define TWI_PRESCALER_4		0x01
#define TWI_PRESCALER_16	0x02
#define TWI_PRESCALER_64	0x03

#define TWI_ACK			1
#define TWI_NACK		0

//-------------------
// private variables
//

static volatile struct {
	twi_state_t state;	// software state (a synthesis of the hardware state)
	u8 addr;		// address to send to or to read from
	u8 nb_data;		// number of data received or transmitted

	u8 ms_buf_len;		// length of the master buffer
	u8* ms_buf;		// pointer to the master buffer

	u8 sl_buf_len;		// length of the slave buffer
	u8* sl_buf;		// pointer to the slave buffer

	void (*call_back)(twi_state_t state, u8 nb_data, void* misc);
				// pointer to function to be called on
				// transfert events or on error
	void* misc;		// a miscellaneous pointer (can hold what ever you want)
} TWI;

//-------------------
// private functions
//

static void TWI_default_call_back(twi_state_t state, u8 nb_data, void* misc)
{
	// ignore the parameters
	(void)state;
	(void)nb_data;
	(void)misc;

	// just stop the communication
	TWI_stop();
}

static void TWI_start(void)
{

	// try to generate a START
	TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWSTA) | _BV(TWEN) | _BV(TWIE);

	// clear TWSTA bit
	// will be done by next write to TWCR
}

static void TWI_ack(void)
{
	TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
}

static void TWI_nack(void)
{
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
}

static u8 TWI_read(void)
{
	return TWDR;
}

static void TWI_write(u8 data)
{
	TWDR = data;
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
}

static void TWI_call_back_call(twi_state_t state)
{
	TWI.state = state;
	(*TWI.call_back)(state, TWI.nb_data, TWI.misc);
}

// TWI_isr
ISR(TWI_vect)
{
	static const void* const status_action[] PROGMEM = {
		&&_TW_BUS_ERROR,		// 0x00
		&&_TW_START,			// 0x08
		&&_TW_REP_START,		// 0x10
		&&_TW_MT_SLA_ACK,		// 0x18
		&&_TW_MT_SLA_NACK,		// 0x20
		&&_TW_MT_DATA_ACK,		// 0x28
		&&_TW_MT_DATA_NACK,		// 0x30
		// &&_TW_MT_ARB_LOST,		// 0x38
		&&_TW_MR_ARB_LOST,		// 0x38
		&&_TW_MR_SLA_ACK,		// 0x40
		&&_TW_MR_SLA_NACK,		// 0x48
		&&_TW_MR_DATA_ACK,		// 0x50
		&&_TW_MR_DATA_NACK,		// 0x58
		&&_TW_SR_SLA_ACK,		// 0x60
		&&_TW_SR_ARB_LOST_SLA_ACK,	// 0x68
		&&_TW_SR_GCALL_ACK,		// 0x70
		&&_TW_SR_ARB_LOST_GCALL_ACK,	// 0x78
		&&_TW_SR_DATA_ACK,		// 0x80
		&&_TW_SR_DATA_NACK,		// 0x88
		&&_TW_SR_GCALL_DATA_ACK,	// 0x90
		&&_TW_SR_GCALL_DATA_NACK,	// 0x98
		&&_TW_SR_STOP,			// 0xa0
		&&_TW_ST_SLA_ACK,		// 0xa8
		&&_TW_ST_ARB_LOST_SLA_ACK,	// 0xb0
		&&_TW_ST_DATA_ACK,		// 0xb8
		&&_TW_ST_DATA_NACK,		// 0xc0
		&&_TW_ST_LAST_DATA,		// 0xc8
	};

	u8 buf;
	void* action;

	// check status
	buf = TW_STATUS;

	if ( buf == TW_NO_INFO )	// 0xf8
		goto _TW_NO_INFO;

	if ( buf > TW_ST_LAST_DATA )	// unknown value
		goto other;

	// the TWI status is only the 5 MSB bits of the register
	// shifting it, makes it fit the jump table 
	buf = buf >> 3;

	// jump to the routine associated to the TWI status
	// goto *status_action[buf]; can't be done directly, since the table is in flash
	action = (void*)pgm_read_word(&(status_action[buf]));
	goto *action;

	// Master
	_TW_START:			// 0x08 : start
	_TW_REP_START:			// 0x10 : repeated start
		// start or restart correctly generated

		// reset nb_data to zero
		TWI.nb_data = 0;
		
		// send slave addr
		TWI_write(TWI.addr);
		return;

	// -----------------------------------------------------------------------
	// Master transmitter
	_TW_MT_SLA_ACK:			// 0x18 : slave addr ack
		// slave present and ready to receive

		// if more data to send
		if ( TWI.nb_data < TWI.ms_buf_len ) {
			buf = TWI.nb_data;
			TWI.nb_data++;
			TWI_write( *(TWI.ms_buf + buf) );
		}
		// else transmission is finish
		else {
			// reset len to prevent data resending
			TWI.ms_buf_len = 0;

			TWI_call_back_call(TWI_MS_TX_END);
		}

		return;

	_TW_MT_SLA_NACK:		// 0x20 : slave addr nack
		// slave not present at given address

		// reset len to prevent data resending
		TWI.ms_buf_len = 0;

		TWI_call_back_call(TWI_NO_SL);

		return;

	_TW_MT_DATA_ACK:		// 0x28 : slave data ack
		// slave can receive more data

		// if more data to send
		if ( TWI.nb_data < TWI.ms_buf_len ) {
			buf = TWI.nb_data;
			TWI.nb_data++;
			TWI_write(*(TWI.ms_buf + buf));
		}
		// else transmission is finish
		else {
			// reset len to prevent data resending
			TWI.ms_buf_len = 0;

			TWI_call_back_call(TWI_MS_TX_END);
		}

		return;

	_TW_MT_DATA_NACK:		// 0x30 : slave data nack
		// transmission finish from slave point of view or slave disconnected

		// reset len to prevent data resending
		TWI.ms_buf_len = 0;

		// signal it to the application
		TWI_call_back_call(TWI_MS_TX_END);

		return;

//	_TW_MT_ARB_LOST: // same value and same action as following case

	// -----------------------------------------------------------------------
	// Master receiver
	_TW_MR_ARB_LOST:		// 0x38 : arbitration lost
		// arbitration lost

		// retry to generate a START when the bus becomes free
		TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWSTA) | _BV(TWEN) | _BV(TWIE);

		return;

	_TW_MR_SLA_ACK:			// 0x40 : slave addr ack
		// slave present and ready to send

		// if more than 1 data to received
		if ( (TWI.nb_data + 1) < TWI.ms_buf_len ) {
			TWI_ack();
		}
		// else nack the next data as it is the only one
		else {
			TWI_nack();
		}

		return;

	_TW_MR_SLA_NACK:		// 0x48 : slave addr nack
		// slave not present

		// reset len to prevent data resending
		TWI.ms_buf_len = 0;

		// signal it to the application
		TWI_call_back_call(TWI_NO_SL);

		return;

	_TW_MR_DATA_ACK:		// 0x50 : slave data ack
		// one more byte received from slave

		// store it
		*(TWI.ms_buf + TWI.nb_data) = TWI_read();
		TWI.nb_data++;

		// if more data to receive
		if ( (TWI.nb_data + 1) < TWI.ms_buf_len ) {
			TWI_ack();
		}
		// else about to receive the last data
		else {
			TWI_nack();
		}

		return;

	_TW_MR_DATA_NACK:		// 0x58 : slave data nack
		// last byte received

		// store it
		*(TWI.ms_buf + TWI.nb_data) = TWI_read();
		TWI.nb_data++;

		// reset len to prevent data resending
		TWI.ms_buf_len = 0;

		// no more data to read (reception finished)
		TWI_call_back_call(TWI_MS_RX_END);

		return;

	// -----------------------------------------------------------------------
	// Slave transmitter
	_TW_ST_SLA_ACK:			// 0xa8 : own slave addr + R
	_TW_ST_ARB_LOST_SLA_ACK:	// 0xb0 : arb lost as master, own slave addr + R
		// adressed as slave (whether or not the arbitration was lost)

		// reset nb_data to zero
		TWI.nb_data = 0;

		// need support from application
		// a buffer to transmit shall be provided
		TWI_call_back_call(TWI_SL_TX_BEGIN);

		// send data if any
		if ( TWI.nb_data < TWI.sl_buf_len ) {
			buf = TWI.nb_data;
			TWI.nb_data++;
			TWI_write( *(TWI.sl_buf + buf) );
			TWI_ack();
		}
		else {	// send dummy value
			TWI_write(0x33);
			TWI_nack();
		}

		return;

	_TW_ST_DATA_ACK:		// 0xb8 : byte transmit, ack received
		// byte sent to the master and it wants more
		
		// send data if any
		if ( TWI.nb_data < TWI.sl_buf_len ) {
			buf = TWI.nb_data;
			TWI.nb_data++;
			TWI_write( *(TWI.sl_buf + buf) );
			TWI_ack();
		}
		else {	// send dummy value
			TWI_write(0x33);
			TWI_nack();
		}

		return;

	_TW_ST_DATA_NACK:		// 0xc0 : byte transmit, nack received
	_TW_ST_LAST_DATA:		// 0xc8 : last data transmit, ack received
		// master nack, so tx is finished

		// signal it to the application
		TWI_call_back_call(TWI_SL_TX_END);

		// if we have data to send or receive as master
		if (TWI.ms_buf_len != 0) {
			// generate a restart
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
		}
		else {
			// give up bus control, wait for next start
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
		}

		return;

	// -----------------------------------------------------------------------
	// Slave reveicer
	_TW_SR_SLA_ACK:			// 0x60 : own slave addr + W
	_TW_SR_ARB_LOST_SLA_ACK:	// 0x68 : arb lost, own slave addr +W
		// addressed as slave for rx (whether or not the arbitration was lost)

		// reset nb_data to zero
		TWI.nb_data = 0;

		// need support from application
		// a buffer to received the data shall be provided
		TWI_call_back_call(TWI_SL_RX_BEGIN);

		// if more data to receive
		if ( (TWI.nb_data + 1) < TWI.sl_buf_len ) {
			TWI_ack();
		}
		// else about to receive the last data
		else {
			TWI_nack();
		}

		return;

	_TW_SR_GCALL_ACK:		// 0x70 : gen call received, ack sent
	_TW_SR_ARB_LOST_GCALL_ACK:	// 0x78 : arb lost + idem above
		// general call received (whether or not the arbitration was lost)

		// reset nb_data to zero
		TWI.nb_data = 0;

		// need support from application
		// a buffer to received the data shall be provided
		TWI_call_back_call(TWI_GENCALL_BEGIN);

		// if more data to receive
		if ( (TWI.nb_data + 1) < TWI.sl_buf_len ) {
			TWI_ack();
		}
		// else about to receive the last data
		else {
			TWI_nack();
		}

		return;

	_TW_SR_DATA_ACK:		// 0x80 : data received, ack sent
		// one more byte received and more to come

		// store it
		*(TWI.sl_buf + TWI.nb_data) = TWI_read();
		TWI.nb_data++;

		// if more data to receive
		if ( (TWI.nb_data + 1) < TWI.sl_buf_len ) {
			TWI_ack();
		}
		// else about to receive the last data
		else {
			TWI_nack();
		}

		return;

	_TW_SR_DATA_NACK:		// 0x88 : data received, nack sent
		// last byte received

		// store it
		*(TWI.sl_buf + TWI.nb_data) = TWI_read();
		TWI.nb_data++;

		// signal end of rx
		TWI_call_back_call(TWI_SL_RX_END);

		return;

	_TW_SR_GCALL_DATA_ACK:		// 0x90 : in gen call, data received, ack sent
		// one more byte received and more to come

		// store it
		*(TWI.sl_buf + TWI.nb_data) = TWI_read();
		TWI.nb_data++;

		// if more data to receive
		if ( (TWI.nb_data + 1) < TWI.sl_buf_len ) {
			TWI_ack();
		}
		// else about to receive the last data
		else {
			TWI_nack();
		}

		return;

	_TW_SR_GCALL_DATA_NACK:		// 0x98 : in gen call, data received, nack sent
		// last byte received

		// store it
		*(TWI.sl_buf + TWI.nb_data) = TWI_read();
		TWI.nb_data++;

		// signal end of rx
		TWI_call_back_call(TWI_GENCALL_END);

		return;

	_TW_SR_STOP:			// 0xa0 : stop or restart received
		// the previous transfer as slace receiver (general call or not)
		// is completely finished
		// and it has already been notified to the application

		// but if we have data to send or read as master
		// try to generate a restart
		if (TWI.ms_buf_len != 0) {
			// generate a restart
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
		}
		else {
			// give up bus control, wait for next start
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
		}

		return;

	// -----------------------------------------------------------------------
	// Error cases
	_TW_NO_INFO:			// 0xf8 : no relevant information, should never happened in INT mode
		// if it should never happen, it WILL NEVER HAPPEN

		return;

	_TW_BUS_ERROR:			// 0x00 : internal protocol violation
		// hardware error (may I mess the registers!?!? :-) )

		// recover releasing the bus
		//TWCR = _BV(TWEA) | _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
		TWI_call_back_call(TWI_ERROR);

		return;

	other:				// unknown status
		// so, what happens?????

		TWI_call_back_call(TWI_ERROR);

		return;
}


//-------------------
// public functions
//

void TWI_init(void(*call_back)(twi_state_t state, u8 nb_data, void* misc), void* misc)
{
	// switch off TWI interface
	TWCR = 0x00;

	// bit rate 100k
	// prescaler = 1
	TWSR = TWI_PRESCALER_1;

	// baud rate = 32
	TWBR = 32;

	// reset engine state to IDLE
	TWI.state = TWI_IDLE;

	// set call_back pointer
	if ( call_back != NULL )
		TWI.call_back = call_back;
	else
		TWI.call_back = TWI_default_call_back;

	// set the misc pointer
	TWI.misc = misc;

	// enable automatic ACK bit and TWI interface in interrupt mode
	TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE);

	// disable address recognition
	TWAR = 0;
}


void TWI_set_sl_addr(u8 sl_addr)
{
	TWAR &= _BV(TWGCE);		// reset the TWAR except the general call recognition bit
	TWAR |= sl_addr << 1;		// set the slave address we shall respond to
}


u8 TWI_get_sl_addr(void)
{
	return TWAR >> 1;		// get the slave address we're responding to
}


void TWI_gen_call(u8 gen_call)
{
	// if enabled
	if (gen_call) {
		TWAR |= _BV(TWGCE);	// set the general call recognition bit
	}
	else {	// disabled
		TWAR &= ~_BV(TWGCE);	// reset the general call recognition bit
	}
}


void TWI_stop(void)
{
	// if hard is not already IDLE
	if (TW_STATUS != 0xf8) {
		// try to generate a STOP
		TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWSTO) | _BV(TWIE);

		// in master mode, when the STOP is executed
		// on the bus, the TWSTO bit is cleared
		// automatically.
	}

	// reset engine state to IDLE
	TWI.state = TWI_IDLE;
}


u8 TWI_ms_tx(u8 adr, u8 len, u8* data)
{
	// check if TWI is idle
	if ( (TWI.state != TWI_IDLE) && (TWI.state != TWI_ERROR) )
		return KO;

	// reset TWI state
	TWI.state = TWI_MS_TX_BEGIN;

	// set ms_buf
	TWI.ms_buf = data;
	TWI.ms_buf_len = len;

	// set addr with write bit
	TWI.addr = (adr << 1) | TW_WRITE;

	// begin transmission
	TWI_start();

	return OK;
}


u8 TWI_ms_rx(u8 adr, u8 len, u8* data)
{
	// check if TWI is idle
	if ( (TWI.state != TWI_IDLE) && (TWI.state != TWI_ERROR) )
		return KO;

	// reset TWI state
	TWI.state = TWI_MS_RX_BEGIN;

	// set rx_buf
	TWI.ms_buf = data;
	TWI.ms_buf_len = len;

	// set addr with read bit
	TWI.addr = (adr << 1) | TW_READ;

	// begin transmission
	TWI_start();

	return OK;
}


u8 TWI_sl_tx(u8 len, u8* data)
{
	// check if TWI is slave transmitting
	if (TWI.state != TWI_SL_TX_BEGIN)
		return KO;

	// set sl_buf
	TWI.sl_buf = data;
	TWI.sl_buf_len = len;

	// transmission will be continued by the master
	// on its will

	return OK;
}


u8 TWI_sl_rx(u8 len, u8* data)
{
	// check if TWI is slave receiving
	// general call is just a special case of slave receiving
	if ( (TWI.state != TWI_SL_RX_BEGIN) && (TWI.state != TWI_GENCALL_BEGIN) )
		return KO;

	// set sl_buf
	TWI.sl_buf = data;
	TWI.sl_buf_len = len;

	// reception will be continued by the master
	// on its will

	return OK;
}

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


#define NNK_TWI_PRESCALER_1		0x00
#define NNK_TWI_PRESCALER_4		0x01
#define NNK_TWI_PRESCALER_16	0x02
#define NNK_TWI_PRESCALER_64	0x03

#define NNK_TWI_ACK			1
#define NNK_TWI_NACK		0

//-------------------
// private variables
//

static volatile struct {
        enum nnk_twi_state state;	// software state (a synthesis of the hardware state)
        u8 addr;		// address to send to or to read from
        u8 nb_data;		// number of data received or transmitted

        u8 ms_buf_len;		// length of the master buffer
        u8* ms_buf;		// pointer to the master buffer

        u8 sl_buf_len;		// length of the slave buffer
        u8* sl_buf;		// pointer to the slave buffer

        void (*call_back)(enum nnk_twi_state state, u8 nb_data, void* misc);
        // pointer to function to be called on
        // transfert events or on error
        void* misc;		// a miscellaneous pointer (can hold what ever you want)
} twi;

//-------------------
// private functions
//

static void nnk_twi_default_call_back(enum nnk_twi_state state, u8 nb_data, void* misc)
{
        // ignore the parameters
        (void)state;
        (void)nb_data;
        (void)misc;

        // just stop the communication
        nnk_twi_stop();
}

static void nnk_twi_start(void)
{

        // try to generate a START
        TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWSTA) | _BV(TWEN) | _BV(TWIE);

        // clear TWSTA bit
        // will be done by next write to TWCR
}

static void nnk_twi_ack(void)
{
        TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
}

static void nnk_twi_nack(void)
{
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
}

static u8 nnk_twi_read(void)
{
        return TWDR;
}

static void nnk_twi_write(u8 data, u8 ack)
{
        TWDR = data;
        if (ack) {
                TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
        }
        else {
                TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        }
}

static void nnk_twi_call_back_call(enum nnk_twi_state state)
{
        twi.state = state;
        (*twi.call_back)(state, twi.nb_data, twi.misc);
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
        twi.nb_data = 0;

        // send slave addr
        nnk_twi_write(twi.addr, OK);
        return;

        // -----------------------------------------------------------------------
        // Master transmitter
_TW_MT_SLA_ACK:			// 0x18 : slave addr ack
        // slave present and ready to receive

        // if more data to send
        if ( twi.nb_data < twi.ms_buf_len ) {
                buf = twi.nb_data;
                twi.nb_data++;
                nnk_twi_write( *(twi.ms_buf + buf), OK);
        }
        // else transmission is finish
        else {
                // reset len to prevent data resending
                twi.ms_buf_len = 0;

                nnk_twi_call_back_call(NNK_TWI_MS_TX_END);
        }

        return;

_TW_MT_SLA_NACK:		// 0x20 : slave addr nack
        // slave not present at given address

        // reset len to prevent data resending
        twi.ms_buf_len = 0;

        nnk_twi_call_back_call(NNK_TWI_NO_SL);

        return;

_TW_MT_DATA_ACK:		// 0x28 : slave data ack
        // slave can receive more data

        // if more data to send
        if ( twi.nb_data < twi.ms_buf_len ) {
                buf = twi.nb_data;
                twi.nb_data++;
                nnk_twi_write( *(twi.ms_buf + buf), OK);
        }
        // else transmission is finish
        else {
                // reset len to prevent data resending
                twi.ms_buf_len = 0;

                nnk_twi_call_back_call(NNK_TWI_MS_TX_END);
        }

        return;

_TW_MT_DATA_NACK:		// 0x30 : slave data nack
        // transmission finish from slave point of view or slave disconnected

        // reset len to prevent data resending
        twi.ms_buf_len = 0;

        // signal it to the application
        nnk_twi_call_back_call(NNK_TWI_MS_TX_END);

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
        if ( (twi.nb_data + 1) < twi.ms_buf_len ) {
                nnk_twi_ack();
        }
        // else nack the next data as it is the only one
        else {
                nnk_twi_nack();
        }

        return;

_TW_MR_SLA_NACK:		// 0x48 : slave addr nack
        // slave not present

        // reset len to prevent data resending
        twi.ms_buf_len = 0;

        // signal it to the application
        nnk_twi_call_back_call(NNK_TWI_NO_SL);

        return;

_TW_MR_DATA_ACK:		// 0x50 : slave data ack
        // one more byte received from slave

        // store it
        *(twi.ms_buf + twi.nb_data) = nnk_twi_read();
        twi.nb_data++;

        // if more data to receive
        if ( (twi.nb_data + 1) < twi.ms_buf_len ) {
                nnk_twi_ack();
        }
        // else about to receive the last data
        else {
                nnk_twi_nack();
        }

        return;

_TW_MR_DATA_NACK:		// 0x58 : slave data nack
        // last byte received

        // store it
        *(twi.ms_buf + twi.nb_data) = nnk_twi_read();
        twi.nb_data++;

        // reset len to prevent data resending
        twi.ms_buf_len = 0;

        // no more data to read (reception finished)
        nnk_twi_call_back_call(NNK_TWI_MS_RX_END);

        return;

        // -----------------------------------------------------------------------
        // Slave transmitter
_TW_ST_SLA_ACK:			// 0xa8 : own slave addr + R
_TW_ST_ARB_LOST_SLA_ACK:	// 0xb0 : arb lost as master, own slave addr + R
        // adressed as slave (whether or not the arbitration was lost)

        // reset nb_data to zero
        twi.nb_data = 0;

        // need support from application
        // a buffer to transmit shall be provided
        nnk_twi_call_back_call(NNK_TWI_SL_TX_BEGIN);

        // send data if any
        if ( twi.nb_data < twi.sl_buf_len ) {
                buf = twi.nb_data;
                twi.nb_data++;
                nnk_twi_write( *(twi.sl_buf + buf), OK);
        }
        else {	// send dummy value
                nnk_twi_write(0x33, KO);
        }

        return;

_TW_ST_DATA_ACK:		// 0xb8 : byte transmit, ack received
        // byte sent to the master and it wants more

        // send data if any
        if ( twi.nb_data < twi.sl_buf_len ) {
                buf = twi.nb_data;
                twi.nb_data++;
                nnk_twi_write( *(twi.sl_buf + buf), OK);
        }
        else {	// send dummy value
                nnk_twi_write(0x33, KO);
        }

        return;

_TW_ST_DATA_NACK:		// 0xc0 : byte transmit, nack received
_TW_ST_LAST_DATA:		// 0xc8 : last data transmit, ack received
        // master nack, so tx is finished

        // signal it to the application
        nnk_twi_call_back_call(NNK_TWI_SL_TX_END);

        // if we have data to send or receive as master
        if (twi.ms_buf_len != 0) {
                // generate a restart
                TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
        }
        else {
                // give up bus control, wait for next start
                TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
                twi.state = NNK_TWI_IDLE;
        }

        return;

        // -----------------------------------------------------------------------
        // Slave reveicer
_TW_SR_SLA_ACK:			// 0x60 : own slave addr + W
_TW_SR_ARB_LOST_SLA_ACK:	// 0x68 : arb lost, own slave addr +W
        // addressed as slave for rx (whether or not the arbitration was lost)

        // reset nb_data to zero
        twi.nb_data = 0;

        // need support from application
        // a buffer to received the data shall be provided
        nnk_twi_call_back_call(NNK_TWI_SL_RX_BEGIN);

        // if more data to receive
        if ( (twi.nb_data + 1) < twi.sl_buf_len )
                nnk_twi_ack();
        // else about to receive the last data
        else
                nnk_twi_nack();

        return;

_TW_SR_GCALL_ACK:		// 0x70 : gen call received, ack sent
_TW_SR_ARB_LOST_GCALL_ACK:	// 0x78 : arb lost + idem above
        // general call received (whether or not the arbitration was lost)

        // reset nb_data to zero
        twi.nb_data = 0;

        // need support from application
        // a buffer to received the data shall be provided
        nnk_twi_call_back_call(NNK_TWI_GENCALL_BEGIN);

        // if more data to receive
        if ( (twi.nb_data + 1) < twi.sl_buf_len )
                nnk_twi_ack();
        // else about to receive the last data
        else
                nnk_twi_nack();

        return;

_TW_SR_DATA_ACK:		// 0x80 : data received, ack sent
        // one more byte received and more to come

        // store it
        *(twi.sl_buf + twi.nb_data) = nnk_twi_read();
        twi.nb_data++;

        // if more data to receive
        if ( (twi.nb_data + 1) < twi.sl_buf_len )
                nnk_twi_ack();
        // else about to receive the last data
        else
                nnk_twi_nack();

        return;

_TW_SR_DATA_NACK:		// 0x88 : data received, nack sent
        // last byte received

        // store it
        *(twi.sl_buf + twi.nb_data) = nnk_twi_read();
        twi.nb_data++;

        // signal end of rx
        nnk_twi_call_back_call(NNK_TWI_SL_RX_END);

        return;

_TW_SR_GCALL_DATA_ACK:		// 0x90 : in gen call, data received, ack sent
        // one more byte received and more to come

        // store it
        *(twi.sl_buf + twi.nb_data) = nnk_twi_read();
        twi.nb_data++;

        // if more data to receive
        if ( (twi.nb_data + 1) < twi.sl_buf_len )
                nnk_twi_ack();
        // else about to receive the last data
        else
                nnk_twi_nack();

        return;

_TW_SR_GCALL_DATA_NACK:		// 0x98 : in gen call, data received, nack sent
        // last byte received

        // store it
        *(twi.sl_buf + twi.nb_data) = nnk_twi_read();
        twi.nb_data++;

        // signal end of rx
        nnk_twi_call_back_call(NNK_TWI_GENCALL_END);

        return;

_TW_SR_STOP:			// 0xa0 : stop or restart received
        // the previous transfer as slace receiver (general call or not)
        // is completely finished
        if (twi.state == NNK_TWI_SL_RX_BEGIN)
                nnk_twi_call_back_call(NNK_TWI_SL_RX_END);
        else
                nnk_twi_call_back_call(NNK_TWI_GENCALL_END);

        // but if we have data to send or read as master
        // try to generate a restart
        if (twi.ms_buf_len != 0)
                // generate a restart
                TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);

        return;

        // -----------------------------------------------------------------------
        // Error cases
_TW_NO_INFO:			// 0xf8 : no relevant information
        // back to idle state: re-enable interface
        TWCR = _BV(TWEA) | _BV(TWINT) | _BV(TWEN)  | _BV(TWIE);

        // signal idle mode
        nnk_twi_call_back_call(NNK_TWI_IDLE);

        return;

_TW_BUS_ERROR:			// 0x00 : internal protocol violation
        // hardware error (may I mess the registers!?!? :-) )

        // recover releasing the bus
        //TWCR = _BV(TWEA) | _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
        nnk_twi_call_back_call(NNK_TWI_ERROR);

        return;

other:				// unknown status
        // so, what happens?????

        nnk_twi_call_back_call(NNK_TWI_ERROR);

        return;
}


//-------------------
// public functions
//

void nnk_twi_init(void(*call_back)(enum nnk_twi_state state, u8 nb_data, void* misc), void* misc)
{
        // switch off TWI interface
        TWCR = 0x00;

        // disable internal pull-ups of SDA and SCL (PC4 and PC5)
        MCUCR |= _BV(PUD);
        DDRC &= ~( _BV(DDC4) | _BV(DDC5) );
        PORTC &= ~( _BV(PORTC4) | _BV(PORTC5) );

        // bit rate 100k
        // prescaler = 1
        TWSR = NNK_TWI_PRESCALER_1;

        // baud rate = 32
        TWBR = 32;

        // reset engine state to IDLE
        twi.state = NNK_TWI_IDLE;

        // set call_back pointer
        if ( call_back != NULL )
                twi.call_back = call_back;
        else
                twi.call_back = nnk_twi_default_call_back;

        // set the misc pointer
        twi.misc = misc;

        // enable automatic ACK bit and TWI interface in interrupt mode
        // resetting any previous interrupt
        TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWINT);

        // disable address recognition
        TWAR = 0;
}


void nnk_twi_sl_addr_set(u8 sl_addr)
{
        TWAR &= _BV(TWGCE);		// reset the TWAR except the general call recognition bit
        TWAR |= sl_addr << 1;		// set the slave address we shall respond to
}


u8 nnk_twi_sl_addr_get(void)
{
        return TWAR >> 1;		// get the slave address we're responding to
}


void nnk_twi_gen_call(u8 gen_call)
{
        // if enabled
        if (gen_call) {
                TWAR |= _BV(TWGCE);	// set the general call recognition bit
        }
        else {	// disabled
                TWAR &= ~_BV(TWGCE);	// reset the general call recognition bit
        }
}


void nnk_twi_stop(void)
{
        if (twi.state == NNK_TWI_SL_RX_END || twi.state == NNK_TWI_GENCALL_END) {
                TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
        }

        // if hard is not already IDLE
        if (TW_STATUS != TW_NO_INFO) {
                // try to generate a STOP
                TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWSTO) | _BV(TWIE);

                // in master mode, when the STOP is executed on the bus,
                // the TWSTO bit is cleared automatically.
        }

        // reset engine state to IDLE
        twi.state = NNK_TWI_IDLE;
}


u8 nnk_twi_ms_tx(u8 adr, u8 len, u8* data)
{
        // check if TWI is idle
        if ( (twi.state != NNK_TWI_IDLE) && (twi.state != NNK_TWI_ERROR) )
                return KO;

        // check if bus is stopped
        if (TWCR & _BV(TWSTO))
                return KO;

        // reset TWI state
        twi.state = NNK_TWI_MS_TX_BEGIN;

        // set ms_buf
        twi.ms_buf = data;
        twi.ms_buf_len = len;

        // set addr with write bit
        twi.addr = (adr << 1) | TW_WRITE;

        // begin transmission
        nnk_twi_start();

        return OK;
}


u8 nnk_twi_ms_rx(u8 adr, u8 len, u8* data)
{
        // check if TWI is idle
        if ( (twi.state != NNK_TWI_IDLE) && (twi.state != NNK_TWI_ERROR) )
                return KO;

        // check if bus is stopped
        if (TWCR & _BV(TWSTO))
                return KO;

        // reset TWI state
        twi.state = NNK_TWI_MS_RX_BEGIN;

        // set rx_buf
        twi.ms_buf = data;
        twi.ms_buf_len = len;

        // set addr with read bit
        twi.addr = (adr << 1) | TW_READ;

        // begin transmission
        nnk_twi_start();

        return OK;
}


u8 nnk_twi_sl_tx(u8 len, u8* data)
{
        // check if TWI is slave transmitting
        if (twi.state != NNK_TWI_SL_TX_BEGIN)
                return KO;

        // set sl_buf
        twi.sl_buf = data;
        twi.sl_buf_len = len;

        // transmission will be continued by the master
        // on its will

        return OK;
}


u8 nnk_twi_sl_rx(u8 len, u8* data)
{
        // check if TWI is slave receiving
        // general call is just a special case of slave receiving
        if ( (twi.state != NNK_TWI_SL_RX_BEGIN) && (twi.state != NNK_TWI_GENCALL_BEGIN) )
                return KO;

        // set sl_buf
        twi.sl_buf = data;
        twi.sl_buf_len = len;

        // reception will be continued by the master
        // on its will

        return OK;
}

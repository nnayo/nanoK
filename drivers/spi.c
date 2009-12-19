#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>	// ISR()


//-------------------------------------------
// private functions
//

static struct {
	spi_behaviour_t behaviour;

	// reception handling
	u8* rx_buf;
	u8 rx_len;

	// transmission handling
	u8* tx_buf;
	u8 tx_len;

	// current transfert handling
	u8 index;
	spi_state_t state;

	// call-back
	void (*call_back)(spi_state_t st, void* misc);
	void* misc;
	volatile u8 fini;	// flag to be used in blocking mode

	// stats
	u16 false_triggers;
	u16 overflows;
} SPI;


//-------------------------------------------
// private functions
//

ISR(SPI_STC_vect)
{
	// handling depends on set behaviour 
	switch ( SPI.behaviour ) {
		case SPI_MASTER:
			// if the interrupt is not awaited
			if ( SPI.state != SPI_RUNNING ) {
				// update stats
				SPI.false_triggers++;

				// signal error
				SPI.call_back(SPI_ERROR, SPI.misc);
				SPI.state = SPI_ERROR;

				return;
			}

			// if there are still data to send
			if ( SPI.index < SPI.tx_len ) {
				// update tx register
				SPDR = SPI.tx_buf[SPI.index + 1];
			}
			else {
				// no more data to send but apparently still some to receive
				// so send useless byte
				SPDR = 0xff;
			}

			// if there are still data to receive
			if ( SPI.index < SPI.rx_len ) {
				// retrieve it from rx register
				SPI.rx_buf[SPI.index] = SPDR;
			}

			// update index
			SPI.index++;

			// when emitting buffer is empty and receiving buffer is full 
			if ( (SPI.index >= SPI.tx_len) && (SPI.index >= SPI.rx_len) ) {
				// reset internals
				SPI.state = SPI_IDLE;
				SPI.index = 0;

				// and signal end of transmission
				SPI.call_back(SPI_MASTER_END, SPI.misc);
				PORTB |= _BV(PB4);	// set /SS high to block slave
				SPI.state = SPI_IDLE;
			}
			break;

		case SPI_SLAVE:
			// if the interrupt is not awaited
			if ( (SPI.state != SPI_IDLE) || (SPI.state != SPI_RUNNING) ) {
				// update stats
				SPI.false_triggers++;

				// signal error
				SPI.call_back(SPI_ERROR, SPI.misc);
				SPI.state = SPI_ERROR;

				return;
			}

			// on start of slave reception
			if ( SPI.index == 0 ) {
				// signal it to have the data buffer set
				SPI.call_back(SPI_SLAVE_BEGIN, SPI.misc);

				// start reception process
				SPI.state = SPI_RUNNING;
			}

			// if there is still empty place for the data to receive
			if (  SPI.index < SPI.rx_len ) {
				// retrieve it from rx register
				SPI.rx_buf[SPI.index] = SPDR;

				// provide stub data in emssion
				SPDR = 0xff;
			}

			// update index
			SPI.index++;

			// if received buffer is full
			if ( SPI.index >= SPI.rx_len ) {
				// signal end of transmission
				SPI.call_back(SPI_SLAVE_END, SPI.misc);
				SPI.state = SPI_IDLE;
			}
			break;

		case SPI_RESET:
			SPI.false_triggers++;
			SPI.call_back(SPI_ERROR, SPI.misc);
			SPI.state = SPI_IDLE;
			break;
	}
}


static void SPI_default_call_back(spi_state_t st, void* misc)
{
	(void)st;
	(void)misc;

	SPI.fini = OK;
}


//-------------------------------------------
// public functions
//

void SPI_init(spi_behaviour_t behaviour, spi_mode_t mode, spi_data_order_t data_order, spi_clock_div_t clock_div)
{
	// default mode
	SPI.behaviour = SPI_RESET;

	switch ( behaviour ) {
		case SPI_MASTER:
			SPCR = _BV(MSTR);
			DDRB |= _BV(PB7);	// SCK as output
			DDRB |= _BV(PB5);	// MOSI as output
			DDRB |= _BV(PB4);	// /SS as output
			PORTB |= _BV(PB4);	// /SS is high to block slave
			break;

		case SPI_SLAVE:
			SPCR = 0;
			DDRB &= ~_BV(PB4);	// /SS as input
			DDRB &= ~_BV(PB6);	// MISO as input
			break;

		default:
			SPCR = 0;
			DDRB &= ~_BV(PB4);	// /SS as input
			return;
			break;
	}

	switch ( mode ) {
		case SPI_ZERO:	// CPOL = 0 and CPHA = 0
			break;

		case SPI_ONE:	// CPOL = 0 and CPHA = 1
			SPCR |= _BV(CPHA);
			break;

		case SPI_TWO:	// CPOL = 1 and CPHA = 0
			SPCR |= _BV(CPOL);
			break;

		case SPI_THREE:	// CPOL = 1 and CPHA = 1
			SPCR |= _BV(CPOL);
			SPCR |= _BV(CPHA);
			break;

		default:
			return;
			break;
	}

	switch ( data_order ) {
		case SPI_LSB:
			SPCR |= _BV(DORD);
			break;

		case SPI_MSB:
			break;

		default:
			return;
			break;
	}

	switch ( clock_div ) {
		case SPI_DIV_2:
			SPSR |= _BV(SPI2X);
			break;

		case SPI_DIV_4:
			break;

		case SPI_DIV_8:
			SPSR |= _BV(SPI2X);
			SPCR |= _BV(SPR0);
			break;

		case SPI_DIV_16:
			SPCR |= _BV(SPR0);
			break;

		case SPI_DIV_32:
			SPSR |= _BV(SPI2X);
			SPCR |= _BV(SPR1);
			break;

		case SPI_DIV_64:
			SPCR |= _BV(SPR1);
			break;

		case SPI_DIV_128:
			SPCR |= _BV(SPR0);
			SPCR |= _BV(SPR1);
			break;

		default:
			return;
			break;
	}

	// configuration passed : set the correct behaviour
	SPI.behaviour = behaviour;

	// reset internals
	SPI.state = SPI_IDLE;
	SPI.index = 0;
	SPI.call_back = SPI_default_call_back;
	SPI.misc = NULL;
	SPI.false_triggers = 0;
	SPI.overflows = 0;

	// finally enable the SPI and its interrupt
	SPCR |= _BV(SPE);
	SPCR |= _BV(SPIE);
}


// call-back init
void SPI_call_back_set(void (*call_back)(spi_state_t st, void* misc), void* misc)
{
	SPI.call_back = call_back;
	SPI.misc = misc;
}


// handle a transmission as master
u8 SPI_master(u8* tx_buf, u8 tx_len, u8* rx_buf, u8 rx_len)
{
	// check initial conditions
	if ( (SPI.state != SPI_IDLE) || (SPI.behaviour != SPI_MASTER) ) {
		return KO;
	}

	// save contexts
	SPI.tx_buf = tx_buf;
	SPI.tx_len = tx_len;
	SPI.rx_buf = rx_buf;
	SPI.rx_len = rx_len;

	// start transmission
	SPI.state = SPI_RUNNING;
	PORTB &= ~_BV(PB4);	// set /SS low to select slave
	SPDR = SPI.tx_buf[0];

	// now the ISR will do the rest of the job
	return OK;
}


u8 SPI_master_blocking(u8* tx_buf, u8 tx_len, u8* rx_buf, u8 rx_len)
{
	// save set call-back context
	void (*call_back)(spi_state_t st, void* misc);
	call_back = SPI.call_back;

	// force SPI default call_back
	SPI.call_back = SPI_default_call_back;

	// set the flag to detect end of transmission
	SPI.fini = KO;

	// start transmission
	if (OK != SPI_master(tx_buf, tx_len, rx_buf, rx_len) ) {
		return KO;
	}

	// wait until transmission is over
	while (KO == SPI.fini)
		;

	// restore call-back context
	SPI.call_back = call_back;

	return OK;
}


// handle a transmission as slave
u8 SPI_slave(u8* rx_buf, u8 rx_len)
{
	// check initial conditions
	if ( (SPI.state != SPI_IDLE) || (SPI.behaviour != SPI_SLAVE) ) {
		return KO;
	}

	// save contexts
	SPI.tx_buf = NULL;
	SPI.tx_len = 0;
	SPI.rx_buf = rx_buf;
	SPI.rx_len = rx_len;

	// start transmission
	SPI.state = SPI_RUNNING;

	// now the ISR will do the rest of the job
	return OK;
}

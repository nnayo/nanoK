#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>	// ISR()

//
// in master mode, the SS pin is driven by the code
// for projects, égère and TRoy 2, the pin PB0 is used
// but in slave mode, it is mandatory to use PB2
//

//-------------------------------------------
// private functions
//

static struct {
	spi_behaviour_t behaviour:2;

	volatile u8 fini:1;	// flag to be used in blocking mode
	u8 error:1;			// transfert on error

	// current transfert handling
	spi_state_t state:4;
	u8 index;

	// reception handling
	u8* rx_buf;
	u8 rx_len;

	// transmission handling
	u8* tx_buf;
	u8 tx_len;

	// call-back
	void (*call_back)(spi_state_t st, void* misc);
	void* misc;

#ifdef STATS
	// stats
	u16 false_triggers;
	u16 overflows;
#endif
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
#ifdef STATS
			// update stats
			SPI.false_triggers++;
#endif

			// signal error
			SPI.call_back(SPI_ERROR, SPI.misc);
			SPI.state = SPI_IDLE;

			return;
		}

		// if there are still data to receive
		if ( SPI.index < SPI.rx_len ) {
			// retrieve it from rx register
			SPI.rx_buf[SPI.index] = SPDR;
		}
        else {
            // just consume the received data
            (void)SPDR;
        }

		// update index
		SPI.index++;

		// when emitting buffer is empty and receiving buffer is full 
		if ( (SPI.index >= SPI.tx_len) && (SPI.index >= SPI.rx_len) ) {
			// signal end of transmission
			SPI.call_back(SPI_MASTER_END, SPI.misc);
			PORTB |= _BV(PB0);	// set /SS high to block slave

			// reset internals
			SPI.state = SPI_IDLE;
			SPI.index = 0;
			break;
		}

		// if there are still data to send
		if ( SPI.index < SPI.tx_len ) {
			// update tx register
			SPDR = SPI.tx_buf[SPI.index];
		}
		else {
			// no more data to send but apparently still some to receive
			// so send useless byte
			SPDR = 0xff;
		}
		break;

	case SPI_SLAVE:
		// if the interrupt is not awaited
		if ( (SPI.state != SPI_IDLE) || (SPI.state != SPI_RUNNING) ) {
#ifdef STATS
			// update stats
			SPI.false_triggers++;
#endif

			// signal error
			SPI.call_back(SPI_ERROR, SPI.misc);
			SPI.state = SPI_IDLE;

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

			// provide stub data in emission
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
#ifdef STATS
		SPI.false_triggers++;
#endif
		SPI.call_back(SPI_ERROR, SPI.misc);
		SPI.state = SPI_IDLE;
		break;
	}
}


// default call-back only useful for blocking mode
static void SPI_default_call_back(spi_state_t st, void* misc)
{
	(void)misc;

	SPI.error = ( st == SPI_ERROR ) ? 1 : 0;
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
		DDRB |= _BV(PB0);	// /SS as output
		PORTB |= _BV(PB0);	// /SS is high to block slave
		break;

	case SPI_SLAVE:
		SPCR = 0;
		DDRB &= ~_BV(PB2);	// /SS as input
		DDRB &= ~_BV(PB6);	// MISO as input
		break;

	default:
		SPCR = 0;
		DDRB &= ~_BV(PB2);	// /SS as input
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

	SPI_set_clock(clock_div);

	// configuration passed : set the correct behaviour
	SPI.behaviour = behaviour;

	// reset internals
	SPI.state = SPI_IDLE;
	SPI.index = 0;
	SPI.call_back = SPI_default_call_back;
	SPI.misc = NULL;
#ifdef STATS
	SPI.false_triggers = 0;
	SPI.overflows = 0;
#endif

	// finally enable the SPI and its interrupt
	SPCR |= _BV(SPE);
	SPCR |= _BV(SPIE);
}

// set the SPI clock speed
void SPI_set_clock(spi_clock_div_t clock_div)
{
    SPSR &= (~_BV(SPI2X));
    SPCR &= (~_BV(SPR0)) & (~_BV(SPR1));
    
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

	// set the flag to detect end of transmission
	SPI.fini = KO;

	// start transmission
	SPI.state = SPI_RUNNING;
	PORTB &= ~_BV(PB0);	// set /SS low to select slave
	SPDR = SPI.tx_buf[0];

	// now the ISR will do the rest of the job
	return OK;
}


// when using the default call-back, call this function to know if the transfert is done
u8 SPI_is_fini(void)
{
	return SPI.fini;
}


// when using the default call-back, call this function to know if the transfert is OK
u8 SPI_is_ok(void)
{
	return SPI.error;
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

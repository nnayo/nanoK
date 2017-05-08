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
	enum nnk_spi_behaviour behaviour:2;

	volatile u8 fini:1;	// flag to be used in blocking mode
	u8 error:1;			// transfert on error

	// current transfert handling
	enum nnk_spi_state state:4;
	u8 index;

	// reception handling
	u8* rx_buf;
	u8 rx_len;

	// transmission handling
	const u8* tx_buf;
	u8 tx_len;

	// call-back
	void (*call_back)(enum nnk_spi_state st, void* misc);
	void* misc;

#ifdef STATS
	// stats
	u16 false_triggers;
	u16 overflows;
#endif
} spi;


//-------------------------------------------
// private functions
//

ISR(SPI_STC_vect)
{
	// handling depends on set behaviour 
	switch ( spi.behaviour ) {
	case NNK_SPI_MASTER:
		// if the interrupt is not awaited
		if ( spi.state != NNK_SPI_RUNNING ) {
#ifdef STATS
			// update stats
			spi.false_triggers++;
#endif

			// signal error
			spi.call_back(NNK_SPI_ERROR, spi.misc);
			spi.state = NNK_SPI_IDLE;

			return;
		}

		// if there are still data to receive
		if ( spi.index < spi.rx_len ) {
			// retrieve it from rx register
			spi.rx_buf[spi.index] = SPDR;
		}
        else {
            // just consume the received data
            (void)SPDR;
        }

		// update index
		spi.index++;

		// when emitting buffer is empty and receiving buffer is full 
		if ( (spi.index >= spi.tx_len) && (spi.index >= spi.rx_len) ) {
			// signal end of transmission
			spi.call_back(NNK_SPI_MASTER_END, spi.misc);
			PORTB |= _BV(PB0);	// set /SS high to block slave

			// reset internals
			spi.state = NNK_SPI_IDLE;
			spi.index = 0;
			break;
		}

		// if there are still data to send
		if ( spi.index < spi.tx_len ) {
			// update tx register
			SPDR = spi.tx_buf[spi.index];
		}
		else {
			// no more data to send but apparently still some to receive
			// so send useless byte
			SPDR = 0xff;
		}
		break;

	case NNK_SPI_SLAVE:
		// if the interrupt is not awaited
		if ( (spi.state != NNK_SPI_IDLE) || (spi.state != NNK_SPI_RUNNING) ) {
#ifdef STATS
			// update stats
			spi.false_triggers++;
#endif

			// signal error
			spi.call_back(NNK_SPI_ERROR, spi.misc);
			spi.state = NNK_SPI_IDLE;

			return;
		}

		// on start of slave reception
		if ( spi.index == 0 ) {
			// signal it to have the data buffer set
			spi.call_back(NNK_SPI_SLAVE_BEGIN, spi.misc);

			// start reception process
			spi.state = NNK_SPI_RUNNING;
		}

		// if there is still empty place for the data to receive
		if (  spi.index < spi.rx_len ) {
			// retrieve it from rx register
			spi.rx_buf[spi.index] = SPDR;

			// provide stub data in emission
			SPDR = 0xff;
		}

		// update index
		spi.index++;

		// if received buffer is full
		if ( spi.index >= spi.rx_len ) {
			// signal end of transmission
			spi.call_back(NNK_SPI_SLAVE_END, spi.misc);
			spi.state = NNK_SPI_IDLE;
		}
		break;

	case NNK_SPI_RESET:
#ifdef STATS
		spi.false_triggers++;
#endif
		spi.call_back(NNK_SPI_ERROR, spi.misc);
		spi.state = NNK_SPI_IDLE;
		break;
	}
}


// default call-back only useful for blocking mode
static void nnk_spi_default_call_back(enum nnk_spi_state st, void* misc)
{
	(void)misc;

	spi.error = ( st == NNK_SPI_ERROR ) ? 1 : 0;
	spi.fini = OK;
}


//-------------------------------------------
// public functions
//

void nnk_spi_init(enum nnk_spi_behaviour behaviour, enum nnk_spi_mode mode, enum nnk_spi_data_order data_order, enum nnk_spi_clock_div clock_div)
{
	// default mode
	spi.behaviour = NNK_SPI_RESET;

	switch ( behaviour ) {
	case NNK_SPI_MASTER:
		SPCR = _BV(MSTR);
		DDRB |= _BV(PB7);	// SCK as output
		DDRB |= _BV(PB5);	// MOSI as output
		DDRB |= _BV(PB0);	// /SS as output
		PORTB |= _BV(PB0);	// /SS is high to block slave
		break;

	case NNK_SPI_SLAVE:
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
	case NNK_SPI_ZERO:	// CPOL = 0 and CPHA = 0
		break;

	case NNK_SPI_ONE:	// CPOL = 0 and CPHA = 1
		SPCR |= _BV(CPHA);
		break;

	case NNK_SPI_TWO:	// CPOL = 1 and CPHA = 0
		SPCR |= _BV(CPOL);
		break;

	case NNK_SPI_THREE:	// CPOL = 1 and CPHA = 1
		SPCR |= _BV(CPOL);
		SPCR |= _BV(CPHA);
		break;

	default:
		return;
		break;
	}

	switch ( data_order ) {
	case NNK_SPI_LSB:
		SPCR |= _BV(DORD);
		break;

	case NNK_SPI_MSB:
		break;

	default:
		return;
		break;
	}

	nnk_spi_set_clock(clock_div);

	// configuration passed : set the correct behaviour
	spi.behaviour = behaviour;

	// reset internals
	spi.state = NNK_SPI_IDLE;
	spi.index = 0;
	spi.call_back = nnk_spi_default_call_back;
	spi.misc = NULL;
#ifdef STATS
	spi.false_triggers = 0;
	spi.overflows = 0;
#endif

	// finally enable the SPI and its interrupt
	SPCR |= _BV(SPE);
	SPCR |= _BV(SPIE);
}

// set the SPI clock speed
void nnk_spi_set_clock(enum nnk_spi_clock_div clock_div)
{
    SPSR &= (~_BV(SPI2X));
    SPCR &= (~_BV(SPR0)) & (~_BV(SPR1));
    
    switch ( clock_div ) {
    case NNK_SPI_DIV_2:
        SPSR |= _BV(SPI2X);
        break;
        
    case NNK_SPI_DIV_4:
        break;
        
    case NNK_SPI_DIV_8:
        SPSR |= _BV(SPI2X);
        SPCR |= _BV(SPR0);
        break;
        
    case NNK_SPI_DIV_16:
        SPCR |= _BV(SPR0);
        break;
        
    case NNK_SPI_DIV_32:
        SPSR |= _BV(SPI2X);
        SPCR |= _BV(SPR1);
        break;
        
    case NNK_SPI_DIV_64:
        SPCR |= _BV(SPR1);
        break;
        
    case NNK_SPI_DIV_128:
        SPCR |= _BV(SPR0);
        SPCR |= _BV(SPR1);
        break;
        
    default:
        return;
        break;
    }
}

// call-back init
void nnk_spi_call_back_set(void (*call_back)(enum nnk_spi_state st, void* misc), void* misc)
{
	spi.call_back = call_back;
	spi.misc = misc;
}


// handle a transmission as master
u8 nnk_spi_master(const u8* const tx_buf, u8 tx_len, u8* const rx_buf, u8 rx_len)
{
	// check initial conditions
	if ( (spi.state != NNK_SPI_IDLE) || (spi.behaviour != NNK_SPI_MASTER) ) {
		return KO;
	}

	// save contexts
	spi.tx_buf = tx_buf;
	spi.tx_len = tx_len;
	spi.rx_buf = rx_buf;
	spi.rx_len = rx_len;

	// set the flag to detect end of transmission
	spi.fini = KO;

	// start transmission
	spi.state = NNK_SPI_RUNNING;
	PORTB &= ~_BV(PB0);	// set /SS low to select slave
	SPDR = spi.tx_buf[0];

	// now the ISR will do the rest of the job
	return OK;
}


// when using the default call-back, call this function to know if the transfert is done
u8 nnk_spi_is_fini(void)
{
	return spi.fini;
}


// when using the default call-back, call this function to know if the transfert is OK
u8 nnk_spi_is_ok(void)
{
	return spi.error;
}


// handle a transmission as slave
u8 nnk_spi_slave(u8* rx_buf, u8 rx_len)
{
	// check initial conditions
	if ( (spi.state != NNK_SPI_IDLE) || (spi.behaviour != NNK_SPI_SLAVE) ) {
		return KO;
	}

	// save contexts
	spi.tx_buf = NULL;
	spi.tx_len = 0;
	spi.rx_buf = rx_buf;
	spi.rx_len = rx_len;

	// start transmission
	spi.state = NNK_SPI_RUNNING;

	// now the ISR will do the rest of the job
	return OK;
}

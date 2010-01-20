#ifndef __SPI_H__
#define __SPI_H__

# include "type_def.h"


//-------------------------------------------
// types
//

// driver behaviour
typedef enum {
	SPI_RESET,
	SPI_MASTER,
	SPI_SLAVE,
} spi_behaviour_t;

// polarity and phase modes
typedef enum {
	SPI_ZERO,	// 0
	SPI_ONE,	// 1
	SPI_TWO,	// 2
	SPI_THREE,	// 3
} spi_mode_t;

// data order
typedef enum {
	SPI_LSB,	// LSB first
	SPI_MSB,	// MSB first
} spi_data_order_t;

// clock divisor
typedef enum {
	SPI_DIV_2,
	SPI_DIV_4,
	SPI_DIV_8,
	SPI_DIV_16,
	SPI_DIV_32,
	SPI_DIV_64,
	SPI_DIV_128,
} spi_clock_div_t;

// data exchange state
typedef enum {
	SPI_IDLE,
	SPI_RUNNING,
	SPI_MASTER_END,
	SPI_SLAVE_BEGIN,
	SPI_SLAVE_END,
	SPI_ERROR
} spi_state_t;


//-------------------------------------------
// public functions
//

// set the behaviour and the mode of the SPI block
extern void SPI_init(spi_behaviour_t behaviour, spi_mode_t mode, spi_data_order_t data_order, spi_clock_div_t clock_div);

// set the SPI clock speed
extern void SPI_set_clock(spi_clock_div_t clock_div);

// set a call-back function and an optional parameter
extern void SPI_call_back_set(void (*call_back)(spi_state_t st, void* misc), void* misc);

// transmit and receive data as a master
extern u8 SPI_master(u8* tx_buf, u8 tx_len, u8* rx_buf, u8 rx_len);

// transmit and receive data as a master in blocking mode
extern u8 SPI_master_blocking(u8* tx_buf, u8 tx_len, u8* rx_buf, u8 rx_len);

// receive data as a slave
extern u8 SPI_slave(u8* rx_buf, u8 rx_len);


#endif	// __SPI_H__

#ifndef __NNK_SPI_H__
#define __NNK_SPI_H__

# include "type_def.h"


//-------------------------------------------
// types
//

// driver behaviour
enum nnk_spi_behaviour {
	NNK_SPI_RESET,
	NNK_SPI_MASTER,
	NNK_SPI_SLAVE,
};

// polarity and phase modes
enum nnk_spi_mode {
	NNK_SPI_ZERO,	// 0
	NNK_SPI_ONE,	// 1
	NNK_SPI_TWO,	// 2
	NNK_SPI_THREE,	// 3
};

// data order
enum nnk_spi_data_order {
	NNK_SPI_LSB,	// LSB first
	NNK_SPI_MSB,	// MSB first
};

// clock divisor
enum nnk_spi_clock_div {
	NNK_SPI_DIV_2,
	NNK_SPI_DIV_4,
	NNK_SPI_DIV_8,
	NNK_SPI_DIV_16,
	NNK_SPI_DIV_32,
	NNK_SPI_DIV_64,
	NNK_SPI_DIV_128,
};

// data exchange state
enum nnk_spi_state {
	NNK_SPI_IDLE,
	NNK_SPI_RUNNING,
	NNK_SPI_MASTER_END,
	NNK_SPI_SLAVE_BEGIN,
	NNK_SPI_SLAVE_END,
	NNK_SPI_ERROR
};


//-------------------------------------------
// public functions
//

// set the behaviour and the mode of the SPI block
extern void nnk_spi_init(enum nnk_spi_behaviour behaviour, enum nnk_spi_mode mode, enum nnk_spi_data_order data_order, enum nnk_spi_clock_div clock_div);

// set the SPI clock speed
extern void nnk_spi_set_clock(enum nnk_spi_clock_div clock_div);

// set a call-back function and an optional parameter
extern void nnk_spi_call_back_set(void (*call_back)(enum nnk_spi_state st, void* misc), void* misc);

// transmit and receive data as a master
// the buffers shall be remained allocated till the end of the transfer
extern u8 nnk_spi_master(const u8* const tx_buf, u8 tx_len, u8* const rx_buf, u8 rx_len);

// when using the default call-back, call this function to know if the transfert is done
extern u8 nnk_spi_is_fini(void);

// when using the default call-back, call this function to know if the transfert is OK
extern u8 nnk_spi_is_ok(void);

// receive data as a slave
extern u8 nnk_spi_slave(u8* const rx_buf, u8 rx_len);

#endif	// __NNK_SPI_H__

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


// ADXL driver is right now only intend for ADXL345 chip from Analog Devices
//


#include "externals/adxl345_internals.h"

#include "externals/adxl345.h"

#include "drivers/spi.h"

#include <string.h>		// memcpy()


//-----------------------------------------------------
// private variables
//


//-----------------------------------------------------
// private functions
//


//-----------------------------------------------------
// public functions
//

// initialization of the ADXL345 component
u8 ADXL_init(void)
{
	u8 tx_buf[2];
	u8 rx_buf[2];

	// read the device ID
	tx_buf[0] = ADXL_READ | DEVID;
	SPI_master_blocking(tx_buf, 1, rx_buf, 2);

	// check if device is the ADXL345
	if ( rx_buf[1] != DEVID_VALUE ) {
		return KO;
	}

	// set the measure bit in power control to start acquisitions
	tx_buf[0] = ADXL_WRITE | POWER_CTL;
	tx_buf[1] = (1 << 3);
	SPI_master_blocking(tx_buf, 2, rx_buf, 0);

	return KO;
}


u8 ADXL_range_set(acc_range_t rg)
{
	u8 tx_buf[2];
	u8 rx_buf[2];

	// set the acceleration range and format the data to be right justified
	tx_buf[0] = ADXL_WRITE | DATA_FORMAT;
	tx_buf[1] = (1 << 3) | rg;
	SPI_master_blocking(tx_buf, 2, rx_buf, 0);

	return OK;
}


// retrieve X, Y and Z accelerations values
u8 ADXL_get(u16* acc_x, u16* acc_y, u16* acc_z)
{
	u8 tx_buf[1];
	u8 rx_buf[7];

	// read the 6 acceleration values in a row
	tx_buf[0] = ADXL_READ | ADXL_MB | DATAX0;
	SPI_master_blocking(tx_buf, 1, rx_buf, 7);

	// format the values
	*acc_x = (rx_buf[1] << 8) + rx_buf[2];
	*acc_y = (rx_buf[3] << 8) + rx_buf[4];
	*acc_z = (rx_buf[5] << 8) + rx_buf[6];

	return OK;
}

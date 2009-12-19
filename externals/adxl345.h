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


// ADXL driver is right now only intend for ADXL345 chip from Analog Device
//


#ifndef __ADXL_H__
# define __ADXL_H__

# include "type_def.h"

typedef enum {
	ADXL_2G,
	ADXL_4G,
	ADXL_8G,
	ADXL_16G,
} acc_range_t;


// initialization of the ADXL345 component
extern u8 ADXL_init(void);	

// set the acceleration range
extern u8 ADXL_range_set(acc_range_t rg);

// retrieve X, Y and Z accelerations values
extern u8 ADXL_get(u16* acc_x, u16* acc_y, u16* acc_z);

#endif	// __ADXL_H__

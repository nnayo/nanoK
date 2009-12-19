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

// ADXL345 chip register description
//
// this chip is from Analog Devices
//

#ifndef __ADXL345_H__
# define __ADXL345_H__

# include "type_def.h"

//------------------------------------------
// public defines
//

# define ADXL_WRITE		(0 << 7)
# define ADXL_READ		(1 << 7)
# define ADXL_MB		(1 << 6)

// registers
//

// device ID (reset : 0xe5 = 0b11100101)
# define	DEVID			((u8)0x00)
# define	DEVID_VALUE		((u8)0xe5)

// reserved
// 0x01 - 0x1c

// tap threshold
# define	THRESH_TAP		((u8)0x1d)

// X-axis offset
# define	OFSX			((u8)0x1e)

// Y-axis offset
# define	OFSY			((u8)0x1f)

// Z-axis offset
# define	OFSZ			((u8)0x20)

// tap duration
# define	DUR				((u8)0x21)

// tap latency
# define	LATENT			((u8)0x22)

// tap window
# define	WINDOW			((u8)0x23)

// activity threshold
# define	THRESH_ACT		((u8)0x24)

// inactivity threshold
# define	THRESH_INACT	((u8)0x25)

// inactivity time
# define	TIME_INACT		((u8)0x26)

// axis enable control for activity and inactivity detection
# define	ACT_INACT_CTL	((u8)0x27)

// free-fall threshold
# define	THRESH_FF		((u8)0x28)

// free-fall time
# define	TIME_FF			((u8)0x29)

// axis control for tap/double tap
# define	TAP_AXES		((u8)0x2a)

// source of tap/double tap
# define	ACT_TAP_STATUS	((u8)0x2b)

// data rate and power mode control
# define	BW_RATE			((u8)0x2c)

// power-saving features control
# define	POWER_CTL		((u8)0x2d)

// interrupt enable control
# define	INT_ENABLE		((u8)0x2e)

// interrupt mapping control
# define	INT_MAP			((u8)0x2f)

// source of interrupts
# define	INT_SOURCE		((u8)0x30)

// data format control
# define	DATA_FORMAT		((u8)0x31)

// X-axis data 0
# define	DATAX0			((u8)0x32)

// X-axis data 1
# define	DATAX1			((u8)0x33)

// Y-axis data 0
# define	DATAY0			((u8)0x34)

// Y-axis data 1
# define	DATAY1			((u8)0x35)

// Z-axis data 0
# define	DATAZ0			((u8)0x36)

// Z-axis data 1
# define	DATAZ1			((u8)0x37)

// FIFO control
# define	FIFO_CTL		((u8)0x38)

// FIFO status
# define	FIFO_STATUS		((u8)0x39)



//------------------------------------------
// public types
//


#endif	// __ADXL345_H__

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

// the driver is intended for SDCARD
//


# include "externals/sdcard.h"


// initialization of the SDCARD component
u8 SD_init(void)
{
	return OK;
}

// read len byte(s) from SDCARD address addr and copy them in data
u8 SD_read(u16 addr, u8* data, u8 len)
{
	return KO;
}

// write len byte(s) to SDCARD address addr from data
u8 SD_write(u16 addr, u8* data, u8 len)
{
	return KO;
}

// check if SDCARD write has ended
u8 SD_is_fini(void)
{
	return OK;
}

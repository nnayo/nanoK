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

#ifndef __EEPROM_H__
# define __EEPROM_H__

# include "type_def.h"


// EEPROM driver initialization
extern void EEP_init(void);

// read len byte(s) from EEPROM address addr and copy them in data
extern u8 EEP_read(u16 addr, u8* data, u8 len);

// write len byte(s) to EEPROM address addr from data
extern u8 EEP_write(u16 addr, u8* data, u8 len);

// check if eeprom write has ended
extern u8 EEP_is_fini(void);

#endif	// __EEPROM_H__

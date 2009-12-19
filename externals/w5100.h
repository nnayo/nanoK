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

// the driver is intended for W5100 chip from Wiznet
//


#ifndef __W5100_H__
# define __W5100_H__

# include "type_def.h"


// initialization of the W5100 component
extern u8 W5100_init(void);

// send a data block to destination IP and port 
extern u8 W5100_tx(u32 ip, u16 port, u8* data, u8 len);

// read a data block received on port
extern u8 W5100_rx(u16 port, const u8* data, u8 len);


#endif	// __W5100_H__

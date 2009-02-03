//---------------------
//  Copyright (C) 2008  <Yann GOUY>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
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

#ifndef __SLP_H__
# define __SLP_H__

# include "type_def.h"

typedef u16 slp_t;			// sleep mask

extern void SLP_init(void);		// setup for SLEEP system

extern slp_t SLP_register(void);	// return the sleep mask for the registering client

extern u8 SLP_request(slp_t mask);	// a registered client can request to sleep
					// the function returns OK if sleep has happened, KO else

#endif

//---------------------
//  Copyright (C) 2000-2008  <Yann GOUY>
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

// TIME
//
// this is intended to hold an internal time
// the time resolution is 10 micro-second
//
// as the time is stored in a 32-bytes variable
// up to 42949.6 seconds can be counted
// that is 715.8 minutes or 11.9 hours
//


#ifndef __TIME_H__
# define __TIME_H__

# include "type_def.h"


# define TIME_1_MSEC	( (u32)10 )						// one milli-second
# define TIME_1_SEC		( (u32)(1000 * TIME_1_MSEC) )	// one second
# define TIME_MAX		( (u32)0xffffffff )				// max time value (11.93 hours)

// init the internals of TIME
// and may provide a function to have a better
// precision when calling TIME_get()
extern void TIME_init(u32(*adjust)(void));

// set the TIME increment.
// the increment value is depending
// on the calling period of TIME_incr()
// incr shall be given in tenth of milli-second
extern void TIME_set_incr(u32 incr);

// get the TIME increment in tenth of milli-second
extern u32 TIME_get_incr(void);

// increment the TIME
// the call to this function is on the responsability of the user
extern void TIME_incr(void);

// get current value of TIME in tenth of milli-second
extern u32 TIME_get(void);

// get a more accurate value of TIME in tenth of milli-second
extern u32 TIME_get_precise(void);


#endif	// __TIME_H__

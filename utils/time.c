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
// see description in time.h
//


#include "time.h"


// ---------------------------------------
// private variables
//

static struct {
	u32 time;		// current time
	u32 incr;		// increment step
	u32 (*adjust)(void);	// if provided, more precision for TIME_get()
} TIME;


// ---------------------------------------
// public functions
//

// init the internals of TIME
void TIME_init(u32(*adjust)(void))
{
	TIME.time = 0;
	TIME.incr = 0;
	TIME.adjust = adjust;
}


// set the TIME increment.
void TIME_set_incr(u32 incr)
{
	TIME.incr = incr;
}


// get the TIME increment.
u32 TIME_get_incr(void)
{
	return TIME.incr;
}


// increment the TIME
// the call to this function is on the responsability of the user
// it is preferably done in the nanoK hook.
void TIME_incr(void)
{
	TIME.time += TIME.incr;
}


// get current value of TIME
u32 TIME_get(void)
{
	return TIME.time;
}


// get a more accurate current value of TIME
u32 TIME_get_precise(void)
{
	u32 time;

	time = TIME.time;

	if (TIME.adjust) {
		time += TIME.adjust();
	}

	return time;
}

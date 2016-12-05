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
	u32 (*adjust)(void);	// if provided, more precision for nnk_time_get()
} time;


// ---------------------------------------
// public functions
//

// init the internals of TIME
void nnk_time_init(u32(*adjust)(void))
{
	time.time = 0;
	time.incr = 0;
	time.adjust = adjust;
}


// set the time increment.
void nnk_time_set_incr(u32 incr)
{
	time.incr = incr;
}


// get the time increment.
u32 nnk_time_get_incr(void)
{
	return time.incr;
}


// increment the time
// the call to this function is on the responsability of the user
// it is preferably done in the nanoK hook.
void nnk_time_incr(void)
{
	time.time += time.incr;
}


// get current value of time
u32 nnk_time_get(void)
{
	return time.time;
}


// get a more accurate current value of time
u32 nnk_time_get_precise(void)
{
	u32 t;

	t = time.time;

	if (time.adjust) {
		t += time.adjust();
	}

	return t;
}

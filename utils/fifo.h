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


#ifndef __FIFO_H__
# define __FIFO_H__

# include "type_def.h"


typedef struct {
	int lng;	// elements buffer length
	int nb;		// elements number
	int elem_size;	// element size
	void* donnees;	// pointer on the data buffer
	void* in;	// insertion pointer
	void* out;	// extraction pointer
} fifo_t;


// init the FIFO
// with a pointer to the data buffer
// and the length in elements of the buffer
//
extern void FIFO_init(fifo_t* f, void* buf, u16 nb_elem, u16 elem_size);


// add an element to the given fifo
// return OK if every thing ok else KO
//
extern u8 FIFO_put(fifo_t* f, void* elem);


// get an element from the given fifo
// return OK if every thing ok else KO
// 
extern u8 FIFO_get(fifo_t* f, void* elem);


// put back an element to the given fifo
// return OK if every thing ok else KO
// 
extern u8 FIFO_unget(fifo_t* f, void* elem);


// returns the free place in the given fifo
// in term of elements
//
extern u16 FIFO_free(fifo_t* f);


// returns the place taken in the given fifo
// in term of elements
//
extern u16 FIFO_full(fifo_t* f);

#endif

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

#ifndef __TYPE_DEF_H__
# define __TYPE_DEF_H__

#include <inttypes.h>

// redefinition of types
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

// some pratical values
# define KO	((u8)0)
# define FALSE	(0 == 1)
# define OK	((u8)1)
# define TRUE	(1 == 1)

# ifndef NULL
#  define NULL   ((void*)0)
# endif

# ifndef _BV
#  define _BV(x)   (1 << x)
# endif

#endif	// __TYPE_DEF_H__

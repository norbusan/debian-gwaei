/* -*- mode: C; c-file-style: "bsd"; tab-width: 4 -*- */
/* pilotcompat.h - non-Pilot compatibility defines, particularly for Unix.
 * JStroke 1.x - Japanese Kanji handwriting recognition technology demo.
 * Copyright (C) 1997  Robert E. Wells
 * http://wellscs.com/pilot
 * mailto:robert@wellscs.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (gpl.html); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * pilotcompat.h created by Owen Taylor <owt1@cornell.edu>, 9/1997.
 * Owen is using the jstroke recognition code on Linux in his KanjiPad
 * application, and uses pilotcompat.h to provide compatibility with some
 * Pilot environment definitions.  Owen says: "It's a rough attempt - 
 * it probably would need some small changes for machines where malloc 
 * returns (char *) instead of (void *)." -19970907.
 *
 * -------------------------------------------------------------------------*/

#ifndef __PILOTCOMPAT_H__
#define __PILOTCOMPAT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int Boolean;
typedef unsigned char Byte;
typedef long Long;
typedef unsigned long ULong;
typedef unsigned int UInt;
typedef void * VoidPtr;
typedef char * CharPtr;

#define MemPtrNew malloc
#define MemPtrFree free
#define StrLen strlen
#define StrIToA(str, n) sprintf((str),"%ld",(long)(n))
#define StrIToH(str, n) sprintf((str),"%lx",(long)(n))
#define false 0
#define true 1

#endif /*__PILOTCOMPAT_H__*/
/* ----- End of pilotcompat.h --------------------------------------------- */

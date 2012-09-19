/* -*- mode: C; c-file-style: "bsd"; tab-width: 4 -*- */
/* memowrite.h - definitions for MemoWrite utility trace functions.
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
 * See readme.txt, changelo, and gpl.html for more information.
 *
 * Commentary:
 *
 * This provides a mechanism for generating traces from a program as Pilot
 * Memo entries, where they can be examined, HotSync'ed, and deleted using
 * standard applications.
 * ------------------------------------------------------------------------- */
#ifndef __MEMOWRITE_H__
#define __MEMOWRITE_H__

#ifdef FOR_MEMOWRITE

#define MemoWriteOpen()   Memo_WriteOpen()
#define MemoWriteClose()  Memo_WriteClose()
#define MemoWriteLen(a,b) Memo_WriteLen((a),(b))
#define MemoWrite(a)      Memo_Write(a)
#define MemoWriteln(a)    Memo_Writeln(a)
#define MemoWrite2(a,b)   Memo_Write2((a),(b))
#define MemoWrite2d(a,b)  Memo_Write2d((a),(b))
#define MemoWrite2h(a,b)  Memo_Write2h((a),(b))

void Memo_WriteOpen(void);
void Memo_WriteClose(void);
void Memo_WriteLen(char* cp, UInt len);
void Memo_Write(char* cp);
void Memo_Writeln(char* cp);
void Memo_Write2(char* cp, char* cpVal);
void Memo_Write2d(char* cp, long iVal);
void Memo_Write2h(char* cp, long iVal);

#else  /* FOR_MEMOWRITE not defined */

#define MemoWriteOpen()   
#define MemoWriteClose()  
#define MemoWriteLen(a,b) 
#define MemoWrite(a)      
#define MemoWriteln(a)    
#define MemoWrite2(a,b)   
#define MemoWrite2d(a,b)  
#define MemoWrite2h(a,b)  

#endif /* FOR_MEMOWRITE not defined */

#endif /*__MEMOWRITE_H__*/
/* ----- end of memowrite.h ------------------------------------------------*/

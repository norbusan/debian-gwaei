/* -*- mode: C; c-file-style: "bsd"; tab-width: 4 -*- */
/* jstroke.h - System-independent functions/defines
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
 * Derived from prior work by Todd David Rudick on JavaDict and StrokeDic.
 * Makes use of KANJIDIC data from Jim Breen of Monash University.
 * Further credit details available at http://wellscs.com/pilot
 * See readme.txt, ChangeLog, and gpl.html for more information.
 *
 * CONDITIONAL COMPILATION FLAGS:
 * -DFOR_PILOT_GCC -- Robert Wells uses this for code that is specific to the
 *                US Robotics/3COM Pilot GCC development environment.  
 *                Contact robert@wellscs.com, http://wellscs.com/pilot.
 *
 * -DFOR_PILOT_COMPAT -- Owen Taylor uses this for his perl front-ended 
 *                application.  He puts this code in a subdirectory under the
 *                rest of his code.  Contact owt1@cornell.edu for more 
 *                information, or visit his web site at 
 *                http://www.msc.cornell.edu/~otaylor/. 9/1997.
 *
 * -------------------------------------------------------------------------*/
#ifndef __JSTROKE_H__
#define __JSTROKE_H__

#ifdef FOR_PILOT_COMPAT
#include "pilotcompat.h"
#endif /*FOR_PILOT_COMPAT*/

#ifdef FOR_PILOT_GCC
#pragma pack(2)	// $$$ Probably not needed anymore... -rwells, 19970921.
#include <Common.h>
#include <System/SysAll.h>
#include <UI/UIAll.h>
#endif /*FOR_PILOT_GCC*/

/* Limit list to what we can afford (5), or what will fit on screen at most.
 * #define diMaxListCount     (diScreenHeight/diFontLineHeight)
 */
#define diMaxListCount       5
#define diMaxXyPairs       256	/* Max pairs in stroke... */

/* ----- List Memory ---------------------------------------------------------
 * The idea here is to have a single nonmovable chunk of memory which contains
 * all the structures and cross-pointers needed to support the item list for
 * passing to LstSetListChoices, and related List control functions.  We
 * allocate and free it as a single chunk of application memory.
 */

typedef struct {
	UInt   m_argc;
	char** m_argv;
} ListMem;

/* ----- RawStroke ---------------------------------------------------------*/

typedef struct {
	UInt   m_len;
	Byte   m_x[diMaxXyPairs];
	Byte   m_y[diMaxXyPairs];
} RawStroke;

/* ----- ScoreItem ---------------------------------------------------------*/

typedef struct ScoreItemStruct *ScoreItemPtr;

typedef struct ScoreItemStruct {
	ULong        m_iScore;
    CharPtr      m_cp;
} ScoreItem;

/* ----- StrokeScorer------------------------------------------------------ */

typedef struct StrokeScorer *StrokeScorerPtr;

typedef struct StrokeScorerStruct {
	CharPtr     m_cpStrokeDic;
	RawStroke*  m_pRawStrokes;
	UInt        m_iStrokeCnt;
	ScoreItem*  m_pScores;
	UInt        m_iScoreLen;
	CharPtr     m_cpPath;
} StrokeScorer;

ListMem*  AppEmptyList();
Long      Angle32(Long xdif, Long ydif);
void      ErrBox(CharPtr msg);
void      ErrBox2(CharPtr msg1, CharPtr msg2);

/* Create a StrokeScorer object. (Returns NULL if can't get memory) */
StrokeScorer *StrokeScorerCreate  (CharPtr cpStrokeDic, RawStroke *rsp,
			 					   UInt iStrokeCnt);

/* Destroy a StrokeScorer object */
void          StrokeScorerDestroy  (StrokeScorer *pScorer);

/* Process some database entries (maximum iMaxCnt, -1 for all).
 * Returns 0 when none remaining (should eventually return count remaining
 * to facilitate a progressbar.
 */
Long          StrokeScorerProcess  (StrokeScorer *pScorer, Long iMaxCnt);

/* Return best diMaxListCount candidates processed so far */
ListMem*      StrokeScorerTopPicks (StrokeScorer *pScorer);

#endif /*__JSTROKE_H__*/
/* ----- End of jstroke.h ------------------------------------------------- */

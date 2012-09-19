/* -*- mode: C; c-file-style: "bsd"; tab-width: 4 -*- */
/* util.c - Functions shared between handwriting engine and UI
 * JStroke 1.x - Japanese Kanji handwriting recognition technology demo.
 * Copyright (C) 1997  Robert E. Wells
 * http://wellscs.com/pilot
 * mailto:robert@wellscs.com
 * 
 * gWaei is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gWaei is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
 * -------------------------------------------------------------------------*/

#include "jstroke.h"
#include "jstrokerc.h"

/* ----- AppEmptyList ------------------------------------------------------*/

ListMem*  AppEmptyList() {
	UInt      iTotSize;
	ListMem*  pListMem;
	CharPtr   cpList;
	CharPtr*  cppList;

	iTotSize = sizeof(ListMem) + 2 * sizeof(CharPtr) + 1;

	if (!(cpList = MemPtrNew(iTotSize))) {
		ErrBox("ERROR: no mem in top picks");
		return NULL;
	}

	pListMem = (ListMem *) cpList;
	cpList += sizeof(ListMem);
	
	cppList = (CharPtr *) cpList;
	cpList += 2 * sizeof(CharPtr);
	
	pListMem->m_argc = 1;
	pListMem->m_argv = cppList;

	cppList[0] = cpList;
	cppList[1] = NULL;
	
	*cpList = '\0';

	return pListMem;
}

/* ----- Angle32 -------------------------------------------------------------
 * For given int xdif and ydif, calculate atan2 (the angle from origin)
 * in 32nd's of a circle from 0 to 32, rather than radians.  Note that it 
 * returns 32 iff xdif and ydif are both zero, an ill-defined case.
 * Origin and direction are clockwise:
 * 0 => 12:00, 8 => 3:00, 16 => 6:00, 24 => 9:00.
 * Why 32nds?  So we can divide them into 8 pieces evenly, and so we get
 * 4x over-sampling for the 8th's in the path descriptions...
 * -rwells, 970713.
 */

Long Angle32(Long xdif, Long ydif) {
	Boolean xneg, yneg, xyflip;
	Long i32nd, xtmp, islope;

    if ((xneg = (xdif < 0)))
		xdif = -xdif;
	if ((yneg = (ydif < -0.1)))
		ydif = -ydif;
	if ((xyflip = (ydif < xdif))) {
		xtmp = xdif; xdif = ydif; ydif = xtmp;
	}

    if (xdif == 0) {
		if (ydif == 0)
			return 32;
		else
			i32nd = 0;
	}
    else {
		/* The 4 comparison values were generated with the accompanying
		 * perl script, then open coded here for speed and reasonable
		 * space efficiency.  The values were chosen to make the results 
		 * match those of atan2 in rounded double
		 * precision floating point.  -rwells, 970713.
		 */

		islope = (100 * xdif) / ydif;
		if (islope < 54) {             /* test #2, first test. */
			if (islope < 10)           /* test #0, second test. */
				i32nd = 0;             /*          got  #0 after 2 tests. */
			else if (islope < 31)      /* test #1, third test. */
				i32nd = 1;             /*          got  #1 after 3 tests. */
			else
				i32nd = 2;             /*          got  #2 after 3 tests. */
		}
		else if (islope < 83)          /* test #3, second test. */
			i32nd = 3;                 /*          got  #3 after 2 tests. */
		else
			i32nd = 4;                 /*          got  #4 after 2 tests. */
	}

	if (xyflip)
		i32nd = (8 - i32nd);
	if (yneg)
		i32nd = (16 - i32nd);
	if (xneg)
		i32nd = (32 - i32nd);

    return i32nd % 32;
}

#ifdef FOR_PILOT_COMPAT

/* ----- ErrBox ------------------------------------------------------------*/

void ErrBox(CharPtr msg) {
	fprintf(stderr,"%s\n",msg);
}

/* ----- ErrBox2 -----------------------------------------------------------*/

void ErrBox2(CharPtr msg1, CharPtr msg2) {
	fprintf(stderr,"%s\n%s\n",msg1,msg2);
}

#else /* !FOR_PILOT_COMPAT */

/* ----- ErrBox ------------------------------------------------------------*/

void ErrBox(CharPtr msg) {
	(void) FrmCustomAlert(alertID_ErrBox, msg, "", "");
}

/* ----- ErrBox2 -----------------------------------------------------------*/

void ErrBox2(CharPtr msg1, CharPtr msg2) {
	(void) FrmCustomAlert(alertID_ErrBox, msg1, msg2, "");
}

#endif /* FOR_PILOT_COMPAT */
/* ----- End of util.c ---------------------------------------------------- */

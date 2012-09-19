/* -*- mode: C; c-file-style: "bsd"; tab-width: 4 -*- */
/* scoring.c - The handwriting recognition engine
 * JStroke 1.x - Japanese Kanji handwriting recognition technology demo.
 * Copyright (C) 1997  Robert E. Wells
 * http://wellscs.com/pilot
 * mailto:robert@wellscs.com
 * 
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
 *
 * Derived from prior work by Todd David Rudick on JavaDict and StrokeDic.
 * Makes use of KANJIDIC data from Jim Breen of Monash University.
 * Further credit details available at http://wellscs.com/pilot
 * See readme.txt, changelo.
 * -------------------------------------------------------------------------*/

#include "jstroke.h"
#include "memowrite.h"

#define diAngCostBase       52	// See angles.pl for derivation.
#define diAngCostScale      98	// See angles.pl for derivation.
#define diHugeCost          (((((ULong)24)*diAngCostScale)+diAngCostBase)*100)

#define diMaxScoreToSquare ((ULong) 0xffff)
#define diMaxScoreSquared  (diMaxScoreToSquare*diMaxScoreToSquare)

/* 2 for Kanji SJIS pair, 2+2 for spaces, 
 * 9 for SJIS:967B,
 * 1 for number sign, and 10 for numeric score, a billion served?
 */
#define diScoreTextLen (2 + 2+2 + 9 + 1 + 10)

#define diPathBufLen        16

CharPtr   StrokeScorerEvalItem(StrokeScorer *pScorer, CharPtr cpEntry,
							   ULong* ipScore /*OUT*/);

ULong     StrokeDicScoreStroke(Byte* bpX, Byte* bpY, UInt iLen,
							   CharPtr cpPath, UInt iPathLen,
							   UInt iDepth);

CharPtr   StrokeScorerExtraFilters(StrokeScorer *pscorer,
								   CharPtr cp, ULong* ipScore /*OUT*/);

Long      StrokeScorerExtraEval(StrokeScorer *pscorer,
								char cArg, UInt iStroke);

ULong     SqrtULong(ULong val);

/* ----- SqrtULong ---------------------------------------------------------*/

ULong SqrtULong(ULong val) {
	ULong root, rootLo, rootHi;

	if (val == 0)
		return 0;
	else if (val == 1)
		return 1;
	else if (val >= diMaxScoreSquared)
		return diMaxScoreToSquare;

	rootLo = 1;
	rootHi = (val >> 1);		/* will be >= 1, since val > 1. */
	if (rootHi > diMaxScoreToSquare)
		rootHi = diMaxScoreToSquare;

	while ((root = ((rootLo + rootHi) >> 1)) > rootLo) {
		if ((root * root) <= val) {
			rootLo = root;
		}
		else {
			rootHi = root;
		}
	}

	return root;
}

/* ----- StrokeScorerCreate-------------------------------------------------*/
/* Create a StrokeScorer object. (Returns NULL if can't get memory) */

StrokeScorer *StrokeScorerCreate  (CharPtr cpStrokeDic, RawStroke *rsp,
			 					   UInt iStrokeCnt) {
	StrokeScorer *pScorer = (StrokeScorer *) MemPtrNew(sizeof(StrokeScorer));
	if (!pScorer) {
		ErrBox("Not enough memory.");
		return NULL;
	}

	pScorer->m_cpStrokeDic = cpStrokeDic;
	pScorer->m_pRawStrokes = rsp;
	pScorer->m_iStrokeCnt = iStrokeCnt;
	pScorer->m_iScoreLen = 0;

	pScorer->m_pScores = (ScoreItemPtr) MemPtrNew(diMaxListCount*sizeof(StrokeScorer));
	
	if (!pScorer->m_pScores) {
		ErrBox("Not enough memory.");
		MemPtrFree(pScorer);
		return NULL;
	}

	pScorer->m_cpPath = MemPtrNew(diPathBufLen+1);

	if (!pScorer->m_cpPath) {
		ErrBox("Not enough memory.");
		MemPtrFree(pScorer->m_pScores);
		MemPtrFree(pScorer);
		return NULL;
	}

	return pScorer;
}

/* ----- StrokeScorerDestroy-------------------------------------------------*/
/* Destroy a StrokeScorer object */

void StrokeScorerDestroy  (StrokeScorer *pScorer) {
	if (pScorer) {
		MemPtrFree (pScorer->m_pScores);
		MemPtrFree (pScorer->m_cpPath);
		MemPtrFree (pScorer);
	}
}

/* ----- StrokeScorerProcess-------------------------------------------------*/
/* Process some database entries (maximum iMaxCnt, -1 for all).
   Returns 0 when none remaining (should eventually return count remaining
   to facilitate a progressbar */

Long     StrokeScorerProcess  (StrokeScorer *pScorer, Long iMaxCnt) {
	CharPtr      cp, cpNext;
	ULong        iScore;
	Long         iCnt;
	ScoreItemPtr pScore, pScoreBase, pSrc;

	if (!pScorer) {
		ErrBox("StrokeScorerProcess: pScorer == NULL.");
		return 0;
	}

	pScoreBase = pScorer->m_pScores;

	/* Evaluate all the items in cpStrokeDic against Context,
	 * and update ScoreItems list as we go.
	 */

	iCnt = 0;
	for (cp = pScorer->m_cpStrokeDic; *cp; cp = cpNext) {

		iCnt++;
		if (iMaxCnt >= 0 && iCnt > iMaxCnt)
			break;

		cpNext = StrokeScorerEvalItem(pScorer, cp, &iScore);

		for (pScore = pScoreBase+pScorer->m_iScoreLen-1;
			 pScore>=pScoreBase; pScore--) { 
			if (iScore >= pScore->m_iScore)
				break;
		}
		pScore++;

		/* If we have a top score, lets register it. */
		if (pScore < (pScoreBase + diMaxListCount)) {

			/* Increase the score list length if it isn't full yet. */
			if (pScorer->m_iScoreLen < diMaxListCount)
				pScorer->m_iScoreLen++;

			/* Push down all lower scores in the list to make room. */
			for (pSrc = pScoreBase+pScorer->m_iScoreLen-2; pSrc >= pScore; pSrc--) {
				pSrc[1].m_iScore = pSrc->m_iScore;
				pSrc[1].m_cp     = pSrc->m_cp;
			}

			/* Actually store our info in the list. */
			pScore->m_iScore = iScore;
			pScore->m_cp = cp;
		}

	} /* for each stroke description... */

	if (cp)
		return 1;				/* should be count remaining */
	else
		return 0;
}

/* ----- StrokeScorerTopPicks -----------------------------------------------*/
/* Return best diMaxListCount candidates processed so far */

ListMem*      StrokeScorerTopPicks (StrokeScorer *pScorer)
{
	ListMem*     pListMem;
	UInt         iTotStrLen, iListSize, iTotSize;
	CharPtr      cpList;
	CharPtr*     cppList;
	ScoreItemPtr pScore, pScoreBase;
	UInt         sJisVal;
	char         buf[10];

	if (!pScorer) {
		ErrBox("StrokeScorerTopPicks: pScorer == NULL.");
		return NULL;
	}

	/* If no list, just return an empty list. */
	if (pScorer->m_iScoreLen <= 0)
		return AppEmptyList();

	pScoreBase = pScorer->m_pScores;

	iTotStrLen = pScorer->m_iScoreLen * (diScoreTextLen + 1);

	iListSize = (pScorer->m_iScoreLen+1)*sizeof(CharPtr); /* One extra for NULL terminator */
	iTotSize = sizeof(ListMem) + iListSize + iTotStrLen;

	if (!(cpList = MemPtrNew(iTotSize))) {
		ErrBox("ERROR: no mem in top picks");
		return NULL;
	}

	pListMem = (ListMem *) cpList;	cpList += sizeof(ListMem);
	cppList = (CharPtr *) cpList;	cpList += iListSize;
	
	pListMem->m_argc = pScorer->m_iScoreLen;
	pListMem->m_argv = cppList;	/* Copy now before we use it in loop... */

	for (pScore = pScoreBase; pScore < (pScoreBase+pScorer->m_iScoreLen); pScore++) {
		*cppList++ = cpList;
		*cpList++ = pScore->m_cp[0];
		*cpList++ = pScore->m_cp[1];
		*cpList++ = ' ';
		*cpList++ = ' ';

		*cpList++ = 'S';
		*cpList++ = 'J';
		*cpList++ = 'I';
		*cpList++ = 'S';
		*cpList++ = ':';

		// BUGFIX: Need to avoid sign extension.  -rwells, 19970926.
		sJisVal = ((pScore->m_cp[0] & 0xff) << 8) + (pScore->m_cp[1] & 0xff);

		StrIToH(buf, sJisVal);
		*cpList++ = buf[4];
		*cpList++ = buf[5];
		*cpList++ = buf[6];
		*cpList++ = buf[7];

		*cpList++ = ' ';
		*cpList++ = ' ';
		*cpList++ = '#';
		StrIToA(cpList, pScore->m_iScore); /* Null terminates */
		cpList += StrLen(cpList) + 1; /* Skip ascii number and null. */
	}
	*cppList = NULL;			/* NULL terminated argv vector, for luck. */
	return pListMem;
}

/* ----- StrokeScorerEvalItem -----------------------------------------------*/

CharPtr StrokeScorerEvalItem(StrokeScorer *pScorer, CharPtr cpEntry,
							 ULong* ipScore /*OUT*/) {
	CharPtr cp = cpEntry;
	UInt    iStroke;
	CharPtr cpPath = pScorer->m_cpPath;
	CharPtr cpPathEnd;
	RawStroke* rsp;
	ULong   iThisScore;
	ULong   iScore = 0;

	MemoWriteLen(pScorer->m_cpStrokeDic, 2); /* DEBUG: tag trace with SJIS char. */

	if (*cp) cp++;				/* Skip over first half SJIS char. */
	if (*cp) cp++;				/* Skip over second half SJIS char. */

	/* The first char must have high order bit set,
	 * and the second char MAY have high order bit set,
	 * but a subsequent char with high order bit set must be
	 * the beginning of the next entry.  -rwells, 970712.
	 */

	/* Loop through stroke descriptions */
	for (iStroke = 0; iStroke < pScorer->m_iStrokeCnt; iStroke++) {

		cpPathEnd = cpPath;

		switch (*cp) {		/* Break out on first char value... */
		case 'A':			/* TDR='1' CLK=07:30 DEG=225 */
			*cpPathEnd++ = 20; break; 
		case 'B':			/* TDR='2' CLK=06:00 DEG=180 */
			*cpPathEnd++ = 16; break; 
		case 'C':			/* TDR='3' CLK=04:30 DEG=135 */
			*cpPathEnd++ = 12; break; 
		case 'D':			/* TDR='4' CLK=09:00 DEG=270 */
			*cpPathEnd++ = 24; break; 
		case 'F':			/* TDR='6' CLK=03:00 DEG=090 */
			*cpPathEnd++ =  8; break; 
		case 'G':			/* TDR='7' CLK=10:30 DEG=315 */
			*cpPathEnd++ = 28; break; 
		case 'H':			/* TDR='8' CLK=12:00 DEG=360 */
			*cpPathEnd++ =  0; break; 
		case 'I':			/* TDR='9' CLK=01:30 DEG=045 */
			*cpPathEnd++ =  4; break; 
		case 'J':			/* TDR='x' down   06:00 then 07:30 */
			*cpPathEnd++ = 16; *cpPathEnd++ = 20; break; 
		case 'K':			/* TDR='y' down   06:00 then 04:30 */
			*cpPathEnd++ = 16; *cpPathEnd++ = 12; break; 
		case 'L':			/* TDR='c' down   06:00 then 03:00 */
			*cpPathEnd++ = 16; *cpPathEnd++ =  8; break; 
		case 'M':			/* TDR='b' across 03:00 then 06:00 */
			*cpPathEnd++ =  8; *cpPathEnd++ = 16; break; 
		default:
			goto NoMoreStrokes;
		} /* end switch on first char value */

		for (cp++; ; cp++) {	/* Loop through following chars for stroke */
			switch (*cp) {		/* Break out on char value... */
			case 'a':			/* TDR='1' CLK=07:30 DEG=225 */
				*cpPathEnd++ = 20; break; 
			case 'b':			/* TDR='2' CLK=06:00 DEG=180 */
				*cpPathEnd++ = 16; break; 
			case 'c':			/* TDR='3' CLK=04:30 DEG=135 */
				*cpPathEnd++ = 12; break; 
			case 'd':			/* TDR='4' CLK=09:00 DEG=270 */
				*cpPathEnd++ = 24; break; 
			case 'f':			/* TDR='6' CLK=03:00 DEG=090 */
				*cpPathEnd++ =  8; break; 
			case 'g':			/* TDR='7' CLK=10:30 DEG=315 */
				*cpPathEnd++ = 28; break; 
			case 'h':			/* TDR='8' CLK=12:00 DEG=360 */
				*cpPathEnd++ =  0; break; 
			case 'i':			/* TDR='9' CLK=01:30 DEG=045 */
				*cpPathEnd++ =  4; break; 
			case 'j':			/* TDR='x' down   06:00 then 07:30 */
				*cpPathEnd++ = 16; *cpPathEnd++ = 20; break; 
			case 'k':			/* TDR='y' down   06:00 then 04:30 */
				*cpPathEnd++ = 16; *cpPathEnd++ = 12; break; 
			case 'l':			/* TDR='c' down   06:00 then 03:00 */
				*cpPathEnd++ = 16; *cpPathEnd++ =  8; break; 
			case 'm':			/* TDR='b' across 03:00 then 06:00 */
				*cpPathEnd++ =  8; *cpPathEnd++ = 16; break; 
			default:
				goto ThisStrokeDone;
			} /* end switch on char value */
		} /* end loop through chars for stroke */
ThisStrokeDone:

		rsp = &(pScorer->m_pRawStrokes[iStroke]);

		iThisScore = StrokeDicScoreStroke(rsp->m_x, rsp->m_y, rsp->m_len,
										  cpPath, (cpPathEnd - cpPath),
										  0 /*depth*/);
		
		MemoWrite2d(" s", iStroke+1);
		MemoWrite2d("=", iThisScore); /* DEBUG: stroke score */

		if (iThisScore >= diMaxScoreToSquare)
			iThisScore = diMaxScoreSquared;
		else
			iThisScore = (iThisScore * iThisScore);

		if (iScore >= (diMaxScoreSquared - iThisScore))
			iScore = diMaxScoreSquared;
		else
			iScore += iThisScore;

	} /* end loop through stroke descriptions */
NoMoreStrokes:

	iScore = SqrtULong(iScore);
	*ipScore = iScore;

	MemoWrite2d(" is=", iScore); /* DEBUG: overall stroke score */

    /* Handle optional extra filters... may modify *ipScore, updates cp. */
	if (*cp == '|') {
		cp = StrokeScorerExtraFilters(pScorer, cp+1, ipScore);
	}

	if (iStroke != pScorer->m_iStrokeCnt)
		ErrBox("JStrokeDic miscount");
	if (!(*cp & 0x80) && (*cp != '\0'))
		ErrBox("JStrokeDic leftovers");


	MemoWrite2d(" fs=", *ipScore); /* DEBUG: final score */
	MemoWrite("\n");

	return cp;
}

/* ----- StrokeDicScoreStroke ---------------------------------------------- */

ULong StrokeDicScoreStroke(Byte* bpX, Byte* bpY, UInt iLen,
						   CharPtr cpPath, UInt iPathLen,
								  UInt iDepth) {
	ULong iScore, iThisScore;
	Long iMid, iStep, iPathMid, iPathRest;
	Long iDifX, iDifY;
	UInt iAng32, iPath32, iDif32;

	if (iLen < 2 || iPathLen < 1)
		return diHugeCost;

	if (iPathLen == 1) {
		iDifX = bpX[iLen-1] - bpX[0];
		iDifY = bpY[0] - bpY[iLen-1]; /* Flip from display to math axes. */

		if (iDifX == 0 && iDifY == 0) /* Two samples at same place... */
			return diHugeCost;

		/* Subdivide recursively while stroke is long and depth is shallow.
		 * $$$ These values are pretty magic... review later. -rwells, 970719.
		 * TDR used 20*20... -rwells, 970719.
		 */
		if ((iDifX*iDifX + iDifY*iDifY) > (20*20) && iLen > 5 && iDepth < 4) {

			iMid = iLen >> 1;

			/* Note that we use the middle point on both sides... */

			iScore  = StrokeDicScoreStroke(bpX, bpY, iMid+1,
										   cpPath, iPathLen, iDepth+1);

			iScore += StrokeDicScoreStroke(bpX+iMid, bpY+iMid, iLen-iMid,
										   cpPath, iPathLen, iDepth+1);

			return (iScore >> 1);

		} /* End if stroke is long, and depth is shallow... */

		/* Time to score this segment against desired direction. */
		
		iAng32 = Angle32(iDifX, iDifY);

		iPath32 = *cpPath;

		if (iAng32 >= iPath32)
			iDif32 = iAng32 - iPath32;
		else
			iDif32 = iPath32 - iAng32;

		return iDif32 * diAngCostScale + diAngCostBase;

	} /* end if path len is 1. */
	else {

		iScore = diHugeCost * iPathLen * 2;
		iPathMid = iPathLen >> 1;
		iPathRest = iPathLen - iPathMid;
		
		if (iLen < 20)
			iStep = 1;
		else
			iStep = iLen / 10;

		for (iMid = iPathMid; iMid < iLen - iPathRest; iMid += iStep) {

			/* TDR original doesn't increase iDepth... -rwells, 970719. */

			iThisScore  = StrokeDicScoreStroke(bpX, bpY, iMid+1,
											   cpPath, iPathMid, iDepth+1);

			iThisScore += StrokeDicScoreStroke(bpX+iMid, bpY+iMid, iLen-iMid,
											   cpPath+iPathMid, iPathRest,
											   iDepth+1);

			/* TDR original doesn't divide sum by 2... -rwells, 970719. */
			iThisScore >>= 1;

			if (iThisScore < iScore)
				iScore = iThisScore;

		} /* end for trials on various mid-point divisions of stroke... */

		return iScore;
	} /* end if path len is >1. */
}

/* ----- StrokeScorerExtraFilters ---------------------------------------------*/

CharPtr StrokeScorerExtraFilters(StrokeScorer *pScorer,
								 CharPtr cp, ULong* ipScore /*OUT*/) {
	char    c;
	char    cArg[2];
	Byte    iStroke[2];
	Long    iDiff, iVal[2];
	UInt    idx = 0;
	Boolean bMust = false;
  
	MemoWrite(" F(");

	cArg[0] = cArg[1] = 0;
	iStroke[0] = iStroke[1] = 0;

    /* Simple parser for Filter strings. assumes a1-b1 structure,
	 * where a and b can be any single alphabetic cmd char, the 
	 * numbers can be multiple digit, and b1 can optionally be 
	 * followed by '!' to insist on the filter passing.  There can
	 * be multiple filters but they have to be separated by '!' or
	 * space(s).  The filter string is terminated by a null byte
	 * or an 8-bit char, the beginning of the next entry.  
	 * Leading spaces and trailing spaces are ignored. -rwells, 970722.
	 */

	for (c = *cp; true; cp++, c = *cp) {
		switch (c) {

		case 'x': 
		case 'y':
		case 'i':
		case 'j':
		case 'a':
		case 'b':
		case 'l':
			cArg[idx] = c;

			MemoWrite2d(" c",idx);
			MemoWrite("=");
			MemoWriteLen(cArg+idx, 1);
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			iStroke[idx] = iStroke[idx] * 10 + (c - '0');
			MemoWrite2d(":", iStroke[idx]);
			break;

		case '-':				/* Switch to second arg when we see minus. */
			idx = 1;
			MemoWrite(" - ");
			break;

		case '!':
			bMust = true;
			MemoWrite(" ! ");
			/* FALLTHRU */

		case ' ':
		default:
			MemoWrite2d(" @", idx);
			MemoWrite(")");

			/* If we are in the second argument, execute and reset. */
			if (idx == 1) {
				iVal[0] = StrokeScorerExtraEval(pScorer, cArg[0], iStroke[0]);
				iVal[1] = StrokeScorerExtraEval(pScorer, cArg[1], iStroke[1]);
				iDiff = (iVal[0] - iVal[1]);

				MemoWrite(" ");
				MemoWriteLen(cArg+0, 1);
				MemoWrite2d("", iStroke[0]);
				MemoWrite2d(":", iVal[0]);
				
				MemoWrite("-");
				MemoWriteLen(cArg+1, 1);
				MemoWrite2d("", iStroke[1]);
				MemoWrite2d(":", iVal[1]);
				
				MemoWrite2d("=", iDiff);

				if (iDiff < 0) {
					iDiff = -iDiff;
					if (bMust)
						iDiff = 9999999;
					if (*ipScore < (diMaxScoreSquared-iDiff))
						*ipScore += iDiff;
					else
						*ipScore = diMaxScoreSquared;
				}
				else {
					if (*ipScore > iDiff)
						*ipScore -= iDiff;
					else
						*ipScore = 0;
				}

				MemoWrite2d(" ips=", *ipScore);

				/* Reset state for next filter... */
				idx = 0;
				bMust = false;
				cArg[0] = cArg[1] = 0;
				iStroke[0] = iStroke[1] = 0;
			}

			/* If this is a terminating char, break out of loop and return. */
			if ((c & 0x80) || (c == '\0'))
				goto ReturnNow;
		} /* end switch on char */
	} /* end for each char in filter spec... */
ReturnNow:
	return cp;
}

/* ----- StrokeScorerExtraEval ------------------------------------------------*/

Long StrokeScorerExtraEval(StrokeScorer *pScorer,
						   char cArg, UInt iStroke) {
	Long  iVal, iSquared;
	Byte* bpX;
	Byte* bpY;
	UInt  iLen;
	RawStroke* rsp;

	iStroke--;					/* Formula strokes are 1-based, my stroke
								 * array is 0-based. */

	if (iStroke >= pScorer->m_iStrokeCnt)
		return 0;				/* Should not happen... */


	rsp = pScorer->m_pRawStrokes + iStroke;
	iLen = rsp->m_len;
	bpX = rsp->m_x;
	bpY = rsp->m_y;

	/* Heretofore we have left the coordinates as we got them, screen
	 * coordinates with 0,0 in upper left, y increasing downward,
	 * x increasing to the right.  And the extra filters agree with this.
	 * -rwells, 970723.
	 *
	 * Removed the subtraction of Rect_x, Rect_y here - it will mess
	 * up debugging messages if the client doesn't subtract them before-
	 * hand, but otherwise should be harmless.
	 *  - OWT, 970903
	 */

	switch(cArg) {
	case 'x':
		return bpX[0];
	case 'y':
		return bpY[0];
	case 'i':
		return bpX[iLen-1];
	case 'j':
		return bpY[iLen-1];
	case 'a':
		return ((bpX[0] + bpX[iLen-1]) >> 1);
	case 'b':
		return ((bpY[0] + bpY[iLen-1]) >> 1);
	
	case 'l':
		/* These are byte values - they can't get very large,
		 * so don't need to check against diMaxScoreToSquare...
		 */
		iSquared = (((Long) bpX[0]) - ((Long)bpX[iLen-1]));
		iVal = iSquared * iSquared;

		iSquared = (((Long) bpY[0]) - ((Long)bpY[iLen-1]));
		iVal += iSquared * iSquared;

		return (Long) SqrtULong((Long) iVal);
	}
	
	return 0;					/* Unrecognized cArg cmd char. */
}
/* ----- end of scoring.c --------------------------------------------------*/

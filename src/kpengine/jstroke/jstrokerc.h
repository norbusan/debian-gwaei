// -*- mode: C++; c-file-style: "bsd"; tab-width: 4 -*- 
// jstrokerc.h - Pilot resource file IDs included by resource file and C code.
// JStroke 1.x - Japanese Kanji handwriting recognition technology demo.
// Copyright (C) 1997  Robert E. Wells
// http://wellscs.com/pilot
// mailto:robert@wellscs.com
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program (gpl.html); if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Derived from prior work by Todd David Rudick on JavaDict and StrokeDic.
// Makes use of KANJIDIC data from Jim Breen of Monash University.
// Further credit details available at http://wellscs.com/pilot
// See readme.txt, changelo, and gpl.html for more information.
//
// The pilrc tool is very picky about its input, and this file is included
// by jstroke.rcp.  Only C++ style comments are supported, and there can't
// be any comments once the #define lines have started.  -rwells, 19970921.
// ---------------------------------------------------------------------------

#define formID_App               1000

#define menuID_App               2000

#define menuitemID_about         3000
#define menuitemID_copy          3001
#define menuitemID_paste         3002

#define fieldID_text             4001

#define fieldID_textMaxChars       80

#define alertID_about            5000
#define alertID_ErrBox           5001

#define buttonID_recognize       6000
#define buttonID_erase           6001
#define buttonID_angles          6002
#define buttonID_replay          6003

#define poptrigID_kanji          7000

#define listID_kanji             8000

/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//!  @file result.c
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <libwaei/libwaei.h>



//!
//! @brief Creates a new LwResult object
//! @return An allocated LwResult that will be needed to be freed by lw_result_free.
//!
LwResult* 
lw_result_new ()
{
    LwResult* temp;

    temp = (LwResult*) malloc(sizeof(LwResult));

    if (temp != NULL)
    {
      lw_result_init (temp);
    }

    return temp;
}

//!
//! @brief Releases a LwResult object from memory.
//! @param result A LwResult object created by lw_result_new.
//!
void 
lw_result_free (LwResult *result)
{
    lw_result_deinit (result);
    free (result);
}


//!
//! @brief Used to initialize the memory inside of a new LwDictInfo
//!        object.  Usually lw_dictinfo_new calls this for you.  It is also 
//!        used in class implimentations that extends LwDictInfo.
//! @param result The LwResultline to initialize the memory of
//!
void 
lw_result_init (LwResult *result)
{
    lw_result_clear (result);
}


//!
//! @brief Used to free the memory inside of a LwResult object.
//!         Usually lw_dictinfo_free calls this for you.  It is also used
//!         in class implimentations that extends LwResult.
//! @param result The LwResult object to have it's inner memory freed.
//!
void 
lw_result_deinit (LwResult *result)
{
}

void 
lw_result_clear (LwResult *result)
{
    //A place for a copy of the raw string
    result->text[0] = '\0';

    result->relevance = LW_RELEVANCE_UNSET;
    
    //General formatting
    result->def_start[0] = NULL;
    result->def_total = 0;
    result->kanji_start = NULL;
    result->furigana_start = NULL;
    result->classification_start = NULL;
    result->important = FALSE;

    //Kanji things
    result->strokes = NULL;
    result->frequency = NULL;
    result->readings[0] = NULL;
    result->readings[1] = NULL;
    result->readings[2] = NULL;
    result->meanings = NULL;
    result->grade = NULL;
    result->jlpt = NULL;
    result->kanji = NULL;
    result->radicals = NULL;
}


gboolean 
lw_result_is_similar (LwResult *result1, LwResult *result2)
{
    //Declarations
    gboolean same_def_totals, same_first_def;

    if (result1 == NULL || result2 == NULL) return FALSE;

    //Initializations
    same_def_totals = (result1->def_total == result2->def_total);
    same_first_def = (strcmp(result1->def_start[0], result2->def_start[0]) == 0);

    return (same_first_def && same_def_totals);
}


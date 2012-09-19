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
//! @file engine-data.c
//!

#include "../private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include <libwaei/engine-data.h>


//!
//! @brief Creates a new LwEngineData object
//! @param engine The LwEngine to include with the LwEngineData 
//! @param item The LwSearchItem to include with the LwEngineData
//! @param exact Whether only exact matching results should be shown for the search
//! @return An allocated LwEngineData that will be needed to be freed by lw_engindata_free.
//!
LwEngineData* 
lw_enginedata_new (LwSearchItem *item, gboolean exact)
{
    LwEngineData *temp;

    temp = (LwEngineData*) malloc(sizeof(LwEngineData));

    if (temp != NULL)
    {
      temp->item = item;
      temp->exact = exact;
    }

    return temp;
}


//!
//! @brief Releases a LwEngineData object from memory.
//! @param di A LwEngineData object created by lw_enginedata_new.
//!
void 
lw_enginedata_free (LwEngineData *data)
{
    if (data != NULL) free (data);
}


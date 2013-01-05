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
//! @file search-data.c
//!
//! @brief To be written
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <waei/waei.h>

#include <waei/gettext.h>


WSearchData* 
w_searchdata_new (GMainLoop *loop, WApplication *application)
{
    //Sanity check
    g_assert (loop != NULL && application != NULL);

    //Declarations
    WSearchData *temp;

    //Initializations
    temp = g_new (WSearchData, 1);

    if (temp != NULL)
    {
      temp->loop = loop;
      temp->application = application;
      temp->less_relevant_header_set = FALSE;
    }

    return temp;
}


void 
w_searchdata_free (WSearchData *sdata)
{
  g_free (sdata);
}


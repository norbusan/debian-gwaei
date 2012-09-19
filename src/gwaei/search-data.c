/******************************************************************************
    AUTHOR:
    KanjiPad - Japanese handwriting recognition front end
    Copyright (C) 1997 Owen Taylor
    File heavily modified and updated by Zachary Dovel.

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


#include "../private.h"

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


GwSearchData* 
gw_searchdata_new (GtkTextView *view, GwSearchWindow *window)
{
    GwSearchData *temp;
    temp = (GwSearchData*) malloc(sizeof(GwSearchData));
    if (temp != NULL)
    {
      temp->window = window;
      temp->view = view;
      temp->resultline = NULL;
    }
    return temp;
}


void 
gw_searchdata_free (GwSearchData *data)
{
    g_assert (data != NULL);

    if (data->resultline != NULL) lw_resultline_free (data->resultline);

    data->window = NULL;
    data->view = NULL;
    data->resultline = NULL;

    free (data);
}


void 
gw_searchdata_set_resultline (GwSearchData *data, LwResultLine *resultline)
{
    g_assert (data != NULL);

    if (data->resultline != NULL) 
      lw_resultline_free (data->resultline);
    data->resultline = resultline;
}

LwResultLine* 
gw_searchdata_get_resultline (GwSearchData *data)
{
  return data->resultline;
}



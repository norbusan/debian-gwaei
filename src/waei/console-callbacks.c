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
//! @file console-callbacks.c
//!
//! @brief To be written
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <waei/waei.h>

int 
w_console_uninstall_progress_cb (double fraction, gpointer data)
{
  //Declarations
  LwDictInfo *di;
  char *uri;

  //Initializations
  di = data;
  uri = lw_util_build_filename_by_dicttype (di->type, di->filename);

  printf("Removing %s...\n", uri);

  g_free (uri);

  return FALSE;
}




static gboolean _group_index_changed = FALSE;
static int _previous_percent = -1;
int 
w_console_install_progress_cb (double fraction, gpointer data)
{
  //Declarations
  LwDictInst *di;
  char *status;
  double current_fraction;
  int current_percent;

  //Initializations
  di = data;
  current_fraction = lw_dictinst_get_process_progress (di, fraction);
  current_percent = (int) (100.0 * current_fraction); 

  //Update the dictinst progress state only when the delta is large enough
  if (current_percent < 100 && _group_index_changed)
  {
    _group_index_changed = FALSE;
    printf("\n");
  }
  else if (current_percent == 100)
  {
    _group_index_changed = TRUE;
  }

  status = lw_dictinst_get_status_string (di, TRUE);
  printf("\r [%d%%] %s", current_percent, status);
  _previous_percent = current_percent;
  g_free (status);

  return FALSE;
}


gboolean 
w_console_append_result_timeout (gpointer data)
{
  LwSearchItem *item;
  WSearchData *sdata;
  int chunk;
  int max_chunk;
  gboolean is_still_searching;

  item = LW_SEARCHITEM (data);
  sdata = W_SEARCHDATA (lw_searchitem_get_data (item));
  chunk = 0;
  max_chunk = 50;

  if (item != NULL && lw_searchitem_should_check_results (item))
  {
    while (item != NULL && lw_searchitem_should_check_results (item) && chunk < max_chunk)
    {
      w_console_append_result (sdata->application, item);
      chunk++;
    }
    is_still_searching = TRUE;
  }
  else
  {
      w_console_no_result (sdata->application, item);
      g_main_loop_quit (sdata->loop);
      is_still_searching = FALSE;
  }

  return is_still_searching;
}

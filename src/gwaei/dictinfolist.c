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
//! @file dictinfolist.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static void _dictinfolist_attach_signals (GwDictInfoList*);
static void gw_dictinfolist_rebuild_liststore (GwDictInfoList*);

//!
//! @brief Sets up the dictionary manager.  This is the backbone of every portion of the GUI that allows editing dictionaries
//!
GwDictInfoList* gw_dictinfolist_new (const int MAX, GwApplication *application)
{
    GwDictInfoList *temp;
    LwPreferences *preferences;

    temp = (GwDictInfoList*) malloc(sizeof(GwDictInfoList));

    if (temp != NULL)
    {
      preferences = gw_application_get_preferences (application);
      lw_dictinfolist_init (LW_DICTINFOLIST (temp), MAX, preferences);
      gw_dictinfolist_init (temp, application);
    }
    return temp;
}


void gw_dictinfolist_free (GwDictInfoList *dil)
{
    gw_dictinfolist_deinit (dil);
    lw_dictinfolist_deinit (LW_DICTINFOLIST (dil));
    free (dil);
}


void gw_dictinfolist_init (GwDictInfoList *dil, GwApplication *application)
{
    //Declarations
    int i;

    //Setup the model and view
    dil->model = gtk_list_store_new (
        TOTAL_GW_DICTINFOLIST_COLUMNS, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_POINTER);

    for (i = 0; i < TOTAL_GW_DICTINFOLIST_SIGNALIDS; i++)
      dil->signalids[i] = 0;

    dil->application = application;

    gw_dictinfolist_rebuild_liststore (dil);

    _dictinfolist_attach_signals (dil);
}


void gw_dictinfolist_deinit (GwDictInfoList *dil)
{
    g_object_unref (dil->model);
}


static void _dictinfolist_attach_signals (GwDictInfoList *dil)
{
    dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED] = g_signal_connect (
          G_OBJECT (dil->model),
          "row-deleted", 
          G_CALLBACK (gw_dictinfolist_list_store_row_changed_action_cb),
          dil
    );
}


static void gw_dictinfolist_rebuild_liststore (GwDictInfoList *dil)
{
    //Declarations
    LwDictInfo *di;
    GtkTreeIter tree_iter;
    char *iconname;
    char shortcutname[10];
    char ordernumber[10];
    char *favoriteicon;
    GList *list_iter;

    favoriteicon = "emblem-favorite";

    gtk_list_store_clear (GTK_LIST_STORE (dil->model));

    for (list_iter = dil->list; list_iter != NULL; list_iter = list_iter->next)
    {
      di = LW_DICTINFO (list_iter->data);
      if (di == NULL) continue;

      if (di->load_position == 0)
         iconname = favoriteicon;
      else
        iconname = NULL;
      if (di->load_position + 1 < 10)
        sprintf (shortcutname, "Alt-%d", (di->load_position + 1));
      else
        strcpy(shortcutname, "");
      if ((di->load_position + 1) < 1000)
        sprintf (ordernumber, "%d", (di->load_position + 1));
      else
        strcpy(ordernumber, "");

      gtk_list_store_append (GTK_LIST_STORE (dil->model), &tree_iter);
      gtk_list_store_set (
          dil->model, &tree_iter,
          GW_DICTINFOLIST_COLUMN_IMAGE,        iconname,
          GW_DICTINFOLIST_COLUMN_POSITION,     ordernumber,
          GW_DICTINFOLIST_COLUMN_NAME,         di->shortname,
          GW_DICTINFOLIST_COLUMN_LONG_NAME,    di->longname,
          GW_DICTINFOLIST_COLUMN_ENGINE,       lw_util_dicttype_to_string (di->type),
          GW_DICTINFOLIST_COLUMN_SHORTCUT,     shortcutname,
          GW_DICTINFOLIST_COLUMN_DICT_POINTER, di,
          -1
      );
    }
}


//!
//! Sets updates the list of dictionaries against the list in the global dictlist
//!
void gw_dictinfolist_reload (GwDictInfoList *dil)
{
    LwPreferences *preferences;

    preferences = gw_application_get_preferences (dil->application);

    lw_dictinfolist_reload (LW_DICTINFOLIST (dil));
    lw_dictinfolist_load_dictionary_order_from_pref (LW_DICTINFOLIST (dil), preferences);

    if (dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_block (dil->model, dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED]);

    gw_dictinfolist_rebuild_liststore (dil);

    if (dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_unblock (dil->model, dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED]);
}




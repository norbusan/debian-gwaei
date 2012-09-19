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
//! @file dictionarystore.c
//!
//! @brief To be written
//!



#include "../private.h"

#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/dictionarystore-private.h>

static void gw_dictionarystore_attach_signals (GwDictionaryStore*);

G_DEFINE_TYPE (GwDictionaryStore, gw_dictionarystore, GTK_TYPE_LIST_STORE)

//!
//! @brief Sets up the dictionary manager.  This is the backbone of every portion of the GUI that allows editing dictionaries
//!
GtkListStore* gw_dictionarystore_new ()
{
    //Declarations
    GwDictionaryStore *model;

    //Initializations
    model = GW_DICTIONARYSTORE (g_object_new (GW_TYPE_DICTIONARYSTORE, NULL));

    return GTK_LIST_STORE (model);
}


void static
gw_dictionarystore_init (GwDictionaryStore *store)
{
    GType types[] = { 
        G_TYPE_STRING, //GW_DICTIONARYSTORE_COLUMN_IMAGE
        G_TYPE_STRING, //GW_DICTIONARYSTORE_COLUMN_POSITION
        G_TYPE_STRING, //GW_DICTIONARYSTORE_COLUMN_NAME
        G_TYPE_STRING, //GW_DICTIONARYSTORE_COLUMN_LONG_NAME
        G_TYPE_STRING, //GW_DICTIONARYSTORE_COLUMN_ENGINE
        G_TYPE_STRING, //GW_DICTIONARYSTORE_COLUMN_SHORTCUT
        G_TYPE_POINTER //GW_DICTIONARYSTORE_COLUMN_DICT_POINTER
    };

    gtk_list_store_set_column_types (GTK_LIST_STORE (store), TOTAL_GW_DICTIONARYSTORE_COLUMNS, types);
    store->priv = GW_DICTIONARYSTORE_GET_PRIVATE (store);
    memset(store->priv, 0, sizeof(GwDictionaryStorePrivate));

    gw_dictionarystore_update (store);

    gw_dictionarystore_attach_signals (store);
}


static void 
gw_dictionarystore_finalize (GObject *object)
{
    GwDictionaryStore *store;
    GwDictionaryStorePrivate *priv;

    store = GW_DICTIONARYSTORE (object);
    priv = store->priv;

    if (priv->dictinfolist != NULL) lw_dictinfolist_free (priv->dictinfolist); priv->dictinfolist = NULL;

    G_OBJECT_CLASS (gw_dictionarystore_parent_class)->finalize (object);
}


static void
gw_dictionarystore_class_init (GwDictionaryStoreClass *klass)
{
    //Declarations
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = gw_dictionarystore_finalize;

    g_type_class_add_private (object_class, sizeof (GwDictionaryStorePrivate));
}


static void 
gw_dictionarystore_attach_signals (GwDictionaryStore *store)
{
    GwDictionaryStorePrivate *priv;

    priv = store->priv;

    priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED] = g_signal_connect (
      G_OBJECT (store),
      "row-deleted", 
      G_CALLBACK (gw_dictionarystore_row_changed_cb),
      NULL
    );
}


LwDictInfoList*
gw_dictionarystore_get_dictinfolist (GwDictionaryStore *store)
{
    GwDictionaryStorePrivate *priv;
    priv = store->priv;

    if (priv->dictinfolist == NULL)
    {
      priv->dictinfolist = lw_dictinfolist_new (20);
    }

    return priv->dictinfolist;
}


void 
gw_dictionarystore_update (GwDictionaryStore *store)
{
    //Declarations
    GwDictionaryStorePrivate *priv;
    LwDictInfoList *dictinfolist;
    LwDictInfo *di;
    GtkTreeIter iter;
    gchar shortcutname[10];
    gchar ordernumber[10];
    const gchar *iconname;
    const gchar *favoriteicon;
    GList *link;

    priv = store->priv;

    if (priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_block (store, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    dictinfolist = gw_dictionarystore_get_dictinfolist (store);
    favoriteicon = "emblem-favorite";
    gtk_list_store_clear (GTK_LIST_STORE (store));

    for (link = dictinfolist->list; link != NULL; link = link->next)
    {
      di = LW_DICTINFO (link->data);
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

      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (
          GTK_LIST_STORE (store), &iter,
          GW_DICTIONARYSTORE_COLUMN_IMAGE,        iconname,
          GW_DICTIONARYSTORE_COLUMN_POSITION,     ordernumber,
          GW_DICTIONARYSTORE_COLUMN_NAME,         di->shortname,
          GW_DICTIONARYSTORE_COLUMN_LONG_NAME,    di->longname,
          GW_DICTIONARYSTORE_COLUMN_ENGINE,       lw_util_dicttype_to_string (di->type),
          GW_DICTIONARYSTORE_COLUMN_SHORTCUT,     shortcutname,
          GW_DICTIONARYSTORE_COLUMN_DICT_POINTER, di,
          -1
      );
    }

    if (priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_unblock (store, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);
}


void
gw_dictionarystore_load_order (GwDictionaryStore *store, LwPreferences *preferences)
{
    GwDictionaryStorePrivate *priv;
    LwDictInfoList *dictinfolist;

    priv = store->priv;
    dictinfolist = gw_dictionarystore_get_dictinfolist (store);

    if (priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_block (store, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    lw_dictinfolist_load_order (dictinfolist, preferences);
    gw_dictionarystore_update (store);

    if (priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_unblock (store, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);
}


void
gw_dictionarystore_normalize (GwDictionaryStore *store)
{
    //Declarations
    GwDictionaryStorePrivate *priv;
    GtkTreeModel *model;
    gint position;
    gpointer ptr;
    GtkTreeIter iter;
    LwDictInfo *di;
    LwDictInfoList *dictinfolist;
    gboolean valid;

    //Initializations
    dictinfolist = gw_dictionarystore_get_dictinfolist (store);
    priv = store->priv;
    position = 0;
    model = GTK_TREE_MODEL (store);

    g_signal_handler_block (model, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid)
    {
      gtk_tree_model_get (model, &iter, GW_DICTIONARYSTORE_COLUMN_DICT_POINTER, &ptr, -1);
      if (ptr != NULL)
      {
        di = LW_DICTINFO (ptr);
        di->load_position = position;
        position++;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    g_signal_handler_unblock (model, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    lw_dictinfolist_sort_and_normalize_order (dictinfolist);
}


void
gw_dictionarystore_save_order (GwDictionaryStore *store, LwPreferences *preferences)
{
    //Declarations
    GwDictionaryStorePrivate *priv;
    GtkTreeModel *model;
    gint position;
    gpointer ptr;
    GtkTreeIter iter;
    LwDictInfo *di;
    LwDictInfoList *dictinfolist;
    gboolean valid;

    //Initializations
    dictinfolist = gw_dictionarystore_get_dictinfolist (store);
    priv = store->priv;
    position = 0;
    model = GTK_TREE_MODEL (store);

    g_signal_handler_block (model, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid)
    {
      gtk_tree_model_get (model, &iter, GW_DICTIONARYSTORE_COLUMN_DICT_POINTER, &ptr, -1);
      if (ptr != NULL)
      {
        di = LW_DICTINFO (ptr);
        di->load_position = position;
        position++;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    g_signal_handler_unblock (model, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    gw_dictionarystore_normalize (store);
    lw_dictinfolist_save_order (dictinfolist, preferences);
    gw_dictionarystore_update (store);
}


//!
//! Sets updates the list of dictionaries against the list in the global dictlist
//!
void 
gw_dictionarystore_reload (GwDictionaryStore *store, LwPreferences *preferences)
{
    //Declarations
    GwDictionaryStorePrivate *priv;
    LwDictInfoList *dictinfolist;

    //Initializations
    priv = store->priv;
    dictinfolist = gw_dictionarystore_get_dictinfolist (store);

    lw_dictinfolist_reload (dictinfolist);

    if (priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_block (store, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    gw_dictionarystore_load_order (store, preferences);

    if (priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_unblock (store, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);
}




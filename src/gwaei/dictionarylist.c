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
//! @file dictionarylist.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/dictionarylist-private.h>

static void gw_dictionarylist_attach_signals (GwDictionaryList*);

G_DEFINE_TYPE (GwDictionaryList, gw_dictionarylist, LW_TYPE_DICTIONARYLIST)

//!
//! @brief Sets up the dictionary manager.  This is the backbone of every portion of the GUI that allows editing dictionaries
//!
GwDictionaryList* gw_dictionarylist_new ()
{
    //Declarations
    GwDictionaryList *dictionarylist;

    //Initializations
    dictionarylist = GW_DICTIONARYLIST (g_object_new (GW_TYPE_DICTIONARYLIST, NULL));

    return GW_DICTIONARYLIST (dictionarylist);
}


void static
gw_dictionarylist_init (GwDictionaryList *dictionarylist)
{
    dictionarylist->priv = GW_DICTIONARYLIST_GET_PRIVATE (dictionarylist);
    memset(dictionarylist->priv, 0, sizeof(GwDictionaryListPrivate));

    GwDictionaryListPrivate *priv;

    priv = dictionarylist->priv;

    GType types[] = { 
        G_TYPE_STRING,  //GW_DICTIONARYLIST_COLUMN_IMAGE
        G_TYPE_STRING,  //GW_DICTIONARYLIST_COLUMN_POSITION
        G_TYPE_STRING,  //GW_DICTIONARYLIST_COLUMN_NAME
        G_TYPE_STRING,  //GW_DICTIONARYLIST_COLUMN_LONG_NAME
        G_TYPE_STRING,  //GW_DICTIONARYLIST_COLUMN_ENGINE
        G_TYPE_STRING,  //GW_DICTIONARYLIST_COLUMN_SHORTCUT
        G_TYPE_BOOLEAN, //GW_DICTIONARYLIST_COLUMN_SELECTED
        G_TYPE_POINTER  //GW_DICTIONARYLIST_COLUMN_DICT_POINTER
    };

    priv->liststore = gtk_list_store_newv (TOTAL_GW_DICTIONARYLIST_COLUMNS, types);
    priv->menumodel = G_MENU_MODEL (g_menu_new ());

    gw_dictionarylist_attach_signals (dictionarylist);
}


static void 
gw_dictionarylist_finalize (GObject *object)
{
    GwDictionaryList *dictionarylist;
    GwDictionaryListPrivate *priv;

    dictionarylist = GW_DICTIONARYLIST (object);
    priv = dictionarylist->priv;

    if (priv->menumodel != NULL) g_object_unref (priv->menumodel); priv->menumodel = NULL;
    if (priv->liststore != NULL) g_object_unref (priv->liststore); priv->liststore = NULL;

    G_OBJECT_CLASS (gw_dictionarylist_parent_class)->finalize (object);
}


static void
gw_dictionarylist_class_init (GwDictionaryListClass *klass)
{
    //Declarations
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = gw_dictionarylist_finalize;

    g_type_class_add_private (object_class, sizeof (GwDictionaryListPrivate));
}


static void 
gw_dictionarylist_attach_signals (GwDictionaryList *dictionarylist)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL);    

    g_signal_connect (dictionarylist, "changed", G_CALLBACK (gw_dictionarylist_changed_cb), NULL);
}


static void
gw_dictionarylist_menumodel_append (GwDictionaryList *dictionarylist, 
                                    LwDictionary     *dictionary)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL);
    g_return_if_fail (dictionary != NULL);

    //Declarations
    GMenuModel *menumodel;
    GMenuItem *menuitem;
    gint index;
    gchar *detailed_action;
    const gchar *shortname;
    gchar *longname;
    gchar accel[12];

    //Initializations
    menumodel = gw_dictionarylist_get_menumodel (dictionarylist);
    index = g_menu_model_get_n_items (menumodel) + 1;
    longname = NULL;
    detailed_action = NULL;
    menuitem = NULL;
    *accel = '\0';
    shortname = lw_dictionary_get_name (dictionary);

    longname = g_strdup_printf (gettext("%s Dictionary"), shortname);
    if (longname == NULL) goto errored;
    detailed_action = g_strdup_printf ("win.set-dictionary::%d", index);
    if (detailed_action == NULL) goto errored;
    if (index < 10) sprintf (accel, "<Primary>%d", index);

    menuitem = g_menu_item_new (longname, detailed_action);
    if (menuitem == NULL) goto errored;
    g_menu_item_set_attribute (menuitem, "accel", "s", accel);
    g_menu_append_item (G_MENU (menumodel), menuitem);

errored:
    if (longname != NULL) g_free (longname); longname = NULL;
    if (detailed_action != NULL) g_free (detailed_action); detailed_action = NULL;
    if (menuitem != NULL) g_object_unref (menuitem); menuitem = NULL;
}


void
gw_dictionarylist_sync_menumodel (GwDictionaryList *dictionarylist)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL);

    //Declarations
    GMenuModel *menumodel;
    GMenu *menu;
    GList *link;
    LwDictionary *dictionary;

    //Initializations
    menumodel = gw_dictionarylist_get_menumodel (dictionarylist);
    menu = G_MENU (menumodel);
    link = lw_dictionarylist_get_list (LW_DICTIONARYLIST (dictionarylist));

    while (g_menu_model_get_n_items (menumodel) > 0)
    {
      g_menu_remove (menu, 0);
    }

    while (link != NULL)
    {
      dictionary = LW_DICTIONARY (link->data);

      gw_dictionarylist_menumodel_append (dictionarylist, dictionary);

      link = link->next;
    }
}


static void
gw_dictionarylist_liststore_append (GwDictionaryList *dictionarylist, LwDictionary *dictionary)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL);
    g_return_if_fail (dictionary != NULL);

    //Declarations
    GtkListStore *liststore;
    GtkTreeModel *treemodel;
    GtkTreeIter iter;
    gchar shortcutname[10];
    gchar ordernumber[10];
    const gchar *iconname;
    const static gchar *favoriteicon = "emblem-favorite";
    const gchar *shortname;
    gchar *longname;
    gchar *directoryname;
    gint index;
    gboolean selected;

    //Initializations
    liststore = gw_dictionarylist_get_liststore (dictionarylist);
    treemodel = GTK_TREE_MODEL (liststore);
    index = gtk_tree_model_iter_n_children (treemodel, NULL) + 1;
    shortname = lw_dictionary_get_name (dictionary);
    iconname = NULL;
    *shortcutname = '\0';
    *ordernumber = '\0';
    longname = NULL;
    directoryname = NULL;

    longname = g_strdup_printf (gettext("%s Dictionary"), shortname);
    if (longname == NULL) goto errored;
    directoryname = lw_dictionary_get_directoryname (G_OBJECT_TYPE (dictionary));
    if (directoryname == NULL) goto errored;
    if (index == 1) iconname = favoriteicon;
    if (index < 10) sprintf (shortcutname, "Alt-%d", index);
    if (index < 1000) sprintf (ordernumber, "%d", index);
    selected = lw_dictionary_is_selected (dictionary);

    gtk_list_store_append (liststore, &iter);
    gtk_list_store_set (liststore, &iter,
        GW_DICTIONARYLIST_COLUMN_IMAGE,        iconname,
        GW_DICTIONARYLIST_COLUMN_POSITION,     ordernumber,
        GW_DICTIONARYLIST_COLUMN_NAME,         shortname,
        GW_DICTIONARYLIST_COLUMN_LONG_NAME,    longname,
        GW_DICTIONARYLIST_COLUMN_ENGINE,       directoryname,
        GW_DICTIONARYLIST_COLUMN_SHORTCUT,     shortcutname,
        GW_DICTIONARYLIST_COLUMN_SELECTED,     selected,
        GW_DICTIONARYLIST_COLUMN_DICT_POINTER, dictionary,
        -1
    );

errored:
    if (longname != NULL) g_free (longname); longname = NULL;
    if (directoryname != NULL) g_free (directoryname); directoryname = NULL;
}


void 
gw_dictionarylist_sync_treestore (GwDictionaryList *dictionarylist)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL);

    //Declarations
    GtkListStore *liststore;
    LwDictionary *dictionary;
    GList *link;

    liststore = gw_dictionarylist_get_liststore (dictionarylist);
    link = lw_dictionarylist_get_list (LW_DICTIONARYLIST (dictionarylist));

    gtk_list_store_clear (liststore);

    while (link != NULL)
    {
      dictionary = LW_DICTIONARY (link->data);
      
      gw_dictionarylist_liststore_append (dictionarylist, dictionary);

      link = link->next;
    }
}


static gint
gw_dictionarylist_save_order_compare_func (gconstpointer a, gconstpointer b, gpointer data)
{
    //Declarations
    GHashTable *hashtable;
    gint a_index;
    gint b_index;

    //Initializations
    hashtable = data;
    a_index = GPOINTER_TO_INT (g_hash_table_lookup (hashtable, a));
    b_index = GPOINTER_TO_INT (g_hash_table_lookup (hashtable, b));
    
    if (a_index < b_index) return -1;
    else if (a_index > b_index) return 1;
    else return 0;
}


void
gw_dictionarylist_save_order (GwDictionaryList *dictionarylist, LwPreferences *preferences)
{
    //Declarations
    GtkListStore *liststore;
    GtkTreeModel *treemodel;
    GtkTreeIter treeiter;
    LwDictionary *dictionary;
    gboolean valid;
    GHashTable *hashtable;
    gint index;

    //Initializations
    liststore = gw_dictionarylist_get_liststore (dictionarylist);
    treemodel = GTK_TREE_MODEL (liststore);
    hashtable = g_hash_table_new (g_direct_hash, g_direct_equal);
    index = 0;

    //Fill the hashtable
    valid = gtk_tree_model_get_iter_first (treemodel, &treeiter);
    while (valid)
    {
      gtk_tree_model_get (treemodel, &treeiter, GW_DICTIONARYLIST_COLUMN_DICT_POINTER, &dictionary, -1);
      g_hash_table_insert (hashtable, dictionary, GINT_TO_POINTER (index));
      valid = gtk_tree_model_iter_next (treemodel, &treeiter);
      index++;
    }

    //Sort the LwDictionaryList and save the order
    lw_dictionarylist_sort_with_data (LW_DICTIONARYLIST (dictionarylist), gw_dictionarylist_save_order_compare_func, hashtable);
    lw_dictionarylist_save_order (LW_DICTIONARYLIST (dictionarylist), preferences);

    g_hash_table_unref (hashtable); hashtable = NULL;
}


GMenuModel*
gw_dictionarylist_get_menumodel (GwDictionaryList *dictionarylist)
{
    return dictionarylist->priv->menumodel;
}


GtkListStore*
gw_dictionarylist_get_liststore (GwDictionaryList *dictionarylist)
{
    return dictionarylist->priv->liststore;
}


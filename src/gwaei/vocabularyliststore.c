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
//! @file vocabularyliststore.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

#include <gwaei/vocabularyliststore-private.h>
#include <gwaei/vocabularyliststore.h>

void gw_vocabularyliststore_wordstore_changed_cb (GwVocabularyWordStore*, gpointer);

G_DEFINE_TYPE (GwVocabularyListStore, gw_vocabularyliststore, GTK_TYPE_LIST_STORE)

typedef enum
{
  PROP_0,
} GwVocabularyListStoreProps;


GtkListStore*
gw_vocabularyliststore_new ()
{
    //Declarations
    GwVocabularyListStore *model;

    //Initializations
    model = GW_VOCABULARYLISTSTORE (g_object_new (GW_TYPE_VOCABULARYLISTSTORE, NULL));

    return GTK_LIST_STORE (model);
}


static void 
gw_vocabularyliststore_init (GwVocabularyListStore *store)
{
    GType types[] = { G_TYPE_STRING, G_TYPE_INT, G_TYPE_OBJECT };
    gtk_list_store_set_column_types (GTK_LIST_STORE (store), 3, types);

    store->priv = GW_VOCABULARYLISTSTORE_GET_PRIVATE (store);
    memset(store->priv, 0, sizeof(GwVocabularyListStorePrivate));

    gw_vocabularyliststore_revert_all (store);
}


static void 
gw_vocabularyliststore_finalize (GObject *object)
{
/*
    GwVocabularyListStore *store;

    store = GW_VOCABULARYLISTSTORE (object);
*/

    G_OBJECT_CLASS (gw_vocabularyliststore_parent_class)->finalize (object);
}


static void
gw_vocabularyliststore_class_init (GwVocabularyListStoreClass *klass)
{
    //Declarations
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = gw_vocabularyliststore_finalize;

    g_type_class_add_private (object_class, sizeof (GwVocabularyListStorePrivate));

    klass->signalid[GW_VOCABULARYLISTSTORE_CLASS_SIGNALID_CHANGED] = g_signal_new (
        "changed",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (GwVocabularyListStoreClass, changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );
}

gchar*
gw_vocabularyliststore_get_name_by_iter (GwVocabularyListStore *store, GtkTreeIter *iter)
{
    g_assert (iter != NULL);

    gchar *name;

    gtk_tree_model_get (GTK_TREE_MODEL (store), iter, GW_VOCABULARYLISTSTORE_COLUMN_NAME, &name, -1);

    return name;
}


GtkListStore*
gw_vocabularyliststore_get_wordstore_by_iter (GwVocabularyListStore *store, GtkTreeIter *iter)
{
    GtkListStore *wordstore;
    GtkTreeModel *model;
    model = GTK_TREE_MODEL (store);
    gtk_tree_model_get (model, iter, GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, &wordstore, -1);
    if (wordstore != NULL)
    {
      gw_vocabularywordstore_load (GW_VOCABULARYWORDSTORE (wordstore), NULL);
      g_object_unref (wordstore);
    }
    return wordstore;
}


GtkListStore*
gw_vocabularyliststore_get_wordstore_by_index (GwVocabularyListStore *store, gint index)
{
    //Declarations
    GtkTreeModel *model;
    GtkListStore *wordstore;
    GtkTreeIter iter;
    gboolean valid;

    //Initializations
    model = GTK_TREE_MODEL (store);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    wordstore = NULL;

    while (valid && index > 0)
    {
      valid = gtk_tree_model_iter_next (model, &iter);
      index--;
    }

    if (valid)
    {
      gw_vocabularyliststore_get_wordstore_by_iter (store, &iter);
    }

    return wordstore;
}


GtkListStore*
gw_vocabularyliststore_get_wordstore_by_name (GwVocabularyListStore *store, const gchar *NAME)
{
    //Declarations
    GtkTreeModel *model;
    GtkListStore *wordstore;
    GtkTreeIter iter;
    gboolean valid;
    gchar *name;

    //Initializations
    model = GTK_TREE_MODEL (store);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    wordstore = NULL;

    while (valid && wordstore == NULL)
    {
      gtk_tree_model_get (model, &iter, GW_VOCABULARYLISTSTORE_COLUMN_NAME, &name, -1);
      if (name != NULL)
      {
        if (strcmp(NAME, name) == 0)
        {
          gtk_tree_model_get (model, &iter, GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, &wordstore, -1);
          g_object_unref (wordstore);
        }
        g_free (name); name = NULL;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    if (wordstore == NULL)
    {
      wordstore = gw_vocabularywordstore_new (NAME);
      gtk_list_store_append (GTK_LIST_STORE (store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (store), &iter,
        GW_VOCABULARYLISTSTORE_COLUMN_NAME,    NAME,
        GW_VOCABULARYLISTSTORE_COLUMN_CHANGED, PANGO_WEIGHT_SEMIBOLD,
        GW_VOCABULARYLISTSTORE_COLUMN_OBJECT,  wordstore,
        -1);
      g_signal_connect (G_OBJECT (wordstore), "changed", G_CALLBACK (gw_vocabularyliststore_wordstore_changed_cb), store);
      gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (wordstore), TRUE);
      g_object_unref (wordstore);
    }

    return wordstore;
}


//!
//! @brief Deletes files that have to coresponding vocabulary list
//!
void
gw_vocabularyliststore_clean_files (GwVocabularyListStore *store)
{
    //Definitions
    GDir *dir;
    gchar *uri;
    gchar *filename;
    const gchar *name;

    if ((uri = lw_util_build_filename (LW_PATH_VOCABULARY, NULL)) != NULL)
    {
      if ((dir = g_dir_open (uri, 0, NULL)) != NULL)
      {
        while ((name = g_dir_read_name (dir)) != NULL)
        {
          if (!gw_vocabularyliststore_list_exists (store, name))
          {
            if ((filename = g_build_filename (uri, name, NULL)) != NULL)
            {
              g_remove (filename);
              g_free (filename); filename = NULL;
            }
          }
        }
        g_dir_close (dir); dir = NULL;
      }
      g_free (uri); uri = NULL;
    }
}


gboolean
gw_vocabularyliststore_list_exists (GwVocabularyListStore *store, const gchar *NAME)
{
    //Declarations
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;
    gchar *name;
    gboolean exists;

    model = GTK_TREE_MODEL (store);
    valid = gtk_tree_model_get_iter_first (model, &iter);
    exists = FALSE;

    while (valid && !exists)
    {
      gtk_tree_model_get (model, &iter, GW_VOCABULARYLISTSTORE_COLUMN_NAME, &name, -1);
      if (name != NULL)
      {
        if (strcmp(NAME, name) == 0) exists = TRUE;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    return exists;
}


void
gw_vocabularyliststore_save_list_order (GwVocabularyListStore *store, LwPreferences *preferences)
{
    //Declarations
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar **atoms;
    gchar *order;
    gint i, length;
    gboolean valid;

    //Initializations
    model = GTK_TREE_MODEL (store);
    length = 0;
    i = 0;

    //Find the length
    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid)
    {
      length++;
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    //Allocate it
    atoms = g_new0 (gchar*, length + 1);

    //Set the atoms
    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid && i < length)
    {
      atoms[i] = gw_vocabularyliststore_get_name_by_iter (store, &iter);
      valid = gtk_tree_model_iter_next (model, &iter);
      i++;
    }

    //Create and save the order
    order = g_strjoinv (";", atoms);

    lw_preferences_set_string_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_LIST_ORDER, order);

    //Cleanup
    g_strfreev (atoms); atoms = NULL;
    g_free (order); order = NULL;
}


void
gw_vocabularyliststore_load_list_order (GwVocabularyListStore *store, LwPreferences *preferences)
{
    //Declarations
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;
    gchar **atoms;
    gchar buffer[500];
    gint i;
    gint old_pos, new_pos, atoms_length, reorder_length;
    gchar *name;
    gint *reorder;
    GHashTable *oldhash, *newhash;
    gboolean found;
    gpointer ptr;

    //Initializations
    model = GTK_TREE_MODEL (store);
    lw_preferences_get_string_by_schema (preferences, buffer, LW_SCHEMA_VOCABULARY, LW_KEY_LIST_ORDER, 500);
    atoms = g_strsplit (buffer, ";", -1);
    if (atoms == NULL) return;
    atoms_length = g_strv_length (atoms);
    reorder_length = gtk_tree_model_iter_n_children (model, NULL);
    reorder = g_new (gint, reorder_length);
    newhash = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
    oldhash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    if (atoms != NULL && reorder != NULL && newhash != NULL && oldhash != NULL)
    {
      for (i = 0; i < reorder_length; i++) reorder[i] = -1;

      //Create the hash table for the new order
      for (new_pos = 0; atoms[new_pos] != NULL; new_pos++)
      {
        g_hash_table_insert (newhash, atoms[new_pos], GINT_TO_POINTER (new_pos));
      }

      //Create the hash table for the old order
      valid = gtk_tree_model_get_iter_first (model, &iter);
      for (old_pos = 0; valid; old_pos++)
      {
        name = gw_vocabularyliststore_get_name_by_iter (store, &iter);
        if (name != NULL)
        {
          g_hash_table_insert (oldhash, name, GINT_TO_POINTER (old_pos));
        }
        valid = gtk_tree_model_iter_next (model, &iter);
      }

      //Create the map between the two
      valid = gtk_tree_model_get_iter_first (model, &iter);
      while (valid)
      {
        name = gw_vocabularyliststore_get_name_by_iter (store, &iter);
        if (name != NULL)
        {
          found = g_hash_table_lookup_extended (oldhash, name, NULL, &ptr);
          if (found)
          {
            old_pos = GPOINTER_TO_INT (ptr);
            found = g_hash_table_lookup_extended (newhash, name, NULL, &ptr);
            if (found)
            {
              new_pos = GPOINTER_TO_INT (ptr);
              reorder[new_pos] = old_pos;
            }
            else if (atoms_length < reorder_length)
            {
              new_pos = atoms_length;
              reorder[new_pos] = old_pos;
            }
          }
          g_free (name);
        }
        valid = gtk_tree_model_iter_next (model, &iter);
      }


      valid = TRUE;
      for (new_pos = 0; new_pos < reorder_length && valid; new_pos++) 
        if (reorder[new_pos] == -1) valid = FALSE;

      if (valid) gtk_list_store_reorder (GTK_LIST_STORE (store), reorder);
    }
    if (atoms != NULL) g_strfreev (atoms);
    if (reorder != NULL) g_free (reorder);
    if (oldhash != NULL) g_hash_table_destroy (oldhash);
    if (newhash != NULL) g_hash_table_destroy (newhash);
}


void
gw_vocabularyliststore_save_all (GwVocabularyListStore *store)
{
    //Declarations
    GwVocabularyListStorePrivate *priv;
    GtkTreeModel *model;
    GtkListStore *wordstore;
    GtkTreeIter iter;
    gboolean valid;


    gw_vocabularyliststore_clean_files (store);
    priv = store->priv;
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
    model = GTK_TREE_MODEL (store);

    while (valid)
    {
      gtk_tree_model_get (model, &iter, GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, &wordstore, -1);
      if (wordstore != NULL)
      {
        gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (wordstore), NULL);
        gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (wordstore), FALSE);
        g_object_unref (wordstore); wordstore = NULL;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    priv->has_removed_lists = FALSE;
    gw_vocabularyliststore_set_has_changes (store, FALSE);
}


void
gw_vocabularyliststore_revert_all (GwVocabularyListStore* store)
{
    //Declarations
    GwVocabularyListStorePrivate *priv;
    GtkTreeIter iter;
    gchar **lists;
    int i;
    GtkListStore *liststore;
    GtkListStore *wordstore;

    //Initializations
    priv = store->priv;
    liststore = GTK_LIST_STORE (store);

    gtk_list_store_clear (liststore);

    if ((lists = lw_vocabularylist_get_lists ()) != NULL)
    {
      for (i = 0; lists[i] != NULL; i++)
      {
        wordstore = gw_vocabularywordstore_new (lists[i]);
        gtk_list_store_append (liststore, &iter);
        gtk_list_store_set (liststore, &iter, 
          GW_VOCABULARYLISTSTORE_COLUMN_NAME, lists[i],
          GW_VOCABULARYLISTSTORE_COLUMN_CHANGED, PANGO_WEIGHT_NORMAL,
          GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, wordstore,
          -1);
        gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (wordstore), FALSE);
        g_signal_connect (G_OBJECT (wordstore), "changed", G_CALLBACK (gw_vocabularyliststore_wordstore_changed_cb), store);
        g_object_unref (wordstore);
      }
      g_strfreev (lists); lists = NULL;
    }

    priv->has_removed_lists = FALSE;
    gw_vocabularyliststore_set_has_changes (store, FALSE);
}


gchar*
gw_vocabularyliststore_build_unique_name (GwVocabularyListStore *store)
{
    GwVocabularyListStorePrivate *priv;
    gchar *name;

    priv = store->priv;
    name = g_strdup_printf (gettext("New List %d"), ++priv->list_new_index);

    while (gw_vocabularyliststore_list_exists (store, name))
    {
      g_free (name);
      name = g_strdup_printf (gettext("New List %d"), ++priv->list_new_index);
    }

    return name;
}


void
gw_vocabularyliststore_new_list (GwVocabularyListStore *store, GtkTreeIter *iter)
{
    //Declarations
    gchar *name;
    GtkListStore *wordstore;

    //Initializations
    name = gw_vocabularyliststore_build_unique_name (store);
    wordstore = gw_vocabularywordstore_new (name);
    
    gtk_list_store_append (GTK_LIST_STORE (store), iter);
    gtk_list_store_set (GTK_LIST_STORE (store), iter, 
        GW_VOCABULARYLISTSTORE_COLUMN_NAME, name, 
        GW_VOCABULARYLISTSTORE_COLUMN_CHANGED, PANGO_WEIGHT_SEMIBOLD, 
        GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, wordstore,
    -1);

    g_signal_connect (G_OBJECT (wordstore), "changed", G_CALLBACK (gw_vocabularyliststore_wordstore_changed_cb), store);
    gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (wordstore), TRUE);
    g_object_unref (wordstore);

    g_free (name);
}


void
gw_vocabularyliststore_remove_path_list (GwVocabularyListStore *store, GList *list)
{
    //Declarations
    GwVocabularyListStorePrivate *priv;
    GList *link;
    GtkTreeIter iter;
    GtkTreePath *path;
    gboolean modified;

    //Initializations
    priv = store->priv;
    modified = (list != NULL);

    //Convert the tree paths to row references
    for (link = g_list_last (list); link != NULL; link = link->prev)
    {
      path = (GtkTreePath*) link->data;
      gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);
      gtk_list_store_remove (GTK_LIST_STORE (store), &iter);
    }

    if (modified)
    {
      priv->has_removed_lists = TRUE;
      gw_vocabularyliststore_set_has_changes (store, TRUE);
    }
}


G_MODULE_EXPORT void
gw_vocabularyliststore_wordstore_changed_cb (GwVocabularyWordStore *wordstore, gpointer data)
{
    //Declarations
    GwVocabularyListStore *liststore;
    GwVocabularyWordStore *temp_wordstore;
    GtkTreeModel *listmodel;
    GtkTreeIter iter;
    PangoWeight weight;
    gboolean valid;
    gboolean word_has_changes, list_has_changes;

    //Initializations
    liststore = GW_VOCABULARYLISTSTORE (data);
    temp_wordstore = NULL;
    listmodel = GTK_TREE_MODEL (liststore);
    word_has_changes = gw_vocabularywordstore_has_changes (wordstore);
    if (word_has_changes) weight = PANGO_WEIGHT_SEMIBOLD;
    else weight = PANGO_WEIGHT_NORMAL;
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (liststore), &iter);
    list_has_changes = FALSE;

    while (valid)
    {
        gtk_tree_model_get (listmodel, &iter, GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, &temp_wordstore, -1);
        if (temp_wordstore != NULL)
        {
          if (gw_vocabularywordstore_has_changes (temp_wordstore)) list_has_changes = TRUE;
          if (temp_wordstore == wordstore)
          {
            gtk_list_store_set (GTK_LIST_STORE (liststore), &iter, GW_VOCABULARYLISTSTORE_COLUMN_CHANGED, weight, -1);
          }
          g_object_unref (temp_wordstore); temp_wordstore = NULL;
        }
        valid = gtk_tree_model_iter_next (listmodel, &iter);
    }

    gw_vocabularyliststore_set_has_changes (liststore, list_has_changes);
}


void
gw_vocabularyliststore_set_has_changes (GwVocabularyListStore *store, gboolean has_changes)
{
    GwVocabularyListStoreClass *klass;

    klass = GW_VOCABULARYLISTSTORE_CLASS (G_OBJECT_GET_CLASS (store));
    store->priv->has_changes = has_changes;
    g_signal_emit (G_OBJECT (store), klass->signalid[GW_VOCABULARYLISTSTORE_CLASS_SIGNALID_CHANGED], 0);
}


gboolean
gw_vocabularyliststore_has_changes (GwVocabularyListStore *store)
{
    return (store->priv->has_changes || store->priv->has_removed_lists); 
}



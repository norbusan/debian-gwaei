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
//! @file dictionaryinstallwindow.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/dictionaryinstallwindow-private.h>

G_DEFINE_TYPE (GwDictionaryInstallWindow, gw_dictionaryinstallwindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_dictionaryinstallwindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (g_object_new (GW_TYPE_DICTIONARYINSTALLWINDOW,
                                                       "type",        GTK_WINDOW_TOPLEVEL,
                                                       "application", GW_APPLICATION (application),
                                                       "ui-xml",      "dictionaryinstallwindow.ui",
                                                        NULL));

    return GTK_WINDOW (window);
}

static void 
gw_dictionaryinstallwindow_init (GwDictionaryInstallWindow *window)
{
    window->priv = GW_DICTIONARYINSTALLWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwDictionaryInstallWindowPrivate));
}


static void 
gw_dictionaryinstallwindow_finalize (GObject *object)
{
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;

    window = GW_DICTIONARYINSTALLWINDOW (object);
    priv = window->priv;

    if (priv->encoding_store != NULL) 
      g_object_unref (priv->encoding_store); priv->encoding_store = NULL;
    if (priv->compression_store != NULL)
      g_object_unref (priv->compression_store); priv->compression_store = NULL;
    if (priv->engine_store != NULL) 
      g_object_unref (priv->engine_store); priv->engine_store = NULL;
    if (priv->dictionary_store != NULL) 
      g_object_unref (priv->dictionary_store); priv->dictionary_store = NULL;

    G_OBJECT_CLASS (gw_dictionaryinstallwindow_parent_class)->finalize (object);
}


static void 
gw_dictionaryinstallwindow_constructed (GObject *object)
{
    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GwApplication *application;
    LwDictInstList *dictinstlist;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GList *iter;
    LwDictInst *di;
    GtkTreeIter treeiter;
    int i;
    GtkAccelGroup *accelgroup;
    GtkWidget *widget;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_dictionaryinstallwindow_parent_class)->constructed (object);
    }

    window = GW_DICTIONARYINSTALLWINDOW (object);
    priv = window->priv;
    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));
    application = gw_window_get_application (GW_WINDOW (window));
    dictinstlist = gw_application_get_dictinstlist (application);

    gtk_window_set_title (GTK_WINDOW (window), gettext("Select Dictionaries..."));
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal (GTK_WINDOW (window), TRUE);
    gtk_window_set_default_size (GTK_WINDOW (window), 200, 300);
    gtk_window_set_has_resize_grip (GTK_WINDOW (window), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (window), 4);

    //Initializations
    priv->dictionary_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_DICTSTOREFIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
    priv->view = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "dictionary_install_treeview"));
    priv->add_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "dictionary_install_add_button"));
    priv->details_togglebutton = GTK_TOGGLE_BUTTON (gw_window_get_object (GW_WINDOW (window), "show_dictionary_detail_checkbutton"));

    //Set up the dictionary liststore
    for (iter = dictinstlist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      gtk_list_store_append (GTK_LIST_STORE (priv->dictionary_store), &treeiter);
      gtk_list_store_set (
        priv->dictionary_store, &treeiter,
        GW_DICTINSTWINDOW_DICTSTOREFIELD_SHORT_NAME, di->shortname,
        GW_DICTINSTWINDOW_DICTSTOREFIELD_LONG_NAME, di->longname,
        GW_DICTINSTWINDOW_DICTSTOREFIELD_DICTINST_PTR, di,
        GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, FALSE, 
        -1
      );
    }

    //Set up the Engine liststore
    priv->engine_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_ENGINESTOREFIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < TOTAL_LW_DICTTYPES; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (priv->engine_store), &treeiter);
      gtk_list_store_set (
        priv->engine_store, &treeiter,
        GW_DICTINSTWINDOW_ENGINESTOREFIELD_ID, i,
        GW_DICTINSTWINDOW_ENGINESTOREFIELD_NAME, lw_util_dicttype_to_string (i),
        -1
      );
    }

    //Set up the Compression liststore
    priv->compression_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < LW_COMPRESSION_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (priv->compression_store), &treeiter);
      gtk_list_store_set (
        priv->compression_store, &treeiter,
        GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELD_ID, i,
        GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELD_NAME, lw_util_get_compression_name (i),
        -1
      );
    }

    //Set up the Encoding liststore
    priv->encoding_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_ENCODINGSTOREFIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < LW_ENCODING_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (priv->encoding_store), &treeiter);
      gtk_list_store_set (
        priv->encoding_store, &treeiter,
        GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_ID, i,
        GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_NAME, lw_util_get_encoding_name(i),
        -1
      );
    }

    //Setup the dictionary list view
    gtk_tree_view_set_model (priv->view, GTK_TREE_MODEL (priv->dictionary_store));

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 5);
    column = gtk_tree_view_column_new_with_attributes (" ", renderer, "active", GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, NULL);
    gtk_tree_view_append_column (priv->view, column);
    g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (gw_dictionaryinstallwindow_listitem_toggled_cb), window);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 0);
    column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", GW_DICTINSTWINDOW_DICTSTOREFIELD_LONG_NAME, NULL);
    gtk_tree_view_append_column (priv->view, column);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "dictionary_install_cancel_button"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Escape), 0, GTK_ACCEL_VISIBLE);
}


static void
gw_dictionaryinstallwindow_class_init (GwDictionaryInstallWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_dictionaryinstallwindow_constructed;
  object_class->finalize = gw_dictionaryinstallwindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwDictionaryInstallWindowPrivate));
}


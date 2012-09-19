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

#include <gwaei/gettext.h>
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

    G_OBJECT_CLASS (gw_dictionaryinstallwindow_parent_class)->finalize (object);
}


static GtkListStore*
gw_dictionaryinstallwindow_enginestore_new (GwDictionaryInstallWindow *window)
{
    //Declarations
    GtkListStore *liststore;
    GtkTreeIter treeiter;
    GType *types;
    GType type;
    gint i;
    gchar *directoryname;

    types = g_type_children (LW_TYPE_DICTIONARY, NULL);
    liststore = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_ENGINESTOREFIELDS, G_TYPE_INT, G_TYPE_STRING);

    if (types != NULL)
    {
      for (i = 0; types[i] != 0; i++)
      {
        type = types[i];
        directoryname = lw_dictionary_get_directoryname (type);
        if (directoryname != NULL)
        {
          gtk_list_store_append (liststore, &treeiter);
          gtk_list_store_set (liststore, &treeiter,
            GW_DICTINSTWINDOW_ENGINESTOREFIELD_ID, type,
            GW_DICTINSTWINDOW_ENGINESTOREFIELD_NAME, directoryname,
            -1
          );
          g_free (directoryname); directoryname = NULL;
        }
      }
      g_free (types); types = NULL;
    }
    
    return liststore;
}


static GtkListStore*
gw_dictionaryinstallwindow_encodingstore_new (GwDictionaryInstallWindow *window)
{
    GtkListStore *liststore;
    GtkTreeIter treeiter;
    gint i;

    //Set up the Encoding liststore
    liststore = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_ENCODINGSTOREFIELDS, G_TYPE_INT, G_TYPE_STRING);

    for (i = 0; i < LW_ENCODING_TOTAL; i++)
    {
      gtk_list_store_append (liststore, &treeiter);
      gtk_list_store_set (liststore, &treeiter,
        GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_ID, i,
        GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_NAME, lw_util_get_encodingname(i),
        -1
      );
    }

    return liststore;
}


static void 
gw_dictionaryinstallwindow_constructed (GObject *object)
{
    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GwApplication *application;
    GwDictionaryList *dictionarylist;
    GtkTreeModel *treemodel;
    GtkListStore *liststore;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkAccelGroup *accelgroup;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_dictionaryinstallwindow_parent_class)->constructed (object);
    }

    window = GW_DICTIONARYINSTALLWINDOW (object);
    priv = window->priv;
    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = gw_application_get_installable_dictionarylist (application);
    liststore = gw_dictionarylist_get_liststore (dictionarylist);
    treemodel = GTK_TREE_MODEL (liststore);

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
    priv->view = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "treeview"));
    priv->add_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "add_button"));
    priv->cancel_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "cancel_button"));
    priv->details_togglebutton = GTK_TOGGLE_BUTTON (gw_window_get_object (GW_WINDOW (window), "show_dictionary_detail_checkbutton"));
    priv->details_hbox = GTK_BOX (gw_window_get_object (GW_WINDOW (window), "details_hbox"));

    priv->engine_store = gw_dictionaryinstallwindow_enginestore_new (window);
    priv->encoding_store = gw_dictionaryinstallwindow_encodingstore_new (window);

    //Setup the dictionary list view
    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 5);
    column = gtk_tree_view_column_new_with_attributes (" ", renderer, "active", GW_DICTIONARYLIST_COLUMN_SELECTED, NULL);
    gtk_tree_view_append_column (priv->view, column);
    g_signal_connect_swapped (G_OBJECT (renderer), "toggled", 
                              G_CALLBACK (gw_dictionaryinstallwindow_listitem_toggled_cb), liststore);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 0);
    column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", GW_DICTIONARYLIST_COLUMN_LONG_NAME, NULL);
    gtk_tree_view_append_column (priv->view, column);

    gtk_tree_view_set_model (priv->view, treemodel);

    gtk_widget_add_accelerator (GTK_WIDGET (priv->cancel_button), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (priv->cancel_button), "activate", 
      accelgroup, (GDK_KEY_Escape), 0, GTK_ACCEL_VISIBLE);

    gw_window_unload_xml (GW_WINDOW (window));
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


//!
//! @brief Checks the validity of the LwDictionary data and sets the add button sensitivity accordingly
//!
void 
gw_dictionaryinstallwindow_sync_interface (GwDictionaryInstallWindow *window)
{
    //Declarations
    GwDictionaryInstallWindowPrivate *priv;
    GwApplication *application;
    GwDictionaryList *dictionarylist;
    gboolean sensitivity;

    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = gw_application_get_installable_dictionarylist (application);
    sensitivity = lw_dictionarylist_installer_is_valid (LW_DICTIONARYLIST (dictionarylist));

    //Finalize
    gtk_widget_set_sensitive (GTK_WIDGET (priv->add_button), sensitivity);
}


void 
gw_dictionaryinstallwindow_clear_details_box (GwDictionaryInstallWindow *window)
{
    //Declarations
    GwDictionaryInstallWindowPrivate *priv;
    GtkWidget *hbox;
    GList *children;
    GList *link;

    //Initializations
    priv = window->priv;
    hbox = GTK_WIDGET (priv->details_hbox);
    children = gtk_container_get_children (GTK_CONTAINER (hbox));

    //Set the volatile widget pointers to null
    priv->filename_entry = NULL;
    priv->engine_combobox = NULL;
    priv->source_entry = NULL;
    priv->source_choose_button = NULL;
    priv->source_reset_button = NULL;
    priv->encoding_combobox = NULL;
    priv->postprocess_checkbutton = NULL;

    //Clear the GUI elements
    if (children != NULL)
    {
      for (link = children; link != NULL; link = link->next)
      {
        gtk_widget_destroy (GTK_WIDGET (link->data));
      }
      g_list_free (children); children = NULL;
    }

    priv->dictionary = NULL;
}

static gint
gw_dictionaryinstallwindow_gtype_to_index (GwDictionaryInstallWindow *window, LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (window != NULL, 0);
    g_return_val_if_fail (dictionary != NULL, 0);

    //Declarations
    GwDictionaryInstallWindowPrivate *priv;
    GType type, treetype;
    GtkTreeIter treeiter;
    GtkTreeModel *treemodel;
    gboolean valid;
    gint index;

    //Initializaitons
    priv = window->priv;
    type = G_OBJECT_TYPE (dictionary);
    treemodel = GTK_TREE_MODEL (priv->engine_store);
    valid = gtk_tree_model_get_iter_first (treemodel, &treeiter);
    index = 0;

    while (valid)
    {
      gtk_tree_model_get (treemodel, &treeiter, GW_DICTINSTWINDOW_ENGINESTOREFIELD_ID, &treetype, -1);

      if (type == treetype) break;

      valid = gtk_tree_model_iter_next (treemodel, &treeiter);
      index++;
    }
    
    return index;
}

void 
gw_dictionaryinstallwindow_fill_details_box (GwDictionaryInstallWindow *window, LwDictionary *dictionary)
{
    //Declarations
    GwDictionaryInstallWindowPrivate *priv;
    GtkWidget *parent;
    GtkWidget *button;
    GtkWidget *image;
    GtkWidget *entry;
    GtkWidget *label;
    GtkWidget *grid;
    GtkWidget *hbox;
    GtkWidget *combobox;
    GtkWidget *checkbox;
    GtkCellRenderer *renderer;
    gchar *markup;
    gboolean editable;

    //Initializations
    priv = window->priv;
    priv->dictionary = dictionary;
    parent = GTK_WIDGET (priv->details_hbox);
    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 1);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 0);
    editable = !(lw_dictionary_installer_is_builtin (dictionary));

    //First row
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    markup = g_strdup_printf(gettext("<b>%s Dictionary Install Details</b>"), lw_dictionary_get_name (dictionary));
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), markup);
    if (markup != NULL) g_free (markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (hbox), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Second row
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    markup = g_strdup_printf("%s", lw_dictionary_installer_get_description (dictionary));
    label = gtk_label_new (NULL);
    gtk_widget_set_size_request (GTK_WIDGET (label), 300, -1);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_markup(GTK_LABEL (label), markup);
    if (markup != NULL) g_free (markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (hbox), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Third row
    label = gtk_label_new (gettext("Filename: "));
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 0, 1, 1);

    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), lw_dictionary_installer_get_files (dictionary));
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstallwindow_filename_entry_changed_cb), window);
    gtk_grid_attach (GTK_GRID (grid), entry, 1, 0, 2, 1);
    gtk_widget_set_hexpand (GTK_WIDGET (entry), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (entry), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);
    priv->filename_entry = GTK_ENTRY (entry);

    //Forth row
    label = gtk_label_new (gettext("Engine: "));
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 1, 1, 1);

    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", GW_DICTINSTWINDOW_ENGINESTOREFIELD_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (priv->engine_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), gw_dictionaryinstallwindow_gtype_to_index (window, dictionary));

    gtk_grid_attach (GTK_GRID (grid), combobox, 1, 1, 2, 1);
    gtk_widget_set_hexpand (GTK_WIDGET (combobox), TRUE);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstallwindow_engine_combobox_changed_cb), window);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);
    priv->engine_combobox = GTK_COMBO_BOX (combobox);


    //Fifth row
    label = gtk_label_new (gettext("Source: "));
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 2, 1, 1);

    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));

    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), lw_dictionary_installer_get_downloads (dictionary));
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstallwindow_source_entry_changed_cb), window);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (entry), TRUE, TRUE, 0);
    priv->source_entry = GTK_ENTRY (entry);

    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_dictionaryinstallwindow_select_file_cb), window);
    priv->source_choose_button = GTK_BUTTON (button);

    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    gtk_widget_set_sensitive (GTK_WIDGET (button), !editable);
    gtk_grid_attach (GTK_GRID (grid), hbox, 1, 2, 2, 1);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_dictionaryinstallwindow_reset_default_uri_cb), window);
    priv->source_reset_button = GTK_BUTTON (button);

    //Sixth row
    label = gtk_label_new (gettext("Encoding: "));
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 3, 1, 1);

    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (priv->encoding_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), lw_dictionary_installer_get_encoding (dictionary));
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstallwindow_encoding_combobox_changed_cb), window);
    priv->encoding_combobox = GTK_COMBO_BOX (combobox);

    gtk_grid_attach (GTK_GRID (grid), combobox, 1, 3, 2, 1);
    gtk_widget_set_hexpand (GTK_WIDGET (combobox), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Ninth row
    checkbox = gtk_check_button_new_with_label (gettext("Do Extra Postprocessing"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), lw_dictionary_installer_get_postprocessing (dictionary));
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstallwindow_postprocess_checkbox_toggled_cb), window);
    priv->postprocess_checkbutton = GTK_CHECK_BUTTON (checkbox);

    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 6, 3, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (grid), TRUE, TRUE, 5);
    gtk_widget_show_all (GTK_WIDGET (grid));
}

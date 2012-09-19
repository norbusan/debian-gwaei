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
//! @file dictionaryinstallwindow-callbacks.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/dictionaryinstallwindow-private.h>


static void _dictinstwindow_clear_details_box (GwDictionaryInstallWindow*);
static void _dictinstwindow_fill_details_box (GwDictionaryInstallWindow*, LwDictInst*);
static void _dictinstwindow_update_add_button_sensitivity (GwDictionaryInstallWindow*);

G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_filename_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    const char *value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_entry_get_text (GTK_ENTRY (widget));

    lw_dictinst_set_filename (priv->di, value);

    _dictinstwindow_update_add_button_sensitivity (window);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_engine_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    int value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    priv->di->type = value;
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_source_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    const char *value = gtk_entry_get_text (GTK_ENTRY (widget));

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    value = gtk_entry_get_text (GTK_ENTRY (widget));

    //Set the LwDictInst value
    lw_dictinst_set_download_source (priv->di, value);

    //Update the preference if approprate
    if (priv->di->schema != NULL && priv->di->key != NULL)
    {
      lw_preferences_set_string_by_schema (preferences, priv->di->schema, priv->di->key, value);
    }

    _dictinstwindow_update_add_button_sensitivity (window);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_reset_default_uri_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    char value[200];

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    if (priv->di->schema == NULL || priv->di->key == NULL) return;

    lw_preferences_reset_value_by_schema (preferences, priv->di->schema, priv->di->key);
    lw_preferences_get_string_by_schema (preferences, value, priv->di->schema, priv->di->key, 200);
    gtk_entry_set_text (priv->source_entry, value);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_select_file_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GtkWidget *dialog;
    char *filename;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    dialog = gtk_file_chooser_dialog_new (
      "Select File",
      GTK_WINDOW (window),
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL
    );
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      if (filename != NULL)
      {
        gtk_entry_set_text (priv->source_entry, filename);
        g_free (filename);
      }
    }

    gtk_widget_destroy (dialog);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_encoding_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declaraitons
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    int value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictinst_set_encoding (priv->di, value);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_compression_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    int value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (data);
    priv = window->priv;
    value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictinst_set_compression (priv->di, value);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_split_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    gboolean value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictinst_set_split (priv->di, value);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_merge_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    gboolean value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictinst_set_merge (priv->di, value);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_cursor_changed_cb (GtkTreeView *view, gpointer data)
{
    //Sanity check
    g_return_if_fail (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkWidget *hbox;
    LwDictInst *di;
    GtkTreeIter iter;
    gboolean show_details;
    gboolean has_selection;
    GtkTreeModel *model;
    gboolean editable;
    int height;
    int width;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    hbox = GTK_WIDGET (priv->details_hbox);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
    model = GTK_TREE_MODEL (priv->dictionary_store);
    has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
    show_details = gtk_toggle_button_get_active (priv->details_togglebutton);
    editable = FALSE;

    //Set the approprate contents of the detail pane
    if (has_selection)
    {
      _dictinstwindow_clear_details_box (window);
      gtk_tree_model_get (model, &iter, GW_DICTINSTWINDOW_DICTSTOREFIELD_DICTINST_PTR, &di, -1);
      _dictinstwindow_fill_details_box (window, di);
      editable = !di->builtin;
    }

    //Set the approprate show/hide state of the detail pane
    if (has_selection && (show_details || editable))
      gtk_widget_show (hbox);
    else
      gtk_widget_hide (hbox);

    //Make the window shrink if the detail pane disappeared
    gtk_window_get_size (GTK_WINDOW (window), &width, &height);
    gtk_window_resize (GTK_WINDOW (window), 1, height);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_listitem_toggled_cb (GtkCellRendererToggle *renderer, 
                                                                     gchar *path,
                                                                     gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GtkTreeIter iter;
    gboolean state;
    LwDictInst *di;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (priv->dictionary_store), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (priv->dictionary_store), &iter, GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, &state, GW_DICTINSTWINDOW_DICTSTOREFIELD_DICTINST_PTR, &di, -1);
    gtk_list_store_set (GTK_LIST_STORE (priv->dictionary_store), &iter, GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, !state, -1);
    di->selected = !state;

    _dictinstwindow_update_add_button_sensitivity (window);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_detail_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GtkTreePath *path;
    GtkTreeViewColumn *column;

    //Declarations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    //Trigger the list item selection callback
    gtk_tree_view_get_cursor (priv->view, &path, &column);
    if (path == NULL) return;
    gtk_tree_view_set_cursor (priv->view, path, column, FALSE);

    //Cleanup
    gtk_tree_path_free (path);
}


//!
//! @brief Checks the validity of the LwDictInst data and sets the add button sensitivity accordingly
//!
static void _dictinstwindow_update_add_button_sensitivity (GwDictionaryInstallWindow *window)
{
    //Declarations
    GwDictionaryInstallWindowPrivate *priv;
    GwApplication *application;
    LwDictInstList *dictinstlist;
    gboolean sensitivity;

    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictinstlist = gw_application_get_dictinstlist (application);
    sensitivity = lw_dictinstlist_data_is_valid (dictinstlist);

    //Finalize
    gtk_widget_set_sensitive (GTK_WIDGET (priv->add_button), sensitivity);
}


static void _dictinstwindow_clear_details_box (GwDictionaryInstallWindow *window)
{
    //Declarations
    GwDictionaryInstallWindowPrivate *priv;
    GtkWidget *hbox;
    GList *children;
    GList *iter;

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
    priv->compression_combobox = NULL;
    priv->split_checkbutton = NULL;
    priv->merge_checkbutton = NULL;

    //Clear the GUI elements
    for (iter = children; iter != NULL; iter = iter->next)
    {
      gtk_widget_destroy (GTK_WIDGET (iter->data));
    }
    g_list_free (children);
    children = NULL;

    priv->di = NULL;
}


static void _dictinstwindow_fill_details_box (GwDictionaryInstallWindow *window, LwDictInst *di)
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
    parent = GTK_WIDGET (priv->details_hbox);
    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 1);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 0);
    editable = !di->builtin;
    priv->di = di;

    //First row
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    markup = g_strdup_printf(gettext("<b>%s Install Details</b>"), di->longname);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), markup);
    if (markup != NULL) g_free (markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (hbox), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Second row
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    markup = g_strdup_printf("%s", di->description);
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
    gtk_entry_set_text (GTK_ENTRY (entry), di->filename);
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstallwindow_filename_entry_changed_cb), window);
    gtk_grid_attach (GTK_GRID (grid), entry, 1, 0, 2, 1);
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

    gtk_grid_attach (GTK_GRID (grid), combobox, 1, 1, 2, 1);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->type);
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
    gtk_entry_set_text (GTK_ENTRY (entry), di->uri[LW_DICTINST_NEEDS_DOWNLOADING]);
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
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->encoding);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstallwindow_encoding_combobox_changed_cb), window);
    priv->encoding_combobox = GTK_COMBO_BOX (combobox);

    gtk_grid_attach (GTK_GRID (grid), combobox, 1, 3, 2, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Seventh row
    label = gtk_label_new (gettext("Compression: "));
    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 4, 1, 1);

    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELD_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (priv->compression_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->compression);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstallwindow_compression_combobox_changed_cb), window);
    priv->compression_combobox = GTK_COMBO_BOX (combobox);

    gtk_grid_attach (GTK_GRID (grid), combobox, 1, 4, 2, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Eighth row
    checkbox = gtk_check_button_new_with_label (gettext("Split Places from Names Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->split);
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstallwindow_split_checkbox_toggled_cb), window);
    priv->split_checkbutton = GTK_CHECK_BUTTON (checkbox);

    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 5, 3, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Ninth row
    checkbox = gtk_check_button_new_with_label (gettext("Merge Radicals into Kanji Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->merge);
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstallwindow_merge_checkbox_toggled_cb), window);
    priv->merge_checkbutton = GTK_CHECK_BUTTON (checkbox);

    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_grid_attach (GTK_GRID (grid), hbox, 0, 6, 3, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (grid), FALSE, FALSE, 5);
    gtk_widget_show_all (GTK_WIDGET (grid));
}


//!
//! @brief Closes the window passed throught the widget pointer
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwDictionaryInstallWindow *window;
    
    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);

    gtk_widget_destroy (GTK_WIDGET (window));
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_add_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwDictionaryInstallWindow *window;
    GwSettingsWindow *settingswindow;
    GwInstallProgressWindow *installprogresswindow;
    GwApplication *application;
    
    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    settingswindow = GW_SETTINGSWINDOW (gtk_window_get_transient_for (GTK_WINDOW (window)));
    application = gw_window_get_application (GW_WINDOW (window));
    installprogresswindow = GW_INSTALLPROGRESSWINDOW (gw_installprogresswindow_new (GTK_APPLICATION (application)));

    gtk_widget_destroy (GTK_WIDGET (window));

    gtk_window_set_transient_for (GTK_WINDOW (installprogresswindow), GTK_WINDOW (settingswindow));
    gtk_widget_show (GTK_WIDGET (installprogresswindow));
    gw_installprogresswindow_start (installprogresswindow);
}


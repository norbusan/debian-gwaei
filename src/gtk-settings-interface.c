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
//! @file src/gtk-settings-interface.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! widgets.
//!


#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>

#include <gwaei/engine.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-settings-callbacks.h>
#include <gwaei/gtk-settings-interface.h>
#include <gwaei/gtk-settings-interface-install-line.h>
#include <gwaei/preferences.h>


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
void gw_ui_update_settings_interface ()
{
    //Set the install interface
    GtkWidget *close_button;
    close_button = GTK_WIDGET (gtk_builder_get_object (builder, "settings_close_button"));

    GtkWidget *advanced_tab;
    advanced_tab = GTK_WIDGET (gtk_builder_get_object (builder, "advanced_tab"));

    GtkWidget *organize_dictionaries_tab;
    organize_dictionaries_tab = GTK_WIDGET (gtk_builder_get_object (builder, "organize_dictionaries_tab"));

    if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_UPDATING  ) > 0 ||
        gw_dictlist_get_total_with_status (GW_DICT_STATUS_INSTALLING) > 0 ||
        gw_dictlist_get_total_with_status (GW_DICT_STATUS_REBUILDING) > 0   )
    {
      gtk_widget_set_sensitive (close_button,   FALSE);
      gtk_widget_set_sensitive (advanced_tab,  FALSE);
      gtk_widget_set_sensitive (organize_dictionaries_tab,  FALSE);
    }
    else
    {
      gtk_widget_set_sensitive (close_button,   TRUE );
      gtk_widget_set_sensitive (advanced_tab,  TRUE);
      gtk_widget_set_sensitive (organize_dictionaries_tab,  TRUE);
    }


    GtkWidget *message;
    message = GTK_WIDGET (gtk_builder_get_object (builder, "please_install_dictionary_hbox"));
    if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_INSTALLED  ) > 0)
    {
      gtk_widget_hide (message);
    }
    else
    {
      gtk_widget_show (message);
    }

    GtkWidget *move_dictionary_up;
    move_dictionary_up = GTK_WIDGET (gtk_builder_get_object (builder, "move_dictionary_up_button"));
    GtkWidget *move_dictionary_down;
    move_dictionary_down = GTK_WIDGET (gtk_builder_get_object (builder, "move_dictionary_down_button"));
    GtkWidget *reset_order_button;
    reset_order_button = GTK_WIDGET (gtk_builder_get_object (builder, "reset_dictionary_order_button"));
    GtkListStore *liststore;
    liststore = GTK_LIST_STORE (gtk_builder_get_object (builder, "list_store_dictionaries"));
    GtkWidget *treeview;
    treeview = GTK_WIDGET (gtk_builder_get_object (builder, "organize_dictionaries_treeview"));
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
    GtkTreeSelection * selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
   
    GtkTreeIter selection_iter;
    if (gtk_tree_selection_get_selected (selection, &model, &selection_iter))
    {
      GtkTreePath *selection_path;
      selection_path = gtk_tree_model_get_path (model, &selection_iter);

      GtkTreeIter comparison_iter;
      gtk_tree_model_get_iter_first (model, &comparison_iter);
      GtkTreePath *comparison_path;
      comparison_path = gtk_tree_model_get_path (model, &comparison_iter);

      gtk_widget_set_sensitive (move_dictionary_up, gtk_tree_path_compare (selection_path, comparison_path) != 0);

      gtk_tree_path_free (comparison_path);
      comparison_path = NULL;
      GtkTreeIter previous_iter;
      while (gtk_tree_model_iter_next (model, &comparison_iter)) previous_iter = comparison_iter;
      comparison_iter = previous_iter;
      comparison_path = gtk_tree_model_get_path (model, &comparison_iter);

      gtk_widget_set_sensitive (move_dictionary_down, gtk_tree_path_compare (selection_path, comparison_path) != 0);

      gtk_tree_path_free (comparison_path);
      comparison_path = NULL;
      gtk_tree_path_free (selection_path);
      selection_path = NULL;
    }
    else
    {
      gtk_widget_set_sensitive (move_dictionary_up, FALSE);
      gtk_widget_set_sensitive (move_dictionary_down, FALSE);
    }

    char current_order[5000];
    char default_order[5000];
    gw_pref_get_string (current_order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    gw_pref_get_default_string (default_order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    gtk_widget_set_sensitive (reset_order_button, strcmp (default_order, current_order) != 0);
}


//!
//! @brief Sets the text in the source gtkentry for the appropriate dictionary
//!
//! @param widget Pointer to a GtkEntry to set the text of
//! @param value The constant string to use as the source for the text
//!
void gw_ui_set_dictionary_source (GtkWidget *widget, const char* value)
{
    if (widget != NULL && value != NULL)
    {
      g_signal_handlers_block_by_func (GTK_WIDGET (widget), do_source_entry_changed_action, NULL);
      gtk_entry_set_text (GTK_ENTRY (widget), value);
      g_signal_handlers_unblock_by_func (GTK_WIDGET (widget), do_source_entry_changed_action, NULL);
    }
}


//!
//! @brief A progressbar update function made specifially to be used with curl when downloading
//!
//! @param message
//! @param percent
//! @param data
//!
int gw_ui_update_progressbar (char *message, int percent, gpointer data)
{
    gdk_threads_enter();

    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    GtkWidget *progressbar = il->status_progressbar;

    if (il->di->status != GW_DICT_STATUS_CANCELING)
    {
      if (percent < 0)
      {
        gw_ui_dict_install_set_message (il, NULL, message);
      }
      else if (percent == 0)
      {
      }
      else
      {
        gdouble ratio = ((gdouble) percent) / 100.0;
        gw_ui_progressbar_set_fraction_by_install_line (il, ratio);
      }
      gdk_threads_leave ();
      return FALSE;
    }

    gdk_threads_leave ();
    return TRUE;
}


//!
//! @brief Sets the initial status of the dictionaries in the settings dialog
//!
void gw_settings_initialize_installed_dictionary_list () 
{
    GtkWidget *table;
    GwDictInfo *di;
    GwUiDictInstallLine *il;

    table = GTK_WIDGET (gtk_builder_get_object (builder, "dictionaries_table"));

    di =  gw_dictlist_get_dictinfo_by_name ("English");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictinfo_by_name ("Kanji");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictinfo_by_name ("Names");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictinfo_by_name ("Examples");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    table = GTK_WIDGET (gtk_builder_get_object (builder, "other_dictionaries_table"));

    di =  gw_dictlist_get_dictinfo_by_name ("French");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictinfo_by_name ("German");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictinfo_by_name ("Spanish");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    gw_ui_update_settings_interface ();
}


//!
//! @brief Updates the dictionary orders for the dictionary order tab
//!
void gw_ui_initialize_dictionary_order_list ()
{
      #define XPADDING 4
      #define YPADDING 0

      GtkListStore *list_store = GTK_LIST_STORE (gtk_builder_get_object (builder, "list_store_dictionaries"));
      GtkWidget *treeview = GTK_WIDGET (gtk_builder_get_object (builder, "organize_dictionaries_treeview"));

      GtkWidget *down_button = GTK_WIDGET (gtk_builder_get_object (builder, "move_dictionary_down_button"));
      GtkWidget *up_button = GTK_WIDGET (gtk_builder_get_object (builder, "move_dictionary_up_button"));
      g_signal_connect(G_OBJECT (down_button), "clicked", G_CALLBACK (do_move_dictionary_down), (gpointer) treeview);
      g_signal_connect(G_OBJECT (up_button), "clicked", G_CALLBACK (do_move_dictionary_up), (gpointer) treeview);

      gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
      gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
      gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (list_store));

      GtkCellRenderer *renderer;
      GtkTreeViewColumn *column;

      renderer = gtk_cell_renderer_pixbuf_new ();
      #if GTK_CHECK_VERSION(2,18,0)
      gtk_cell_renderer_set_padding (renderer, YPADDING, XPADDING);
      #endif
      column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "icon-name", 1, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      renderer = gtk_cell_renderer_text_new ();
      #if GTK_CHECK_VERSION(2,18,0)
      gtk_cell_renderer_set_padding (renderer, YPADDING, XPADDING);
      #endif
      column = gtk_tree_view_column_new_with_attributes (gettext("Order"), renderer, "text", 2, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      renderer = gtk_cell_renderer_text_new ();
      #if GTK_CHECK_VERSION(2,18,0)
      gtk_cell_renderer_set_padding (renderer, YPADDING, XPADDING);
      gtk_cell_renderer_set_sensitive (renderer, FALSE);
      #endif
      column = gtk_tree_view_column_new_with_attributes (gettext("Shortcut"), renderer, "text", 3, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      renderer = gtk_cell_renderer_text_new ();
      #if GTK_CHECK_VERSION(2,18,0)
      gtk_cell_renderer_set_padding (renderer, YPADDING, XPADDING);
      #endif
      column = gtk_tree_view_column_new_with_attributes (gettext("Dictionary"), renderer, "text", 0, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      #undef XPADDING
      #undef YPADDING
}


void gw_ui_update_dictionary_order_list ()
{
  printf("Updating distiary order list\n");
      GtkListStore *list_store = GTK_LIST_STORE (gtk_builder_get_object (builder, "list_store_dictionaries"));

      GtkTreeIter iter;
      if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter) == FALSE) return;
      char *icon_name = NULL;
      char *favorite = "emblem-favorite";
      char *order = NULL;
      char *shortcut = NULL;
      gboolean can_continue = TRUE;

      int i = 1;

      while (i < 10 && can_continue)
      {
        if (i == 1) icon_name = favorite;
        else icon_name = NULL;
        order = g_strdup_printf ("%d", i);
        shortcut = g_strdup_printf ("Alt-%d", i);
        gtk_list_store_set (GTK_LIST_STORE (list_store), &iter, 1, icon_name, 2, order, 3, shortcut, -1); 
        g_free (order);
        g_free (shortcut);
        i++;
        can_continue = gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter);
      }
      while (can_continue)
      {
        icon_name = NULL;
        order = g_strdup_printf ("%d", i);
        shortcut = NULL;
        gtk_list_store_set (GTK_LIST_STORE (list_store), &iter, 1, icon_name, 2, order, 3, shortcut, -1); 
        g_free (order);
        i++;
        can_continue = gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter);
      }
}

void gw_ui_set_use_global_document_font_checkbox (gboolean setting)
{
    GtkWidget *checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "system_font_checkbox"));
    GtkWidget *child_settings = GTK_WIDGET (gtk_builder_get_object (builder, "system_document_font_hbox"));

    g_signal_handlers_block_by_func (GTK_WIDGET (checkbox), do_toggle_use_global_document_font, NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), setting);
    gtk_widget_set_sensitive (child_settings, !setting);
    g_signal_handlers_unblock_by_func (GTK_WIDGET (checkbox), do_toggle_use_global_document_font, NULL);
}


void gw_ui_update_global_font_label (const char *font_description_string)
{
    GtkWidget *checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "system_font_checkbox"));
    char *text = g_strdup_printf (gettext("_Use the System Document Font (%s)"), font_description_string);

    if (text != NULL)
    {
      gtk_button_set_label (GTK_BUTTON (checkbox), text);
      g_free (text);
      text = NULL;
    }
}


void gw_ui_update_custom_font_button (const char *font_description_string)
{
    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "custom_font_fontbutton"));
    g_signal_handlers_block_by_func (GTK_WIDGET (button), do_set_custom_document_font, NULL);
    gtk_font_button_set_font_name (GTK_FONT_BUTTON (button), font_description_string);
    g_signal_handlers_unblock_by_func (GTK_WIDGET (button), do_set_custom_document_font, NULL);
}


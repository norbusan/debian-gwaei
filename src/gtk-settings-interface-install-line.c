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
//! @file src/gtk-settings-interface-install-line.c
//!
//! @brief Abstraction layer for gtk object
//!
//! A congromeration of gtk widgets to form a gwaei install line object for easy
//! maintenance.
//!

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <regex.h>

#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>

#include <gwaei/engine.h>
#include <gwaei/preferences.h>
#include <gwaei/gtk.h>
#include <gwaei/gtk-settings-callbacks.h>
#include <gwaei/gtk-settings-interface.h>
#include <gwaei/gtk-settings-interface-install-line.h>


//!
//! @brief Sets the initial default install state of the GwUiDictInstallLine object.
//!
//! @param di A GwuiDictInstallLine to set the state of
//!
void gw_settings_initialize_dictionary_ui_status (GwUiDictInstallLine *il) 
{
    if (gw_dictlist_dictionary_get_status_by_id (il->di->id) == GW_DICT_STATUS_INSTALLED)
    {
      gw_ui_dict_install_set_action_button (il, GTK_STOCK_DELETE, TRUE);
      gw_ui_dict_install_set_message (il, GTK_STOCK_APPLY, gettext("Installed"));
    }
    else
    {
      gw_ui_dict_install_set_action_button (il, GTK_STOCK_ADD, TRUE);
      gw_ui_dict_install_set_message (il, NULL, gettext("Not Installed"));
    }
}


//!
//! @brief The callback for when the widgets of the GwInstallLine get destroyed to free memory
//!
//! @param object The GtkObject that the destroy was called on
//! @param data The GwUiDictInstallLine passed as a gpointer
//!
void gw_ui_dict_install_line_destroy_action (GtkObject *object, gpointer data)
{
    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    free (il);
    printf("Freeing\n");
    il = NULL;
}


//!
//! @brief Allocates and creates a new GwUiDictInstallLine object.
//!
//! @param di A GwDictInfo object to get information from
//!
GwUiDictInstallLine *gw_ui_new_dict_install_line (GwDictInfo *di)
{
    GwUiDictInstallLine *temp = NULL;

    if ((temp = (GwUiDictInstallLine*) malloc(sizeof(GwUiDictInstallLine))) != NULL)
    {
      temp->di = di;

      //Row 1 Column 1 Expander with name
      char *name = g_strdup_printf ("%s: ", di->short_name);
      temp->advanced_expander = gtk_expander_new (name);
      g_free (name);
      temp->advanced_hbox = gtk_hbox_new (FALSE, 0);

      gtk_box_pack_start (GTK_BOX (temp->advanced_hbox), temp->advanced_expander, FALSE, FALSE, 0);

      //Row 1 Column 2 Status messages area
      temp->status_icon = gtk_image_new_from_stock (GTK_STOCK_APPLY, GTK_ICON_SIZE_SMALL_TOOLBAR);
      temp->status_message = gtk_label_new (" ");
      temp->status_progressbar = gtk_progress_bar_new ();
      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (temp->status_progressbar), gettext("Downloading..."));

      temp->status_hbox = gtk_hbox_new (FALSE, 0);
      gtk_widget_set_size_request (GTK_WIDGET (temp->status_hbox), 150, -1);
      temp->message_hbox = gtk_hbox_new (FALSE, 0);

      gtk_box_pack_start (GTK_BOX (temp->status_hbox), temp->status_progressbar, TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (temp->message_hbox), temp->status_icon, FALSE, FALSE, 1);
      gtk_box_pack_start (GTK_BOX (temp->message_hbox), temp->status_message, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (temp->status_hbox), temp->message_hbox, TRUE, FALSE, 0);

      //Row 1 Column 3 Button action area to do things to dictionaries
      temp->action_button = gtk_button_new_from_stock (GTK_STOCK_ADD);

      temp->action_button_hbox = gtk_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (temp->action_button_hbox), temp->action_button, TRUE, TRUE, 0);

      //Row 2 Area for other less general options such as install URI selection
      temp->source_uri_entry = gtk_entry_new ();
      g_signal_connect (G_OBJECT (temp->source_uri_entry), "changed", G_CALLBACK (do_source_entry_changed_action), temp);
      GtkWidget *label = gtk_label_new (gettext("Source: "));
      GtkWidget *alignment = gtk_alignment_new (0, 0, 1, 1);
      gtk_widget_set_size_request (GTK_WIDGET (alignment), 20, -1);
      temp->source_browse_button = gtk_button_new_with_label (gettext("Browse..."));
      g_signal_connect (G_OBJECT (temp->source_browse_button), "clicked", G_CALLBACK (do_dictionary_source_browse), temp);
      gw_prefs_add_change_listener (di->gckey, do_dictionary_source_gconf_key_changed_action, temp->source_uri_entry);
      temp->source_reset_button = gtk_button_new_with_label (gettext("Reset"));
      g_signal_connect (G_OBJECT (temp->source_reset_button), "clicked", G_CALLBACK (do_dictionary_source_reset), temp);

      temp->source_hbox = gtk_hbox_new (FALSE, 0);
      gtk_box_pack_start (GTK_BOX (temp->source_hbox), alignment, FALSE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (temp->source_hbox), label, FALSE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (temp->source_hbox), temp->source_uri_entry, TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (temp->source_hbox), temp->source_browse_button, FALSE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (temp->source_hbox), temp->source_reset_button, FALSE, TRUE, 0);

      g_signal_connect (G_OBJECT (temp->advanced_expander), "activate", G_CALLBACK (do_toggle_advanced_source), temp->source_hbox);
      g_signal_connect (G_OBJECT (temp->advanced_expander), "destroy", G_CALLBACK (gw_ui_dict_install_line_destroy_action), temp);

      return temp;
    }
    printf("Error creating dictionary install line gui object.");
    return NULL;
}


//!
//! @brief Appends the new dictionary install lines to an existing table
//!
//! If the table doesn't have a enough columns, it is expanded new rows are
//! added to add the new install lines.
//!
//! @param table A GtkTable to append the install lines to
//! @param data il A pointer to a GwUiDictInstallLine to use to append widgets to the table
//!
void gw_ui_add_dict_install_line_to_table (GtkTable *table, GwUiDictInstallLine *il)
{
    if (table->ncols < 3) gtk_table_resize (table, table->nrows, 3);

    int row = table->nrows;
    gtk_table_resize (table, table->nrows + 2, table->ncols);

    gtk_table_attach (table, il->advanced_hbox, 0, 1, row, row + 1, GTK_FILL, 0, 0, 0);
    gtk_table_attach (table, il->status_hbox, 1, 2, row, row + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0 ,0);
    gtk_table_attach (table, il->action_button_hbox, 2, 3, row, row + 1, GTK_FILL, GTK_FILL, 0, 0);
    gtk_widget_show_all (il->advanced_hbox);
    gtk_widget_show_all (il->status_hbox);
    gtk_widget_show_all (il->action_button_hbox);

    row++;

    gtk_table_attach (table, il->source_hbox, 0, 3, row, row + 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);

    gw_settings_initialize_dictionary_ui_status (il);
}


//!
//! @brief Frees memory used by a GwUiDictInstallLine object
//!
//! @param il A pointer to a GwUiDictInstallLine to free.
//!
void gw_ui_destroy_dict_install_line (GwUiDictInstallLine *il)
{
    gtk_widget_destroy (il->status_hbox);
    il->status_hbox = NULL;
    il->message_hbox = NULL;
    il->status_icon = NULL;
    il->status_message = NULL;
    il->status_progressbar = NULL;

    gtk_widget_destroy (il->action_button_hbox);
    il->action_button_hbox = NULL;
    il->action_button = NULL;

    gtk_widget_destroy (il->source_hbox);
    il->source_uri_entry = NULL;
    il->source_browse_button = NULL;
    il->source_reset_button = NULL;

    gtk_widget_destroy (il->advanced_hbox);
    il->advanced_hbox = NULL;
    il->advanced_expander = NULL;

    free (il);
}


//!
//! @brief Sets the state of the install line action button.
//!
//! The currently supported states are GTK_STOCK_ADD, GTK_STOCK_REMOVE and GTK_STOCK_CANCEL.
//! All other states will not get a callback attached.
//
//! @param il A pointer to a GwUiDictInstallLine to manipulate the action button on.
//! @param A gtk stock id to use to select the button wanted.
//! @param A boolean to decide if the button should be in a disabled state.
//!
void gw_ui_dict_install_set_action_button (GwUiDictInstallLine *il, const gchar *STOCK_ID, gboolean SENSITIVE)
{
    GtkWidget *parent = gtk_widget_get_parent (il->action_button);
    gtk_widget_destroy (il->action_button);
    il->action_button = gtk_button_new_from_stock (STOCK_ID);
    if (parent != NULL)
    {
      gtk_box_pack_start (GTK_BOX (parent), il->action_button, TRUE, TRUE, 0);
      gtk_widget_show (il->action_button);

      if (strcmp (STOCK_ID, GTK_STOCK_ADD) == 0)
        g_signal_connect(il->action_button, "clicked", G_CALLBACK (do_dictionary_install), (gpointer) il);
      if (strcmp (STOCK_ID, GTK_STOCK_DELETE) == 0)
        g_signal_connect(il->action_button, "clicked", G_CALLBACK (do_dictionary_remove), (gpointer) il);
      if (strcmp (STOCK_ID, GTK_STOCK_CANCEL) == 0)
        g_signal_connect(il->action_button, "clicked", G_CALLBACK (do_cancel_dictionary_install), (gpointer) il);

      gtk_widget_set_sensitive (il->action_button, SENSITIVE);
      gtk_widget_set_size_request (il->action_button, 120, -1);

      gboolean advanced_hbox_is_sensitive;
      advanced_hbox_is_sensitive = (strcmp (STOCK_ID, GTK_STOCK_ADD) == 0);
      gtk_widget_set_sensitive(il->source_hbox, advanced_hbox_is_sensitive);
    }
}


//!
//! @brief Sets the progress of a GwUiDictInstallLIne progressbar
//!
//! Using this function automatically hides the message text and show a progressbar
//! with the wanted amount of progress filled.  
//!
//! @param il A pointer to a GwUiDictInstallLine to manipulate the action button on.
//! @param FRACTION A constant gdouble for the percent of the bar to fill.
//!
void gw_ui_progressbar_set_fraction_by_install_line (GwUiDictInstallLine *il, const gdouble FRACTION)
{
    gtk_widget_hide (il->message_hbox);
    gtk_widget_show (il->status_progressbar);

    if (FRACTION < 0.0)
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (il->status_progressbar), 0.0);
    else if (FRACTION > 1.0)
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (il->status_progressbar), 1.0);
    else
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (il->status_progressbar), FRACTION);
}


//!
//! @brief Sets the a wanted message to the status message area with an optional icon.
//!
//! This funcion automatically hides the progressbar set by_dict_install_line_progress_bar_set_fraction (). 
//!
//! @param il A pointer to a GwUiDictInstallLine to manipulate the action button on.
//! @param STOCK_ICON_ID A constant character string of the wanted gtk stock icon.
//! @param message_text A contstat character string set the message text to.
//!
void gw_ui_dict_install_set_message (GwUiDictInstallLine *il, const gchar *STOCK_ICON_ID, const char *message_text)
{
    //Take care of the icon
    if (STOCK_ICON_ID == NULL)
    {
      gtk_widget_hide (il->status_icon);
    }
    else
    {
      gtk_widget_hide (il->status_progressbar);
      gtk_image_set_from_stock (GTK_IMAGE (il->status_icon), STOCK_ICON_ID, GTK_ICON_SIZE_SMALL_TOOLBAR);
      gtk_widget_show (il->status_icon);
      gtk_widget_show (il->message_hbox);
    }

    //Take care of the message text
    if (message_text == NULL)
    {
      gtk_widget_hide (il->status_message);
    }
    else
    {
      gtk_widget_hide (il->status_progressbar);
      gtk_label_set_text (GTK_LABEL (il->status_message), message_text);
      gtk_widget_show (il->status_message);
      gtk_widget_show (il->message_hbox);
    }
}


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
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
G_MODULE_EXPORT void gw_settings_update_interface ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *message;
    GList *list;

    //Initializations
    builder = gw_common_get_builder ();
    message = GTK_WIDGET (gtk_builder_get_object (builder, "please_install_dictionary_hbox"));
    list = lw_dictinfolist_get_list ();

    //Set the show state of the dictionaries required message
    if (g_list_length (list) > 0)
      gtk_widget_hide (message);
    else
      gtk_widget_show (message);
}


//!
//! @brief Sets the text in the source gtkentry for the appropriate dictionary
//!
//! @param widget Pointer to a GtkEntry to set the text of
//! @param value The constant string to use as the source for the text
//!
void gw_settings_set_dictionary_source (GtkWidget *widget, const char* value)
{
    if (widget != NULL && value != NULL)
    {
      gtk_entry_set_text (GTK_ENTRY (widget), value);
    }
}


//!
//! @brief Sets the initial status of the dictionaries in the settings dialog
//!
void gw_settings_initialize () 
{
    gw_common_load_ui_xml ("settings.ui");
    gw_settings_listeners_initialize ();
    gw_settings_update_interface ();
}


//!
//! @brief Frees the memory used by the settings
//!
void gw_settings_free ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *window;

    //Initializations
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));

    //Free things
    gw_settings_listeners_free ();
    gtk_widget_destroy (window);
}


//!
//! @brief Sets the state of the use global document font checkbox without triggering the signal handler
//!
//! @param setting The new checked state for the use global document font checkbox 
//!
void gw_settings_set_use_global_document_font_checkbox (gboolean setting)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *checkbox;
    GtkWidget *hbox;

    //Initializations
    builder = gw_common_get_builder ();
    checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "system_font_checkbox"));
    hbox = GTK_WIDGET (gtk_builder_get_object (builder, "system_document_font_hbox"));

    //Updates
    g_signal_handlers_block_by_func (checkbox, gw_settings_use_global_document_font_toggled_cb, NULL);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), setting);
    gtk_widget_set_sensitive (hbox, !setting);

    g_signal_handlers_unblock_by_func (checkbox, gw_settings_use_global_document_font_toggled_cb, NULL);
}


//!
//! @brief Updates the default global font label to match that of the system.  The fallback is Sans
//!
//! @param font_description_string The font description in the form "Sans 10"
//!
void gw_settings_update_global_font_label (const char *font_description_string)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *checkbox;
    char *text;

    //Initializations
    builder = gw_common_get_builder ();
    checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "system_font_checkbox"));
    text = g_strdup_printf (gettext("_Use the System Document Font (%s)"), font_description_string);

    //Body
    if (text != NULL) gtk_button_set_label (GTK_BUTTON (checkbox), text);

    //Cleanup
    g_free (text);
}


//!
//! @brief Sets the text in the custom document font button
//!
//! @param font_description_string The font description in the form "Sans 10"
//!
void gw_settings_update_custom_font_button (const char *font_description_string)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *button;

    //Initializations
    builder = gw_common_get_builder ();
    button = GTK_WIDGET (gtk_builder_get_object (builder, "custom_font_fontbutton"));

    //Body
    gtk_font_button_set_font_name (GTK_FONT_BUTTON (button), font_description_string);
}


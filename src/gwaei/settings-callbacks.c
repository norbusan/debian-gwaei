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
//! @file src/gtk-settings-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief Callback to toggle the hiragana-katakana conversion setting for the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_hira_kata_conv_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA);
    lw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA, !state);
}


//!
//! @brief Callback to toggle the katakana-hiragana conversion setting for the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_kata_hira_conv_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA);
    lw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA, !state);
}


//!
//! @brief Callback to toggle spellcheck in the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_spellcheck_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK);
    lw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK, !state);
}


//!
//! @brief Callback to toggle romaji-kana conversion
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_romaji_kana_conv_changed_cb (GtkWidget *widget, gpointer data)
{
    int active;
    active = gtk_combo_box_get_active(GTK_COMBO_BOX (widget));
    lw_pref_set_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA, active);
}


//!
//! @brief Callback to set the user selected color to the color swatch for text highlighting
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_swatch_color_set_cb (GtkWidget *widget, gpointer data)
{
    //Initializations
    GdkColor color;
    gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &color);
    char *hex_color_string = NULL;
    hex_color_string = gdk_color_to_string (&color);
    char *pref_key = NULL;
    pref_key = g_strdup_printf ("%s", gtk_buildable_get_name (GTK_BUILDABLE (widget)));
    char *letter = strchr(pref_key, '_');
    if (letter == NULL) return;
    *letter = '-';

    //Set the color inthe prefs
    if (pref_key != NULL && hex_color_string != NULL)
    {
      lw_pref_set_string_by_schema (GW_SCHEMA_HIGHLIGHT, pref_key, hex_color_string);
    }

    //Cleanup
    if (pref_key != NULL)
    {
      g_free (pref_key);
      pref_key = NULL;
    }
    if (hex_color_string != NULL)
    {
      g_free (hex_color_string);
      hex_color_string = NULL;
    }
}


//!
//! @brief Callback to reset all the colors for all the swatches to the default in the preferences
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_swatch_color_reset_cb (GtkWidget *widget, gpointer data)
{
    //Initializations
    char fallback[100];
    int i = 0;
    char *pref_key[] = {
      GW_KEY_MATCH_FG,
      GW_KEY_MATCH_BG,
      GW_KEY_HEADER_FG,
      GW_KEY_HEADER_BG,
      GW_KEY_COMMENT_FG,
      NULL
    };

    //Start setting the default values
    for (i = 0; pref_key[i] != NULL; i++)
    {
      lw_pref_reset_value_by_schema (GW_SCHEMA_HIGHLIGHT, pref_key[i]);
    }

    //gw_ui_buffer_reload_tagtable_tags();
}


//!
//! @brief Sets the preference key for the global font usage
//! 
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_use_global_document_font_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = lw_pref_get_boolean_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_USE_GLOBAL_FONT);
    lw_pref_set_boolean_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_USE_GLOBAL_FONT, !state);
}


//!
//! @brief Sets the preference key for the new custom document font
//! 
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_custom_document_font_set_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "custom_font_fontbutton"));
    const char *font_description_string = gtk_font_button_get_font_name (GTK_FONT_BUTTON (button));
    lw_pref_set_string_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_CUSTOM_FONT, font_description_string);
}


//!
//! @brief Brings up the preferences dialog to change settings
//!
//! This function sets up the dialog window, makes sure no searches are
//! currently running, then makes the window appear.
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_settings_show_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    gw_main_tab_cancel_all_searches ();
    gw_main_cancel_search_by_target (GW_TARGET_KANJI);

    //Setup please install dictionary message and notebook page
    GtkWidget *notebook;
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "settings_notebook"));
    if (data != NULL)
    {
      gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), GPOINTER_TO_INT (data));
    }
    else
    {
      gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);
    }

    //get some needed window references
    gw_settings_update_interface ();
    gw_common_show_window ("settings_window");
}




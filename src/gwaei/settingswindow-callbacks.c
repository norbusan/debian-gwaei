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
//! @file settings-callbacks.c
//!
//! @brief To be written
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/gettext.h>
#include <gwaei/settingswindow-private.h>


//!
//! @brief Callback to toggle the hiragana-katakana conversion setting for the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_hira_kata_conv_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_HIRA_KATA);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_HIRA_KATA, !request);
}


//!
//! @brief Callback to toggle the katakana-hiragana conversion setting for the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_kata_hira_conv_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_KATA_HIRA);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_KATA_HIRA, !request);
}


//!
//! @brief Callback to toggle spellcheck in the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_spellcheck_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK, !request);
}


//!
//! @brief Callback to toggle search as you type in the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_search_as_you_type_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SEARCH_AS_YOU_TYPE);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SEARCH_AS_YOU_TYPE, !request);
}



//!
//! @brief Callback to toggle romaji-kana conversion
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_romaji_kana_conv_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    int active;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    active = gtk_combo_box_get_active (priv->romaji_to_kana_combobox);

    lw_preferences_set_int_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA, active);
}


//!
//! @brief Callback to set the user selected color to the color swatch for text highlighting
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_swatch_color_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    GdkRGBA color;
    char *hex_color_string;
    char *pref_key;
    char *letter;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (widget), &color);
    hex_color_string = gdk_rgba_to_string (&color);
    pref_key = g_strdup_printf ("%s", gtk_buildable_get_name (GTK_BUILDABLE (widget)));
    letter = strchr(pref_key, '_');
    if (letter == NULL) return;
    *letter = '-';

    letter = strchr(letter, '_');
    if (letter == NULL) return;
    *letter = '\0';

    //Set the color in the prefs
    if (pref_key != NULL && hex_color_string != NULL)
    {
      lw_preferences_set_string_by_schema (preferences, LW_SCHEMA_HIGHLIGHT, pref_key, hex_color_string);
    }

    //Cleanup
    if (pref_key != NULL) g_free (pref_key);
    if (hex_color_string != NULL) g_free (hex_color_string);
}


//!
//! @brief Callback to reset all the colors for all the swatches to the default in the preferences
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_reset_all_swatches_activated_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    int i;
    char *pref_key[] = {
      LW_KEY_MATCH_FG,
      LW_KEY_MATCH_BG,
      LW_KEY_HEADER_FG,
      LW_KEY_HEADER_BG,
      LW_KEY_COMMENT_FG,
      NULL
    };

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    //Start setting the default values
    for (i = 0; pref_key[i] != NULL; i++)
    {
      lw_preferences_reset_value_by_schema (preferences, LW_SCHEMA_HIGHLIGHT, pref_key[i]);
    }
}


//!
//! @brief Sets the preference key for the global font usage
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_use_global_document_font_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_USE_GLOBAL_FONT);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_USE_GLOBAL_FONT, !request);
}


//!
//! @brief Sets the preference key for the new custom document font
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_custom_document_font_changed_cb (GtkWidget *widget, gpointer data)
{
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    const char *font;

    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    font = gtk_font_button_get_font_name (priv->custom_font_fontbutton);

    lw_preferences_set_string_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_CUSTOM_FONT, font);
}


//!
//! @brief Sets the state of the use global document font checkbox without triggering the signal handler
//! @param setting The new checked state for the use global document font checkbox 
//!
G_MODULE_EXPORT void 
gw_settingswindow_sync_use_global_document_font_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleButton *checkbutton;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    request = lw_preferences_get_boolean (settings, KEY);
    checkbutton = GTK_TOGGLE_BUTTON (priv->system_font_checkbutton);

    //Updates
    G_GNUC_EXTENSION g_signal_handlers_block_by_func (checkbutton, gw_settingswindow_use_global_document_font_toggled_cb, toplevel);
    gtk_toggle_button_set_active (checkbutton, request);
    gtk_widget_set_sensitive (GTK_WIDGET (priv->system_document_font_hbox), !request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (checkbutton, gw_settingswindow_use_global_document_font_toggled_cb, toplevel);
}


G_MODULE_EXPORT void 
gw_settingswindow_sync_global_document_font_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    gchar font[50];
    gchar *font2;
    gchar *text;
    PangoFontDescription *desc;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    lw_preferences_get_string_by_schema (preferences, font, LW_SCHEMA_GNOME_INTERFACE, LW_KEY_DOCUMENT_FONT_NAME, 50);
    desc = pango_font_description_from_string (font);
    pango_font_description_set_family (desc, "Serif");
    font2 = pango_font_description_to_string (desc);
    if (font2) text = g_strdup_printf (gettext("_Use the System Document Font (%s)"), font2);
    g_free (font2); font2 = NULL;
    pango_font_description_free (desc); desc = NULL;

    if (text != NULL) 
    {
      gtk_button_set_label (GTK_BUTTON (priv->system_font_checkbutton), text);
      g_free (text);
    }
}


//!
//! @brief Sets the text in the custom document font button
//! @param font_description_string The font description in the form "Sans 10"
//!
G_MODULE_EXPORT void 
gw_settingswindow_sync_custom_font_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    GtkWidget *toplevel;
    GtkButton *button;
    char font[50];

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    lw_preferences_get_string_by_schema (preferences, font, LW_SCHEMA_FONT, LW_KEY_FONT_CUSTOM_FONT, 50);
    button = GTK_BUTTON (priv->custom_font_fontbutton);

    //Body
    G_GNUC_EXTENSION g_signal_handlers_block_by_func (button, gw_settingswindow_custom_document_font_changed_cb, toplevel);
    gtk_font_button_set_font_name (GTK_FONT_BUTTON (button), font);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (button, gw_settingswindow_custom_document_font_changed_cb, toplevel);
}


//!
//! @brief Sets the checkbox to show or hide the toolbar
//! @param request How to set the toolbar
//!
G_MODULE_EXPORT void 
gw_settingswindow_sync_search_as_you_type_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    GtkWidget *toplevel;
    gboolean request;
    GtkToggleButton *togglebutton;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SEARCH_AS_YOU_TYPE);
    togglebutton = GTK_TOGGLE_BUTTON (priv->search_as_you_type_checkbutton);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (togglebutton, gw_settingswindow_search_as_you_type_toggled_cb, toplevel);
    gtk_toggle_button_set_active (togglebutton, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (togglebutton, gw_settingswindow_search_as_you_type_toggled_cb, toplevel);
}


//!
//! @brief Closes the window passed throught the widget pointer
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    GwDictionaryList *dictionarylist;
    
    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = gw_application_get_installed_dictionarylist (application);

    gtk_widget_destroy (GTK_WIDGET (window));

    if (lw_dictionarylist_get_total (LW_DICTIONARYLIST (dictionarylist)) == 0)
      gw_application_quit (application);
}


G_MODULE_EXPORT void 
gw_settingswindow_sync_romaji_kana_conv_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GtkComboBox *combobox;
    int request;

    window = GW_SETTINGSWINDOW (data);
    priv = window->priv;
    combobox = priv->romaji_to_kana_combobox;
    request = lw_preferences_get_int (settings, KEY);

    gtk_combo_box_set_active (combobox, request);
}


G_MODULE_EXPORT void 
gw_settingswindow_sync_hira_kata_conv_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleButton *togglebutton;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    togglebutton = GTK_TOGGLE_BUTTON (priv->hiragana_to_katakana_checkbutton);
    request = lw_preferences_get_boolean (settings, KEY);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (togglebutton, gw_settingswindow_hira_kata_conv_toggled_cb, toplevel);
    gtk_toggle_button_set_active (togglebutton, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (togglebutton, gw_settingswindow_hira_kata_conv_toggled_cb, toplevel);
}


G_MODULE_EXPORT void 
gw_settingswindow_sync_kata_hira_conv_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleButton *togglebutton;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    togglebutton = GTK_TOGGLE_BUTTON (priv->katakana_to_hiragana_checkbutton);
    request = lw_preferences_get_boolean (settings, KEY);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (togglebutton, gw_settingswindow_kata_hira_conv_toggled_cb, toplevel);
    gtk_toggle_button_set_active (togglebutton, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (togglebutton, gw_settingswindow_kata_hira_conv_toggled_cb, toplevel);
}


G_MODULE_EXPORT void 
gw_settingswindow_sync_swatch_color_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkColorChooser *swatch;
    GdkRGBA color;
    char hex_color_string[20];

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    swatch = GTK_COLOR_CHOOSER (data);
    lw_preferences_get_string (hex_color_string, settings, KEY, 20);
    g_assert (swatch != NULL);

    if (gdk_rgba_parse (&color, hex_color_string) == TRUE)
    {
      G_GNUC_EXTENSION g_signal_handlers_block_by_func (swatch, gw_settingswindow_swatch_color_changed_cb, window);
      gtk_color_chooser_set_rgba (swatch, &color);
      G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (swatch, gw_settingswindow_swatch_color_changed_cb, window);
    }
}


G_MODULE_EXPORT void 
gw_settingswindow_sync_spellcheck_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleButton *togglebutton;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    togglebutton = GTK_TOGGLE_BUTTON (priv->spellcheck_checkbutton);
    request = lw_preferences_get_boolean (settings, KEY);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (togglebutton, gw_settingswindow_spellcheck_toggled_cb, toplevel);
    gtk_toggle_button_set_active (togglebutton, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (togglebutton, gw_settingswindow_spellcheck_toggled_cb, toplevel);
}


//!
//! @brief Preforms the action the window manager close event
//! @see gw_searchwindow_close_cb ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean 
gw_settingswindow_delete_event_action_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{ 
    gw_settingswindow_close_cb (widget, data);
    return TRUE;
}


void 
gw_settingswindow_remove_dictionary_cb (GSimpleAction *action, 
                                        GVariant      *parameter,
                                        gpointer       data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    GwDictionaryList *dictionarylist;

    GtkTreePath *path;
    GtkTreeIter iter;
    LwDictionary *dictionary;
    GError *error;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    gboolean has_selection;
    gint* indices;
    GtkTreeView *view;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = gw_application_get_installed_dictionarylist (application);
    view = priv->manage_dictionaries_treeview;
    selection = gtk_tree_view_get_selection (view);
    model = GTK_TREE_MODEL (gw_dictionarylist_get_liststore (dictionarylist));
    has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
    error = NULL;

    //Sanity check
    if (!has_selection) return;

    path = gtk_tree_model_get_path (model, &iter);
    indices = gtk_tree_path_get_indices (path);
    dictionary = lw_dictionarylist_remove_by_position (LW_DICTIONARYLIST (dictionarylist), *indices);

    if (dictionary != NULL)
    {
      lw_dictionary_uninstall (dictionary, NULL, &error);
    }

    //Cleanup
    gtk_tree_path_free (path); path = NULL;

    if (error != NULL)
    {
      gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);
      exit(EXIT_SUCCESS);
    }

    gw_settingswindow_check_for_dictionaries (window);
}


void 
gw_settingswindow_add_dictionary_cb (GSimpleAction *action, 
                                     GVariant      *parameter,
                                     gpointer       data)
{
    GwSettingsWindow *window;
    GtkWindow *dictionaryinstallwindow;
    GwApplication *application;

    window = GW_SETTINGSWINDOW (data);
    application = gw_window_get_application (GW_WINDOW (window));

    dictionaryinstallwindow = gw_dictionaryinstallwindow_new (GTK_APPLICATION (application));
    gtk_window_set_transient_for (GTK_WINDOW (dictionaryinstallwindow), GTK_WINDOW (window));
    gtk_widget_show (GTK_WIDGET (dictionaryinstallwindow));
}


//!
//! @brief Opens the dictionary folder using the user's default file browser
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_settingswindow_open_dictionary_folder_cb (GtkWidget *widget, gpointer data) 
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    char *directory;
    char *uri;
    GError *error;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    directory = lw_util_build_filename (LW_PATH_DICTIONARY, NULL);
    uri = g_build_filename ("file://", directory, NULL);
    error = NULL;

    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);

    gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);

    g_free (uri);
    g_free (directory);
}




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
//! @file src/gtk-gconf-preferences.c
//!
//! @brief Abstraction layer for gconf preferences
//!
//! Allows access to gconf with the ability to specify backup preferences upon
//! failure to get the preference value.
//!


#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdk.h>
#include <gconf/gconf-client.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>

#include <gwaei/gtk-main-interface.h>
#ifdef WITH_LIBSEXY
#include <gwaei/gtk-main-interface-sexy.h>
#endif
#include <gwaei/gtk-settings-interface.h>


//!
//! @brief Returns an integer from the preference backend 
//!
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//!
int gw_pref_get_int (const char *key, const int backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    
    int return_value;
    GError *err = NULL;

    return_value = gconf_client_get_int (client, key, &err);

    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }

    g_object_unref (client);

    return return_value;
}


//!
//! @brief Returns the default integer from the preference backend 
//!
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//!
int gw_pref_get_default_int (const char *key, const int backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();

    GConfValue *value;
    int return_value;
    GError *err = NULL;

    value = gconf_client_get_default_from_schema (client, key, &err);
    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }
    if (value == NULL || value->type != GCONF_VALUE_INT)
    {
      return_value = backup;
    }
    else
    {
      return_value = gconf_value_get_int (value);
    }
    g_object_unref (client);

    return return_value;
}


//!
//! @brief Sets the int to the key in the preferences backend
//!
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void gw_pref_set_int (const char *key, const int request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_int (client, key, request, NULL);
    g_object_unref (client);
}


//!
//! @brief Returns an boolean from the preference backend 
//!
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//!
gboolean gw_pref_get_boolean (const char *key, const gboolean backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    
    gboolean return_value;
    GError *err = NULL;

   return_value = gconf_client_get_bool (client, key, &err);

    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }

    g_object_unref (client);

    return return_value;
}


//!
//! @brief Returns the default boolean from the preference backend 
//!
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//!
gboolean gw_pref_get_default_boolean (const char *key, const gboolean backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();

    GConfValue *value;
    gboolean return_value;
    GError *err = NULL;

    value = gconf_client_get_default_from_schema (client, key, &err);
    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }
    else if (value == NULL || value->type != GCONF_VALUE_BOOL)
    {
      return_value = backup;
    }
    else
    {
      return_value = gconf_value_get_bool (value);
    }
    g_object_unref (client);

    return return_value;
}


//!
//! @brief Sets the boolean to the key in the preferences backend
//!
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void gw_pref_set_boolean (const char *key, const gboolean request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_bool (client, key, request, NULL);
    g_object_unref (client);
}

//!
//! @brief Returns an string from the preference backend 
//!
//! @output string to copy the pref to
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//! @param n The max characters to copy to output
//!
void gw_pref_get_string (char *output, const char *key, const char* backup, const int n)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    
    char* return_value;
    GError *err = NULL;

    return_value = gconf_client_get_string (client, key, &err);

    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = NULL;
      strncpy(output, backup, n);
    }
    else if (return_value == NULL || strlen(return_value) == 0)
    {
      strncpy(output, backup, n);
    }
    else
    {
      strncpy(output, return_value, n);
    }

    g_object_unref (client);
}


//!
//! @brief Returns the default string from the preference backend 
//!
//! @output string to copy the pref to
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//! @param n The max characters to copy to output
//!
void gw_pref_get_default_string (char* output, const char *key, const char* backup, const int n)
{
    GConfClient *client;
    client = gconf_client_get_default ();

    GConfValue *value;
    const char* return_value;
    GError *err = NULL;

    value = gconf_client_get_default_from_schema (client, key, &err);
    if (err != NULL || (value != NULL && value->type != GCONF_VALUE_STRING))
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }
    else
    {
      return_value = gconf_value_get_string (value);
    }
    g_object_unref (client);

    if (return_value == NULL)
      strncpy (output, "", n);
    else
      strncpy (output, return_value, n);
}


//!
//! @brief Sets the string to the key in the preferences backend
//!
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void gw_pref_set_string (const char *key, const char* request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_string (client, key, request, NULL);
    g_object_unref (client);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_dictionary_source_gconf_key_changed_action (GConfClient* client,
                                                    guint cnxn_id,
                                                    GConfEntry* entry,
                                                    gpointer data       )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_STRING)
      gw_ui_set_dictionary_source (data, gconf_value_get_string(value));
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_toolbar_style_pref_changed_action (GConfClient* client, 
                                           guint        cnxn_id,
                                           GConfEntry*  entry,
                                           gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_STRING)
      gw_ui_set_toolbar_style (gconf_value_get_string(value));
    else
      gw_ui_set_toolbar_style ("both");
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_toolbar_show_pref_changed_action ( GConfClient* client,
                                           guint        cnxn_id,
                                           GConfEntry*  entry,
                                           gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_toolbar_show (gconf_value_get_bool(value));
    else
      gw_ui_set_toolbar_show(FALSE);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_use_global_document_font_pref_changed_action (GConfClient* client,
                                                      guint cnxn_id,
                                                      GConfEntry *entry,
                                                      gpointer data        )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_use_global_document_font_checkbox (gconf_value_get_bool(value));
    else
      gw_ui_set_use_global_document_font_checkbox (TRUE);

    gw_ui_set_font (NULL, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_global_document_font_pref_changed_action (GConfClient* client,
                                                  guint cnxn_id,
                                                  GConfEntry *entry,
                                                  gpointer data        )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_STRING)
    {
      gw_ui_update_global_font_label (gconf_value_get_string (value));
      gw_ui_set_font (NULL, NULL);
    }
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_custom_document_font_pref_changed_action (GConfClient* client,
                                                  guint cnxn_id,
                                                  GConfEntry *entry,
                                                  gpointer data        )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_STRING)
    {
      gw_ui_update_custom_font_button (gconf_value_get_string (value));
      gw_ui_set_font (NULL, NULL);
    }
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_font_magnification_pref_changed_action (GConfClient* client,
                                                guint cnxn_id,
                                                GConfEntry *entry,
                                                gpointer data        )
{
    //Get the size from the GCONF key
    int magnification;
    magnification = gconf_client_get_int (client, GCKEY_GW_FONT_MAGNIFICATION, NULL);

    //If the value is strange, get the default value, this function will get called again anvway
    if (magnification < GW_MIN_FONT_MAGNIFICATION | magnification > GW_MAX_FONT_MAGNIFICATION)
    {
      GConfValue *value;
      value = gconf_client_get_default_from_schema (client, GCKEY_GW_FONT_MAGNIFICATION, NULL);
      magnification = gconf_value_get_int (value);
      if (value != NULL && magnification >= GW_MIN_FONT_MAGNIFICATION && magnification <= GW_MAX_FONT_MAGNIFICATION)
      {
        gconf_client_set_int (client, GCKEY_GW_FONT_MAGNIFICATION, magnification, NULL);
      }
      else
        gconf_client_set_int (client, GCKEY_GW_FONT_MAGNIFICATION, 0, NULL);
    }
    //Set the font since the numbers seem to be sane
    else
    {
      gw_ui_set_font (NULL, &magnification);
      gw_ui_update_toolbar_buttons ();
    }
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_less_relevant_show_pref_changed_action ( GConfClient* client,
                                                 guint        cnxn_id,
                                                 GConfEntry*  entry,
                                                 gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_less_relevant_show (gconf_value_get_bool(value));
    else
      gw_ui_set_less_relevant_show(TRUE);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_roman_kana_conv_pref_changed_action (GConfClient* client, 
                                             guint        cnxn_id,
                                             GConfEntry*  entry,
                                             gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_INT)
    {
       int selection;
       selection = gconf_value_get_int(value);

       if (selection <= 2 && selection >= 0)
         gw_ui_set_romaji_kana_conv(selection);
       else
         gw_ui_set_romaji_kana_conv(2);
    }
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_hira_kata_conv_pref_changed_action (GConfClient* client, 
                                            guint        cnxn_id,
                                            GConfEntry*  entry,
                                            gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_hiragana_katakana_conv(gconf_value_get_bool(value));
    else
      gw_ui_set_hiragana_katakana_conv(TRUE);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_kata_hira_conv_pref_changed_action (GConfClient *client, 
                                            guint        cnxn_id,
                                            GConfEntry  *entry,
                                            gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_katakana_hiragana_conv(gconf_value_get_bool(value));
    else
      gw_ui_set_katakana_hiragana_conv(TRUE);
}


#ifdef WITH_LIBSEXY
//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_spellcheck_pref_changed_action (GConfClient* client, 
                                        guint        cnxn_id,
                                        GConfEntry*  entry,
                                        gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_sexy_ui_set_spellcheck (gconf_value_get_bool(value));
    else
      gw_sexy_ui_set_spellcheck (TRUE);
}
#endif


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_color_value_changed_action (GConfClient* client, 
                                    guint cnxn_id,
                                    GConfEntry *entry,
                                    gpointer data       )
{
    //Get the gconf value
    GConfValue *value;
    value = gconf_entry_get_value(entry);
    if (value != NULL &&  value->type == GCONF_VALUE_STRING)
    {
      char hex_color_string[100];
      strncpy (hex_color_string, gconf_value_get_string(value), 100);
      GdkColor color;
      char fallback_value[100];
      const char *pref_key = gconf_entry_get_key (entry);
       
      //If the format of the string is wrong, get the default one
      if (gdk_color_parse (hex_color_string, &color) == FALSE)
      {
        value = gconf_client_get_default_from_schema (client, pref_key, NULL);
        gw_util_strncpy_fallback_from_key (fallback_value, pref_key, 100);
        gw_pref_get_default_string (hex_color_string, pref_key, fallback_value, 100);
        if (value != NULL && value->type == GCONF_VALUE_STRING)
          strncpy (hex_color_string, gconf_value_get_string (value), 100);
        else
          return;
      }

      //Get the widget name which happens to be the same as part of the key
      const char *widget_name = strrchr(pref_key, '/'); widget_name++;

      //Finish up
      gw_ui_set_color_to_swatch (widget_name, hex_color_string);
      gw_ui_buffer_reload_tagtable_tags ();
    }
}


//!
//! @brief Adds a preference change listener for the selected key
//!
//! @param key The preference key
//! @param callback_function The function to call when the key changes
//! @param data The userdata to pass to the callback function
//!
void gw_prefs_add_change_listener (const char *key, void (*callback_function) (GConfClient*, guint, GConfEntry*, gpointer), gpointer data)
{
  //Get the gconf value

  GConfClient *client;
  client = gconf_client_get_default ();

  gconf_client_notify_add (client, key, callback_function, data, NULL, NULL);
  gconf_client_notify (client, key);
}


//!
//! @brief Initializes the preferences backend
//!
void gw_prefs_initialize_preferences()
{
  g_type_init();

  char *string;

  GConfClient *client;
  client = gconf_client_get_default ();
  
  //Add directory listeners gwaei will be using
  gconf_client_add_dir (client, GCPATH_INTERFACE, GCONF_CLIENT_PRELOAD_NONE, NULL);
  gconf_client_add_dir (client, GCPATH_GW, GCONF_CLIENT_PRELOAD_NONE, NULL);

  //Add preference change notifiers
  gw_prefs_add_change_listener (GCKEY_GW_LESS_RELEVANT_SHOW, do_less_relevant_show_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_TOOLBAR_STYLE, do_toolbar_style_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_TOOLBAR_SHOW, do_toolbar_show_pref_changed_action, NULL);

  gw_prefs_add_change_listener (GCKEY_GW_FONT_USE_GLOBAL_FONT, do_use_global_document_font_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_DOCUMENT_FONT_NAME, do_global_document_font_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_FONT_CUSTOM_FONT, do_custom_document_font_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_FONT_MAGNIFICATION, do_font_magnification_pref_changed_action, NULL);

  gw_prefs_add_change_listener (GCKEY_GW_ROMAN_KANA, do_roman_kana_conv_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_HIRA_KATA, do_hira_kata_conv_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_KATA_HIRA, do_kata_hira_conv_pref_changed_action, NULL);
#ifdef WITH_LIBSEXY
  gw_prefs_add_change_listener (GCKEY_GW_SPELLCHECK, do_spellcheck_pref_changed_action, NULL);
#endif
  gw_prefs_add_change_listener (GCKEY_GW_MATCH_FG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_MATCH_BG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_HEADER_FG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_HEADER_BG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_COMMENT_FG, do_color_value_changed_action, NULL);

  g_object_unref(client);
}








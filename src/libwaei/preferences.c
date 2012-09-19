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
//! @file preferences.c
//!


#include <string.h>
#include <stdlib.h>

#include <gio/gio.h>

#include <libwaei/libwaei.h>


//!
//! @brief Creates a new LwDictInfo object
//! @return An allocated LwDictInfo that will be needed to be freed by lw_preferences_free.
//!
LwPreferences* 
lw_preferences_new (GSettingsBackend *backend)
{
  LwPreferences *temp;

  temp = (LwPreferences*) malloc(sizeof(LwPreferences));

  if (temp != NULL)
  {
    lw_preferences_init (temp, backend);
  }

  return temp;
}


//!
//! @brief Releases a LwPreferences object from memory.
//! @param preferences A LwPreferences object created by lw_preferences_new.
//!
void 
lw_preferences_free (LwPreferences *preferences)
{
    lw_preferences_deinit (preferences);

    free (preferences);
}


//!
//! @brief Used to initialize the memory inside of a new LwPreferences
//!        object.  Usually lw_preferences_new calls this for you.  It is also 
//!        used in class implimentations that extends LwPreferences.
//! @param preferences The LwPreferences object to have it's inner memory initialized.
//!
void 
lw_preferences_init (LwPreferences *preferences, GSettingsBackend *backend)
{
    preferences->settingslist = NULL;
    preferences->backend = backend;
    g_mutex_init (&preferences->mutex);
}


//!
//! @brief Used to free the memory inside of a LwPreferences object.
//!         Usually lw_preferences_free calls this for you.  It is also used
//!         in class implimentations that extends LwPreferences.
//! @param preferences The LwPreferences object to have it's inner memory freed.
//!
void 
lw_preferences_deinit (LwPreferences *preferences)
{
    lw_preferences_free_settings (preferences);
    g_mutex_clear (&preferences->mutex); 
    if (preferences->backend != NULL) g_object_unref (preferences->backend); preferences->backend = NULL;
}


//!
//! @brief Frees the GSettings objects held by LwPreferences
//! @param preferences The instance of LwPreferences to free
//!
void 
lw_preferences_free_settings (LwPreferences *preferences)
{
    GList *iter;
    GSettings *settings;

    for (iter = preferences->settingslist; iter != NULL; iter = iter->next)
    {
      settings = (GSettings*) iter->data;
      g_object_unref (settings);
      iter->data = NULL;
    }

    g_list_free (preferences->settingslist);
    preferences->settingslist = NULL;

    if (preferences->backend != NULL)
    {
      g_object_unref (preferences->backend);
      preferences->backend = NULL;
    }
}


gboolean 
lw_preferences_schema_is_installed (const char *SCHEMA)
{
    //Declarations
    const gchar * const * iter;
    gboolean exists;

    //Initializations
    iter = g_settings_list_schemas ();

    while (iter != NULL && *iter != NULL && strcmp(*iter, SCHEMA) != 0)
      iter++;

    exists = (iter != NULL && *iter != NULL);

    if (!exists)
    {
      g_critical ("The GSettings schema \"%s\" isn't installed!  You must make "
                  "sure both gsettings-desktop-schemas from your package "
                  "manager and org.gnome.gwaei.gschema.xml are installed at "
                  "least locally if not globally. See the man page for "
                  "glib-compile-schemas for more information.\n", SCHEMA);
    }

    return exists;
}


//!
//! @brief Fetches a gsettings object by id, and stores it, using the cached one if available
//!
GSettings* 
lw_preferences_get_settings_object (LwPreferences *preferences, const char *SCHEMA)
{
    //Declarations
    GList *iter;
    char *schema;
    GSettings *settings;

    //Initializations
    settings = NULL;

    //Look for an already created gsetting object
    for (iter = preferences->settingslist; iter != NULL; iter = iter->next)
    {
      settings = (GSettings*) iter->data;
      g_object_get (G_OBJECT (settings), "schema", &schema, NULL);
      if (schema != NULL && strcmp(schema, SCHEMA) == 0)
        break;
      if (schema != NULL)
        g_free (schema);
      settings = NULL;
    }
    if (settings != NULL) 
    {
      g_free (schema);
    }
    else
    {
    }

    //If not found, create our own and add it to the list
    if (settings == NULL)
    {
      g_assert (lw_preferences_schema_is_installed (SCHEMA));

      if (preferences->backend == NULL)
        settings = g_settings_new (SCHEMA);
      else
        settings = g_settings_new_with_backend (SCHEMA, preferences->backend);
      if (settings != NULL)
      {
        preferences->settingslist = g_list_append (preferences->settingslist, settings);
      }
    }

    return settings;
}


//!
//! @brief Resets a value in a key
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param key A string identifying the key to reset
//!
void 
lw_preferences_reset_value (GSettings* settings, const char *key)
{
    g_settings_reset (settings, key);
}


//!
//! @brief Resets a value in a key
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param schema A string identifying the schema there the key is
//! @param key A string identifying the key to reset
//!
void 
lw_preferences_reset_value_by_schema (LwPreferences* preferences, const char* schema, const char *key)
{
    GSettings* settings;
    
    settings = lw_preferences_get_settings_object (preferences, schema);
    if (settings != NULL)
    {
      lw_preferences_reset_value (settings, key);
    }
}


//!
//! @brief Returns an integer from the preference backend 
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param key The key to use to look up the pref
//!
int 
lw_preferences_get_int (GSettings *settings, const char *key)
{
    return g_settings_get_int (settings, key);
}


//!
//! @brief Returns an integer from the preference backend 
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//!
int 
lw_preferences_get_int_by_schema (LwPreferences* preferences, const char* schema, const char *key)
{
    GSettings* settings;
    int value;
    
    value = 0;
    settings = lw_preferences_get_settings_object (preferences, schema);

    if (settings != NULL)
    {
      value = lw_preferences_get_int (settings, key);
    }

    return value;
}


//!
//! @brief Sets the int to the key in the preferences backend
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void 
lw_preferences_set_int (GSettings *settings, const char *key, const int request)
{
    g_settings_set_int (settings, key, request);
}


//!
//! @brief Sets the int to the key in the preferences backend
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param schema The schema to look for
//! @param key The key to use to look up the pref inside of the schema
//! @param request The value to set
//!
void 
lw_preferences_set_int_by_schema (LwPreferences* preferences, const char* schema, const char *key, const int request)
{
    GSettings* settings;
    
    settings = lw_preferences_get_settings_object (preferences, schema);
    if (settings != NULL)
    {
      lw_preferences_set_int (settings, key, request);
    }
}


//!
//! @brief Returns an boolean from the preference backend 
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param key The key to use to look up the pref
//!
gboolean 
lw_preferences_get_boolean (GSettings *settings, const char *key)
{
    return g_settings_get_boolean (settings, key);
}


//!
//! @brief Returns an boolean from the preference backend 
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//!
gboolean 
lw_preferences_get_boolean_by_schema (LwPreferences* preferences, const char* schema, const char *key)
{
    GSettings* settings;
    gboolean value;
    
    settings = lw_preferences_get_settings_object (preferences, schema);
    value = FALSE; 

    if (settings != NULL)
    {
      value = lw_preferences_get_boolean (settings, key);
    }

    return value;
}


//!
//! @brief Sets the boolean to the key in the preferences backend
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void 
lw_preferences_set_boolean (GSettings *settings, const char *key, const gboolean request)
{
    g_settings_set_boolean (settings, key, request);
}


//!
//! @brief Sets the boolean to the key in the preferences backend
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param schema The schema to look for
//! @param key The key to use to look up the pref inside of the schema
//! @param request The value to set
//!
void 
lw_preferences_set_boolean_by_schema (LwPreferences* preferences, const char* schema, const char *key, const gboolean request)
{
    GSettings* settings;
    
    settings = lw_preferences_get_settings_object (preferences, schema);

    if (settings != NULL)
    {
      lw_preferences_set_boolean (settings, key, request);
    }
}


//!
//! @brief Returns an string from the preference backend 
//! @param output string to copy the pref to
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param key The key to use to look up the pref
//! @param n The max characters to copy to output
//!
void 
lw_preferences_get_string (char *output, GSettings *settings, const char *key, const int n)
{
    gchar *value = NULL; 

    value = g_settings_get_string (settings, key);
    g_assert (value != NULL);
    strncpy(output, value, n);
    output[n - 1] = '\0';

    g_free (value);
    value = NULL;
}


//!
//! @brief Returns an string from the preference backend 
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param output string to copy the pref to
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param n The max characters to copy to output
//!
void 
lw_preferences_get_string_by_schema (LwPreferences* preferences, char *output, const char *schema, const char *key, const int n)
{
    GSettings* settings;
    
    settings = lw_preferences_get_settings_object (preferences, schema);

    lw_preferences_get_string (output, settings, key, n);
}


//!
//! @brief Sets the string to the key in the preferences backend
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void 
lw_preferences_set_string (GSettings *settings, const char *key, const char* request)
{
    g_settings_set_string (settings, key, request);
}


//!
//! @brief Sets the string to the key in the preferences backend
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param schema The schema to look for
//! @param key The key to use to look up the pref inside of the schema
//! @param request The value to set
//!
void 
lw_preferences_set_string_by_schema (LwPreferences *preferences, const char* schema, const char *key, const char* request)
{
    GSettings *settings;

    settings = lw_preferences_get_settings_object (preferences, schema);

    if (settings != NULL)
    {
      lw_preferences_set_string (settings, key, request);
    }
}


//!
//! @brief Adds a preference change listener for the selected key
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param key The preference key
//! @param callback_function The function to call when the key changes
//! @param data The userdata to pass to the callback function
//! @returns A gulong used to remove a signal later if desired
//!
gulong 
lw_preferences_add_change_listener (GSettings *settings, const char *key, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer data)
{
    g_assert (key != NULL);

    //Declarations
    char *signal_name;
    gulong id;
    GVariant *value;

    //Set up the signal
    signal_name = g_strdup_printf ("changed::%s", key);
    id = g_signal_connect (G_OBJECT (settings), signal_name, G_CALLBACK (callback_function), data);

    //Trigger an initial fire of the change listener
    value = g_settings_get_value (settings, key);
    if (value != NULL) g_settings_set_value (settings, key, value);

    //Cleanup
    g_variant_unref (value);
    value = NULL;
    g_free (signal_name);
    signal_name = NULL;

    return id;
}


//!
//! @brief Adds a preference change listener for the selected key
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param schema The key to use to look up the pref
//! @param key The preference key
//! @param callback_function The function to call when the key changes
//! @param data The userdata to pass to the callback function
//! @returns A gulong used to remove a signal later if desired
//!
gulong 
lw_preferences_add_change_listener_by_schema (LwPreferences *preferences, const char* schema, const char *key, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer data)
{
    g_assert (schema != NULL && key != NULL);

    GSettings *settings;
    gulong id;

    settings = lw_preferences_get_settings_object (preferences, schema);
    id = lw_preferences_add_change_listener (settings, key, callback_function, data);


    return id;
}


//!
//! @brief Used to remove a listener
//! @param settings The GSettings object to act on You will have to get it yourself using lw_preferences_get_settings_object
//! @param id The signalid returned by lw_preferences_add_change_listener
//!
void 
lw_preferences_remove_change_listener (GSettings *settings, gulong id)
{
    if (g_signal_handler_is_connected (G_OBJECT (settings), id))
    {
      g_signal_handler_disconnect (G_OBJECT (settings), id);
    }
    else
    {
    }
}


//!
//! @brief Used to remove a listener
//! @param preferences The LwPreferences to fetch the GSettings object matching the schema
//! @param schema A schema of the GSettings object the signal was connected to
//! @param id The signalid returned by lw_preferences_add_change_listener
//!
void 
lw_preferences_remove_change_listener_by_schema (LwPreferences *preferences, const char* schema, gulong id)
{
    GSettings *settings;

    settings = lw_preferences_get_settings_object (preferences, schema);

    if (settings != NULL)
    {
      lw_preferences_remove_change_listener (settings, id);
    }
    else
    {
    }
}







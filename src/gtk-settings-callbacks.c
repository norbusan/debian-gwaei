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

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/preferences.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-settings-callbacks.h>
#include <gwaei/gtk-main-interface.h>
#include <gwaei/gtk-settings-interface.h>
#include <gwaei/gtk-settings-interface-install-line.h>


static int removed_from = -1;
static int added_to = -1;
static int rebuilding_order_list_processes = 0;


//!
//! @brief Temporary function used to fix failure of closures for blocking the dictionary order functions
//!
void gw_settings_increment_order_list_processes ()
{
  rebuilding_order_list_processes++;
}


//!
//! @brief Temporary function used to fix failure of closures for blocking the dictionary order functions
//!
void gw_settings_decrement_order_list_processes ()
{
  rebuilding_order_list_processes--;
  if (rebuilding_order_list_processes < 0)
    rebuilding_order_list_processes = 0;
}



//!
//! @brief The thread used while rsync is running, updating the dictionaries
//!
//! @param nothing A void pionter that currently does nothing
//!
/*
static void *update_thread (void *nothing)
{
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    GError *error = NULL;

    GwDictInfo* kanji;
    kanji = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_KANJI);
    GwDictInfo* names;
    names = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_NAMES);
    GwDictInfo* radicals;
    radicals = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_RADICALS);

    //Find out how many dictionaries need updating
    gdouble total_dictionary_updates = 0.0;
    gdouble extra_processing_jobs = 0.0;

    GList *dictionarylist = gw_dictlist_get_list();
    GList *updatelist = NULL;
    GwDictInfo* di;

    while (dictionarylist != NULL)
    {
      di = (GwDictInfo*)dictionarylist->data;
      if (di->status == GW_DICT_STATUS_INSTALLED && strlen(di->rsync) > 1)
      {
        updatelist = g_list_append (updatelist, di);
        di->status = GW_DICT_STATUS_UPDATING;
        total_dictionary_updates++;

        if (di->id == GW_DICT_ID_KANJI && radicals->status == GW_DICT_STATUS_INSTALLED)
          extra_processing_jobs++;
        else if (di->id == GW_DICT_ID_NAMES)
          extra_processing_jobs++;
      }
      dictionarylist = dictionarylist->next;
    }

    gdouble dividend;
    dividend = 1.0;
    gdouble divisor;
    divisor = (total_dictionary_updates * 2.0) + extra_processing_jobs;
    gdouble error_deviation;
    error_deviation = 0.0001;
    gdouble increment;
    increment = (dividend / divisor) - error_deviation;
    gdouble progress = 0.0;
    gchar *temp_text = NULL;
    char *path = NULL;
    char *sync_path = NULL;

    gdk_threads_enter();
    //gw_ui_set_install_line_status("update",  "cancel",  NULL);
    gw_ui_update_settings_interface ();
    gdk_threads_leave ();

    while (updatelist != NULL && gw_ui_get_install_line_status("update") != GW_DICT_STATUS_CANCELING)
    {
      di = (GwDictInfo*) updatelist->data;
      path = di->path;
      sync_path = di->sync_path;

      if (error == NULL)
      {
        gdk_threads_enter ();
        // TRANSLATORS: The "%s" stands for the long name of the dictionary (like "English Dictionary" or "Kanji Dictionary")
        temp_text = g_strdup_printf (gettext("Syncing %s with server..."), di->long_name);
        gw_ui_set_progressbar ("update", progress, temp_text);
        g_free (temp_text);
        temp_text = NULL;
        gdk_threads_leave ();

        if (system (di->rsync) != 0)
        {
          error = g_error_new_literal (quark, GW_FILE_ERROR, "Connection failure");
        }
      }
      progress += increment;
      
      if (error == NULL)
      {
        gdk_threads_enter ();
        // TRANSLATORS: The "%s" stands for the long name of the dictionary (like "English Dictionary" or "Kanji Dictionary")
        temp_text = g_strdup_printf (gettext("Finalizing %s changes..."), di->long_name);
        gw_ui_set_progressbar ("update", progress, temp_text);
        g_free (temp_text);
        temp_text = NULL;
        gdk_threads_leave ();
        gw_io_copy_with_encoding (sync_path, path, "EUC-JP","UTF-8", &error);
      }
      progress += increment;

      //Special dictionary post processing
      if (error == NULL)
      {
        if (di->id == GW_DICT_ID_KANJI && 
            gw_dictlist_dictionary_get_status_by_id(GW_DICT_ID_RADICALS) == GW_DICT_STATUS_INSTALLED)
        {
          gdk_threads_enter ();
          temp_text = g_strdup_printf (gettext("Recreating Mixed dictionary..."));
          gw_ui_set_progressbar ("update", progress, temp_text);
          g_free (temp_text);
          temp_text = NULL;
          gdk_threads_leave ();
          gw_dictlist_preform_postprocessing_by_name (di->name, &error);
          progress += increment;
        }
        else if (di->id == GW_DICT_ID_NAMES && error == NULL)
        {
          gdk_threads_enter ();
          temp_text = g_strdup_printf (gettext("Resplitting Names dictionary..."));
          gw_ui_set_progressbar ("update", progress, temp_text);
          g_free (temp_text);
          temp_text = NULL;
          gw_dictlist_preform_postprocessing_by_name (di->name, &error);
          gdk_threads_leave ();
          progress += increment;
        }
      }

      gdk_threads_enter ();
      if (error == NULL)
        di->status = GW_DICT_STATUS_UPDATED;
      else
        di->status = GW_DICT_STATUS_ERRORED;
      gdk_threads_leave ();

      updatelist = updatelist->next;
    }
    g_list_free (updatelist);

    progress = 1.0;
    gdk_threads_enter ();
    gw_ui_set_progressbar ("update", progress, gettext("Finishing..."));
    gdk_threads_leave ();

    gdk_threads_enter();
    if(error != NULL)
    {
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_ERRORED, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATED, GW_DICT_STATUS_INSTALLED);
      //gw_ui_set_install_line_status ("update",  "error", error->message);
      g_error_free(error);
      error = NULL;
    }
    else if (gw_ui_get_install_line_status ("update") == GW_DICT_STATUS_CANCELING)
    {
      temp_text = g_strdup_printf (gettext("Update was cancelled"));
      //gw_ui_set_install_line_status ("update",  "install", text);
      g_free (temp_text);
      temp_text = NULL;
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATED, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_CANCELING, GW_DICT_STATUS_INSTALLED);
    }
    else
    {
      temp_text = g_strdup_printf (gettext("Dictionary update finished"));
      //gw_ui_set_install_line_status ("update", "remove", text);
      g_free (temp_text);
      temp_text = NULL;
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATED, GW_DICT_STATUS_INSTALLED);
    }

    gw_ui_update_settings_interface ();
    gdk_threads_leave ();
}
*/


//!
//! @brief GUI install thread for a dictionary
//!
//! @param dictionary A gpointer to a dictionary
//!
static void *install_thread (gpointer data)
{
    //Preparations
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    GError *error = NULL;

    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    GwDictInfo *di = (GwDictInfo*) il->di;

    if (di->status != GW_DICT_STATUS_NOT_INSTALLED) return FALSE;

    //Preparatation complete, it's showtime
    gdk_threads_enter ();
    di->status = GW_DICT_STATUS_INSTALLING;
    gw_ui_dict_install_set_action_button (il, GTK_STOCK_CANCEL, TRUE);
    gw_ui_update_settings_interface();
    di->status = GW_DICT_STATUS_NOT_INSTALLED;
    gdk_threads_leave ();


    //Offload the install work to the io function
    gw_io_install_dictinfo (di, &gw_ui_update_progressbar, data, FALSE, &error);


    //Finish up
    if (di->status == GW_DICT_STATUS_CANCELING)
    {
      gdk_threads_enter();
        di->status = GW_DICT_STATUS_NOT_INSTALLED;
      gdk_threads_leave();
      g_error_free(error);
      error = NULL;
      do_dictionary_remove (il->action_button, il);
    }
    //Errored
    else if (error != NULL)
    {
      do_dictionary_remove (il->action_button, il);
      gdk_threads_enter();
        di->status = GW_DICT_STATUS_NOT_INSTALLED;
        gw_ui_dict_install_set_message (il, GTK_STOCK_DIALOG_ERROR, error->message);
        gw_ui_dict_install_set_action_button (il, GTK_STOCK_ADD, TRUE);
      gdk_threads_leave();
      g_error_free (error);
      error = NULL;
    }
    //Install was successful
    else
    {
      gdk_threads_enter();
        di->status = GW_DICT_STATUS_INSTALLED;
        di->total_lines =  gw_io_get_total_lines_for_path (di->path);
      gdk_threads_leave();
      //If statement to reduce flicker, between the Kanji/Radical dictionary install
      if (di->id != GW_DICT_ID_KANJI)
      {
        gdk_threads_enter();
          gw_ui_dict_install_set_action_button (il, GTK_STOCK_DELETE, TRUE);
          gw_ui_dict_install_set_message (il, GTK_STOCK_APPLY, gettext("Installed"));
        gdk_threads_leave();
      }

      //Special case where the Radicals dictionary is installed right after the Kanji one.
      if (di->id == GW_DICT_ID_KANJI)
      {
        gdk_threads_enter();
          gw_ui_dict_install_set_message (il, NULL, gettext("Installing..."));
        gdk_threads_leave();
        GwDictInfo *radicals_dict = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_RADICALS);
        GwDictInfo *kanji_dict = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_KANJI);
        il->di = radicals_dict;
        install_thread (data);
        il->di = kanji_dict;
      }
    }

    gdk_threads_enter();
      gw_ui_update_settings_interface();
      rebuild_combobox_dictionary_list();
    gdk_threads_leave();
}


//!
//! @brief Callback to toggle the hiragana-katakana conversion setting for the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_hiragana_katakana_conv_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_HIRA_KATA, TRUE);
    gw_pref_set_boolean (GCKEY_GW_HIRA_KATA, !state);
}


//!
//! @brief Callback to toggle the katakana-hiragana conversion setting for the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_katakana_hiragana_conv_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_KATA_HIRA, TRUE);
    gw_pref_set_boolean (GCKEY_GW_KATA_HIRA, !state);
}


//!
//! @brief Callback to toggle spellcheck in the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_spellcheck_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_SPELLCHECK, TRUE);
    gw_pref_set_boolean (GCKEY_GW_SPELLCHECK, !state);
}


//!
//! @brief Callback to toggle romaji-kana conversion
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_romaji_kana_conv_change (GtkWidget *widget, gpointer data)
{
    int active;
    active = gtk_combo_box_get_active(GTK_COMBO_BOX (widget));
    gw_pref_set_int (GCKEY_GW_ROMAN_KANA, active);
}


//!
//! @brief Callback to set the user selected color to the color swatch for text highlighting
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_set_color_to_swatch (GtkWidget *widget, gpointer data)
{
    //Initializations
    GdkColor color;
    gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &color);
    char *hex_color_string = NULL;
    hex_color_string = gdk_color_to_string (&color);
    char *pref_key = NULL;
    pref_key = g_strdup_printf (GCPATH_GW_HIGHLIGHT "%s", gtk_buildable_get_name (GTK_BUILDABLE (widget)));

    //Set the color inthe prefs
    if (pref_key != NULL && hex_color_string != NULL)
    {
      gw_pref_set_string (pref_key, hex_color_string);
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
G_MODULE_EXPORT void do_color_reset_for_swatches (GtkWidget *widget, gpointer data)
{
    //Initializations
    char fallback[100];
    int i = 0;
    char *pref_key[] = {
      GCKEY_GW_MATCH_FG,
      GCKEY_GW_MATCH_BG,
      GCKEY_GW_HEADER_FG,
      GCKEY_GW_HEADER_BG,
      GCKEY_GW_COMMENT_FG,
      NULL
    };

    char default_hex_color_string[100];

    //Start setting the default values
    for (i = 0; pref_key[i] != NULL; i++)
    {
      gw_util_strncpy_fallback_from_key (fallback, pref_key[i], 100);
      gw_pref_get_default_string (default_hex_color_string, pref_key[i], fallback, 100);
      if (default_hex_color_string != NULL)
        gw_pref_set_string (pref_key[i], default_hex_color_string);
    }

    gw_ui_buffer_reload_tagtable_tags();
}


//!
//! @brief Callback to cancel the installion of the selected dictionary
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Used as a gpointer to a GwUiDictInstallLine to get needed uninstall information from
//!
G_MODULE_EXPORT void do_cancel_dictionary_install (GtkWidget *widget, gpointer data)
{
    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    gw_ui_dict_install_set_action_button (il, GTK_STOCK_CANCEL, FALSE);
    gw_ui_dict_install_set_message (il, NULL, gettext ("Cancelling..."));
    il->di->status = GW_DICT_STATUS_CANCELING;
}


//!
//! @brief Callback to uninstall the selected dictionary
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Used as a gpointer to a GwUiDictInstallLine to get needed uninstall information from
//!
G_MODULE_EXPORT void do_dictionary_remove (GtkWidget* widget, gpointer data)
{
    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    gw_io_uninstall_dictinfo (il->di, NULL, NULL, TRUE);
    gw_ui_dict_install_set_action_button (il, GTK_STOCK_ADD, TRUE);
    gw_ui_dict_install_set_message (il, NULL, gettext ("Not Installed"));
    rebuild_combobox_dictionary_list ();
    gw_ui_update_settings_interface ();
}


//!
//! @brief Callback to install the selected dictionary
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Used as a gpointer to a GwUiDictInstallLine to get needed uninstall information from
//!
G_MODULE_EXPORT void do_dictionary_install (GtkWidget *widget, gpointer data)
{
    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;

    //Make sure the files are clean
    do_dictionary_remove (widget, data);

    gw_ui_dict_install_set_action_button (il, GTK_STOCK_CANCEL, TRUE);
    gw_ui_dict_install_set_message (il, NULL, gettext ("Installing..."));

    //Create the thread
    if (g_thread_create(&install_thread, il, FALSE, NULL) == NULL) {
      g_warning("couldn't create the thread");
      return;
    }
}


//!
//! @brief Updates the gconf entry for the dictionary orders
//!
//! @param widget Pointer to the GtkWidget containing the source URI to copy
//! @param data il A pointer to a GwUiDictInstallLine to use to get the gconf key from
//!
G_MODULE_EXPORT void do_remove_dictionary_from_order_prefs (GtkTreeModel *tree_model,
                                                            GtkTreePath *path,
                                                            gpointer    *data        )
{
    if (rebuilding_order_list_processes != 0) return;

    printf("removing dictionry from order prefs\n");
    int *indices = gtk_tree_path_get_indices (path);
    removed_from = indices[0];

    if (added_to == -1) return;


    //Parse the string
    char order[5000];
    gw_pref_get_string (order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    char *long_name_list[50];
    char **condensed_name_list[50];
    long_name_list[0] = order;
    int i = 0;
    int j = 0;
    while ((long_name_list[i + 1] = g_utf8_strchr (long_name_list[i], -1, L',')) && i < 50)
    {
      i++;
      *long_name_list[i] = '\0';
      long_name_list[i]++;
    }
    long_name_list[i + 1] = NULL;

    i = 0;
    j = 0;
    GwDictInfo *di1, *di2;
    while (long_name_list[i] != NULL && long_name_list[j] != NULL)
    {
      di1 = gw_dictlist_get_dictinfo_by_name (long_name_list[j]);
      di2 = gw_dictlist_get_dictinfo_by_alias (long_name_list[j]);
      if (strcmp(di1->name, di2->name) == 0 && di2->status == GW_DICT_STATUS_INSTALLED)
      {
        condensed_name_list[i] = &long_name_list[j];
        i++; j++;
      }
      else
      {
        j++;
      }
        
    }
    condensed_name_list[i] = NULL;

    int long_total = j;
    int short_total = i;


    //Pull the switcheroo
    int increment_direction = 0;
    if (removed_from < added_to)
    {
      increment_direction = 1;
      added_to--;

    }
    else
    {
      increment_direction = -1;
      removed_from--;
    }

    while (removed_from != added_to)
    {
        char *temp;
        temp = *condensed_name_list[removed_from];
        *condensed_name_list[removed_from] = *condensed_name_list[removed_from + increment_direction];
        *condensed_name_list[removed_from + increment_direction] = temp;
        removed_from = removed_from + increment_direction;
    }


    i = 0;
    char output[5000];
    output[0] = '\0';
    while (long_name_list[i] != NULL)
    {
      strcat (output, long_name_list[i]);
      strcat (output, ",");
      i++;
    }
    output[strlen(output) - 1] = '\0';
    gw_pref_set_string (GCKEY_GW_LOAD_ORDER, output);
    added_to == -1;

    gw_ui_update_dictionary_order_list ();
    gw_ui_update_settings_interface();
}

//!
//! @brief Updates the gconf entry for the dictionary orders
//!
//! @param widget Pointer to the GtkWidget containing the source URI to copy
//! @param data il A pointer to a GwUiDictInstallLine to use to get the gconf key from
//!
G_MODULE_EXPORT void do_add_dictionary_to_order_prefs (GtkTreeModel *tree_model,
                                                       GtkTreePath *path,
                                                       GtkTreeIter *iter,
                                                       gpointer    *data        )
{
    if (rebuilding_order_list_processes != 0) return;

    int *indices = gtk_tree_path_get_indices (path);
    added_to = indices[0];
}

//!
//! @brief Updates the gconf source entry for the dictionary when the user types into the text entry
//!
//! @param widget Pointer to the GtkWidget containing the source URI to copy
//! @param data il A pointer to a GwUiDictInstallLine to use to get the gconf key from
//!
G_MODULE_EXPORT void do_source_entry_changed_action (GtkWidget *widget, gpointer data)
{
    if (widget != NULL && data != NULL)
    {
      GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
      char value[FILENAME_MAX];
      strcpy(value, gtk_entry_get_text(GTK_ENTRY (widget)));
      gw_pref_set_string (il->di->gckey, value);
    }
}


//!
//! @brief gtk callback that resets the dictionary install url to the default
//!
//! @param widget Unused GtkWidget pointer 
//! @param data il A pointer to a GwUiDictInstallLine to use to get the gconf key from
//!
G_MODULE_EXPORT void do_dictionary_source_reset (GtkWidget *widget, gpointer data)
{
    if (widget != NULL && data != NULL)
    {
      GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
      char source_string[100];
      char fallback[100];
      gw_util_strncpy_fallback_from_key (fallback, il->di->gckey, 100);
      gw_pref_get_default_string (source_string, il->di->gckey, fallback, 100);
      gw_pref_set_string (il->di->gckey, source_string);
    }
}


//!
//! @brief gtk callback that lets the user select an install uri
//!
//! @param widget Unused GtkWidget pointer 
//! @param data il A pointer to a GwUiDictInstallLine to use to get the correct uri source entry from
//!
G_MODULE_EXPORT void do_dictionary_source_browse (GtkWidget *widget, gpointer data)
{
    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    //Declarations
    GtkWidget *dialog, *window;
    window = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));
    dialog = gtk_file_chooser_dialog_new (gettext("Dictionary File Select"),
                GTK_WINDOW (window),
                GTK_FILE_CHOOSER_ACTION_OPEN,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                NULL);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "");

    //Run the open as dialog
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        gw_ui_set_dictionary_source(il->source_uri_entry, filename);

        g_free (filename);
        filename = NULL;
    }

    gtk_widget_destroy (dialog);
}


//!
//! @brief Callback to use rsync to update the installed dictionaries
//!
//! This function is currently unused until a rewrite
//
//! @param widget Unused pointer to a GtkWidget
//! @param data Used as a gpointer to a GwUiDictInstallLine to get needed uninstall information from
//!
G_MODULE_EXPORT void do_update_installed_dictionaries(GtkWidget *widget, gpointer data)
{
  /*
    char name[100];
    gw_parse_widget_name(name, widget, TRUE);

    GwDictInfo *dictionary;
    dictionary = gw_dictlist_get_dictinfo_by_alias (name);

    //Create the thread
    if (g_thread_create(&update_thread, NULL, FALSE, NULL) == NULL) {
      g_warning("couldn't create the thread");
      return;
    }
    */
}


//!
//! @brief Callback to cancel the update of the installed dictionaries
//!
//! This function is curretly unused until a rewrite
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Used as a gpointer to a GwUiDictInstallLine to get needed uninstall information from
//!
G_MODULE_EXPORT void do_cancel_update_installed_dictionaries (GtkWidget *widget, gpointer data)
{
    //gw_ui_set_install_line_status("update", "cancelling", NULL);
}


//!
//! @brief Callback for the button that allows users to manually resplit the Names from the Places dictionary
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data A currently unused gpointer
//!
G_MODULE_EXPORT void do_force_names_resplit (GtkWidget *widget, gpointer data)
{
    GError *error = NULL;

    GwDictInfo* di;
    di = gw_dictlist_get_dictinfo_by_alias("Names");

    di->status = GW_DICT_STATUS_REBUILDING;
    gw_dictlist_preform_postprocessing_by_name("Names", &error);
    di->status = GW_DICT_STATUS_INSTALLED;

    if (error != NULL)
    {
      g_error_free (error);
    }
}


//!
//! @brief Callback for the button that allows users to manually force a rebuild of the "Mix" dictionary.
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data A currently unused gpointer
//!
G_MODULE_EXPORT void do_force_mix_rebuild (GtkWidget *widget, gpointer data)
{
    GError *error = NULL;

    GwDictInfo* di;
    di = gw_dictlist_get_dictinfo_by_alias("Mix");

    if (gw_dictlist_dictionary_get_status_by_id (GW_DICT_ID_KANJI)    == GW_DICT_STATUS_INSTALLED &&
        gw_dictlist_dictionary_get_status_by_id (GW_DICT_ID_RADICALS) == GW_DICT_STATUS_INSTALLED   )
    {
      di->status = GW_DICT_STATUS_REBUILDING;
      gw_dictlist_preform_postprocessing_by_name("Mix", &error);
      di->status = GW_DICT_STATUS_INSTALLED;
    }
    if (error != NULL)
    {
      g_error_free (error);
    }
}


//!
//! @brief Callback to switch the dictionary install table shown in the preferences
//!
//! @param widget The GtkComboBox where the user selects the dictionary table the wish to you
//! @param data A currently unused gpointer
//!
G_MODULE_EXPORT void do_switch_displayed_dictionaries_action (GtkWidget *widget, gpointer data)
{
    int active;
    GtkWidget *main_dictionaries_table, *other_dictionaries_table;

    active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    main_dictionaries_table = GTK_WIDGET (gtk_builder_get_object (builder, "dictionaries_table"));
    other_dictionaries_table = GTK_WIDGET (gtk_builder_get_object (builder, "other_dictionaries_table"));

    gtk_widget_hide (other_dictionaries_table);
    gtk_widget_hide (main_dictionaries_table);

    if (active == 0)
       gtk_widget_show (main_dictionaries_table);
    else if (active == 1)
       gtk_widget_show (other_dictionaries_table);
}


//!
//! @brief Callback for to move the current dictionary higher in the list order
//!
//! @param widget A currently unused GtkWidget pointer
//! @param data A pointer to an INT where the dictionary should be moved
//!
G_MODULE_EXPORT void do_move_dictionary_up (GtkWidget *widget, gpointer data)
{
    printf("Moving up...\n");
    GtkTreeIter start_iter, end_iter;
    int *indices = NULL;
    int move_from_here = -1;
    int move_to_here = -1;

    GtkTreeView *treeview = GTK_TREE_VIEW (data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    if (!gtk_tree_selection_get_selected (selection, &model, &start_iter)) return;
    GtkTreePath *path = gtk_tree_model_get_path (model, &start_iter);
    indices = gtk_tree_path_get_indices (path);
    move_from_here = indices[0];
    if (!gtk_tree_path_prev (path)) return;
    indices = gtk_tree_path_get_indices (path);
    move_to_here = indices[0];
    gtk_tree_model_get_iter (model, &end_iter, path);
    gtk_list_store_move_before (GTK_LIST_STORE (model), &start_iter, &end_iter);
    gtk_tree_path_free (path);

    //Parse the string
    char order[5000];
    gw_pref_get_string (order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    char *long_name_list[50];
    char **condensed_name_list[50];
    long_name_list[0] = order;
    int i = 0;
    int j = 0;
    while ((long_name_list[i + 1] = g_utf8_strchr (long_name_list[i], -1, L',')) && i < 50)
    {
      i++;
      *long_name_list[i] = '\0';
      long_name_list[i]++;
    }
    long_name_list[i + 1] = NULL;

    i = 0;
    j = 0;
    GwDictInfo *di1, *di2;
    while (long_name_list[i] != NULL && long_name_list[j] != NULL)
    {
      di1 = gw_dictlist_get_dictinfo_by_name (long_name_list[j]);
      di2 = gw_dictlist_get_dictinfo_by_alias (long_name_list[j]);
      if (strcmp(di1->name, di2->name) == 0 && di2->status == GW_DICT_STATUS_INSTALLED)
      {
        condensed_name_list[i] = &long_name_list[j];
        i++; j++;
      }
      else
      {
        j++;
      }
        
    }
    condensed_name_list[i] = NULL;

    int long_total = j;
    int short_total = i;


    //Pull the switcheroo
    char *temp;
    temp = *condensed_name_list[move_from_here];
    *condensed_name_list[move_from_here] = *condensed_name_list[move_to_here];
    *condensed_name_list[move_to_here] = temp;

    i = 0;
    char output[5000];
    output[0] = '\0';
    while (long_name_list[i] != NULL)
    {
      strcat (output, long_name_list[i]);
      strcat (output, ",");
      i++;
    }
    output[strlen(output) - 1] = '\0';
    gw_pref_set_string (GCKEY_GW_LOAD_ORDER, output);

    gw_ui_update_dictionary_order_list ();
    gw_ui_update_settings_interface ();
}


//!
//! @brief Callback for to move the current dictionary lower in the list order
//!
//! @param widget A currently unused GtkWidget pointer
//! @param data A pointer to an INT where the dictionary should be moved
//!
G_MODULE_EXPORT void do_move_dictionary_down (GtkWidget *widget, gpointer data)
{
    printf("Moving down...\n");
    GtkTreeIter start_iter, end_iter;
    int *indices = NULL;
    int move_from_here = -1;
    int move_to_here = -1;

    GtkTreeView *treeview = GTK_TREE_VIEW (data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    if (!gtk_tree_selection_get_selected (selection, &model, &start_iter)) return;
    GtkTreePath *path = gtk_tree_model_get_path (model, &start_iter);
    indices = gtk_tree_path_get_indices (path);
    move_from_here = indices[0];
    gtk_tree_path_next (path);
    indices = gtk_tree_path_get_indices (path);
    move_to_here = indices[0];
    gtk_tree_model_get_iter (model, &end_iter, path);
    gtk_tree_path_free (path);

    if (move_from_here == move_to_here) return;

    //Parse the string
    char order[5000];
    gw_pref_get_string (order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    char *long_name_list[50];
    char **condensed_name_list[50];
    long_name_list[0] = order;
    int i = 0;
    int j = 0;
    while ((long_name_list[i + 1] = g_utf8_strchr (long_name_list[i], -1, L',')) && i < 50)
    {
      i++;
      *long_name_list[i] = '\0';
      long_name_list[i]++;
    }
    long_name_list[i + 1] = NULL;

    i = 0;
    j = 0;
    GwDictInfo *di1, *di2;
    while (long_name_list[i] != NULL && long_name_list[j] != NULL)
    {
      di1 = gw_dictlist_get_dictinfo_by_name (long_name_list[j]);
      di2 = gw_dictlist_get_dictinfo_by_alias (long_name_list[j]);
      if (strcmp(di1->name, di2->name) == 0 && di2->status == GW_DICT_STATUS_INSTALLED)
      {
        condensed_name_list[i] = &long_name_list[j];
        i++; j++;
      }
      else
      {
        j++;
      }
        
    }
    condensed_name_list[i] = NULL;

    int long_total = j;
    int short_total = i;

    if (move_to_here == short_total) return;
    gtk_list_store_move_after (GTK_LIST_STORE (model), &start_iter, &end_iter);

    //Pull the switcheroo
    char *temp;
    temp = *condensed_name_list[move_from_here];
    *condensed_name_list[move_from_here] = *condensed_name_list[move_to_here];
    *condensed_name_list[move_to_here] = temp;

    i = 0;
    char output[5000];
    output[0] = '\0';
    while (long_name_list[i] != NULL)
    {
      strcat (output, long_name_list[i]);
      strcat (output, ",");
      i++;
    }
    output[strlen(output) - 1] = '\0';
    gw_pref_set_string (GCKEY_GW_LOAD_ORDER, output);

    gw_ui_update_dictionary_order_list ();
    gw_ui_update_settings_interface ();
}


G_MODULE_EXPORT void do_reset_dictionary_orders (GtkWidget *widget, gpointer data)
{
    char dictionary_orders[5000];
    gw_pref_get_default_string (dictionary_orders, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    gw_pref_set_string (GCKEY_GW_LOAD_ORDER, dictionary_orders);
    rebuild_combobox_dictionary_list();
    gw_ui_update_settings_interface();
}


//!
//! @brief Opens and closes the advanced toggle button for the dictionary install screen
//!
//! @param widget A pointer to the GtkExpander widget that was toggled
//! @param data An unused gpointer
//!
G_MODULE_EXPORT void do_toggle_advanced_source (GtkWidget *widget, gpointer data)
{
    gboolean expanded;
    expanded = gtk_expander_get_expanded(GTK_EXPANDER (widget));
    
    if (expanded == TRUE)
      gtk_widget_hide (GTK_WIDGET (data));
    else
      gtk_widget_show_all (GTK_WIDGET (data));
}


//!
//! @brief Sets the preference key for the global font usage
//! 
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_toggle_use_global_document_font (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_FONT_USE_GLOBAL_FONT, TRUE);
    gw_pref_set_boolean (GCKEY_GW_FONT_USE_GLOBAL_FONT, !state);
}


//!
//! @brief Sets the preference key for the new custom document font
//! 
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_set_custom_document_font (GtkWidget *widget, gpointer data)
{
    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "custom_font_fontbutton"));
    const char *font_description_string = gtk_font_button_get_font_name (GTK_FONT_BUTTON (button));
    gw_pref_set_string (GCKEY_GW_FONT_CUSTOM_FONT, font_description_string);
}



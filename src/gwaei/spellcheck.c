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
//! @file spellcheck.c
//!
//! @brief To be written
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <hunspell/hunspell.h>

#include <gwaei/gwaei.h>
#include <gwaei/gettext.h>
#include <gwaei/spellcheck-private.h>

static void gw_spellcheck_attach_signals (GwSpellcheck*);
static void gw_spellcheck_remove_signals (GwSpellcheck*);

G_DEFINE_TYPE (GwSpellcheck, gw_spellcheck, G_TYPE_OBJECT)


typedef enum
{
  PROP_0,
  PROP_APPLICATION
} GwSpellcheckProps;


GwSpellcheck*
gw_spellcheck_new (GwApplication *application)
{
    GwSpellcheck *spellcheck;

    spellcheck = GW_SPELLCHECK (g_object_new (GW_TYPE_SPELLCHECK, "application", application, NULL));

    return spellcheck;
}


GwSpellcheck* 
gw_spellcheck_new_with_entry (GwApplication *application, GtkEntry *entry)
{
    GwSpellcheck *spellcheck;
    
    spellcheck = gw_spellcheck_new (application);

    if (spellcheck != NULL)
      gw_spellcheck_set_entry (spellcheck, entry);

    return spellcheck;
}

static gchar**
gw_spellcheck_get_dictionary_paths ()
{
    //Declarations
    gchar **path;
    gchar *text;
    const gchar * const * system_path = g_get_system_data_dirs ();
    gint i;

    for (i = 0; system_path[i] != NULL; i++)
    {
      gchar *temp_path1 = g_build_filename (system_path[i], "myspell", NULL);
      gchar *temp_path2 = g_build_filename (system_path[i], "myspell", "dicts", NULL);
      gchar *temp_path3 = g_build_filename (system_path[i], "hunspell", NULL);

      text = g_strjoin (":", HUNSPELL_MYSPELL_DICTIONARY_PATH, temp_path1, temp_path2, temp_path3, NULL);

      g_free (temp_path1);
      g_free (temp_path2);
      g_free (temp_path3);
    }

    path = g_strsplit (text, ":", -1);

    g_free (text);

    return path;
}


static gchar* 
gw_spellcheck_build_noramalized_locale (const gchar *NAME)
{
    GDir *dir;
    gchar *locale;
    gchar *ptr;
    gchar **pathlist;
    const gchar *FILENAME;
    gint i;
    gint length;

    locale = g_strdup (NAME);

    //Truncate the UTF8 part of en_US.UTF8
    ptr = strchr(locale, '.');  
    if (ptr != NULL) *ptr = '\0'; 

    pathlist = gw_spellcheck_get_dictionary_paths ();
    if (pathlist != NULL)
    {
      //If the local lacks a country name, try to bulid one
      ptr = strchr(locale, '_'); 
      if (ptr == NULL)
      {
        for (i = 0; ptr == NULL && pathlist[i] != NULL; i++)
        {
          dir = g_dir_open (pathlist[i], 0, NULL);
          if (dir != NULL)
          {
            length = strlen (locale);
            while (ptr == NULL && (FILENAME = g_dir_read_name (dir)) != NULL)
            {
              if (strncmp(FILENAME, locale, length) == 0)
              {
                if (locale != NULL) g_free (locale);
                locale = g_strdup (FILENAME);
                ptr = strchr(locale, '.');  
                if (ptr != NULL) *ptr = '\0';
                ptr = strchr(locale, '_');
              }
            }
            g_dir_close (dir); dir = NULL;
          }
        }
      }
      g_strfreev (pathlist); pathlist = NULL;
    }

    //Wasn't a valid locale for a dictionary
    if (ptr == NULL && locale != NULL)
    {
      g_free (locale); locale = NULL;
    }
   
    return locale; 
}


static Hunhandle*
gw_spellcheck_get_hunhandle (const gchar *NAME)
{
    gchar **pathlist;
    gchar *path, *dpath, *affpath;
    gchar *locale;
    gint i;
    Hunhandle *handle;

    handle = NULL;

    locale = gw_spellcheck_build_noramalized_locale (NAME);
    if (locale != NULL)
    {
      pathlist = gw_spellcheck_get_dictionary_paths ();
      if (pathlist != NULL)
      {
        for (i = 0; handle == NULL && pathlist[i] != NULL; i++)
        {
          path = g_build_filename (pathlist[i], locale, NULL);
          dpath = g_strjoin (".", path, "dic", NULL);
          affpath = g_strjoin (".", path, "aff", NULL);
          if (g_file_test (affpath, G_FILE_TEST_IS_REGULAR) && 
              g_file_test (dpath, G_FILE_TEST_IS_REGULAR))
            handle = Hunspell_create (affpath, dpath);
          if (path != NULL) g_free (path); path = NULL;
          if (dpath != NULL) g_free (dpath); dpath = NULL;
          if (affpath != NULL) g_free (affpath); affpath = NULL;
        }
        g_strfreev (pathlist); pathlist = NULL;
      }
      g_free (locale); locale = NULL;
    }
    
    return handle;
}


void
gw_spellcheck_load_dictionary (GwSpellcheck *spellcheck)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    LwPreferences *preferences;
    const gint MAX = 100;
    const gchar *locale;
    gchar preferred[MAX];

    priv = spellcheck->priv;
    preferences = gw_application_get_preferences (priv->application);
    lw_preferences_get_string_by_schema (
      preferences, 
      preferred, 
      LW_SCHEMA_BASE, 
      LW_KEY_SPELLCHECK_DICTIONARY, 
      100);
    locale = setlocale (LC_ALL, NULL);

    //Clear the previous handle
    if (priv->handle != NULL) Hunspell_destroy (priv->handle); priv->handle = NULL;

    //See if we should try setting the prefered handle
    if (priv->handle == NULL && strncmp("auto", preferred, strlen("auto")) != 0)
      priv->handle = gw_spellcheck_get_hunhandle (preferred);

    //Load from locale if it starts with en
    if (priv->handle == NULL && strncmp("en", locale, strlen("en")) == 0)
      priv->handle = gw_spellcheck_get_hunhandle (locale);

    //Load from en_US
    if (priv->handle == NULL)
      priv->handle = gw_spellcheck_get_hunhandle ("en_US");

    //Load from en
    if (priv->handle == NULL)
      priv->handle = gw_spellcheck_get_hunhandle ("en");
}


static void
gw_spellcheck_init (GwSpellcheck *spellcheck)
{
    spellcheck->priv = GW_SPELLCHECK_GET_PRIVATE (spellcheck);
    memset(spellcheck->priv, 0, sizeof(GwSpellcheckPrivate));

    gw_spellcheck_set_timeout_threshold (spellcheck, 2);

    gw_spellcheck_attach_signals (spellcheck);
}


static void
gw_spellcheck_finalize (GObject *object)
{
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;

    spellcheck = GW_SPELLCHECK (object);
    priv = spellcheck->priv;

    if (priv->handle != NULL) Hunspell_destroy (priv->handle); priv->handle = NULL;

    gw_spellcheck_remove_signals (spellcheck);
    gw_spellcheck_clear (spellcheck);

    if (priv->entry != NULL) gtk_widget_queue_draw (GTK_WIDGET (priv->entry));

    G_OBJECT_CLASS (gw_spellcheck_parent_class)->finalize (object);
}


static void 
gw_spellcheck_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;

    spellcheck = GW_SPELLCHECK (object);
    priv = spellcheck->priv;

    switch (property_id)
    {
      case PROP_APPLICATION:
        priv->application = GW_APPLICATION (g_value_get_object (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void 
gw_spellcheck_get_property (GObject      *object,
                            guint         property_id,
                            GValue       *value,
                            GParamSpec   *pspec)
{
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;

    spellcheck = GW_SPELLCHECK (object);
    priv = spellcheck->priv;

    switch (property_id)
    {
      case PROP_APPLICATION:
        g_value_set_object (value, priv->application);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void
gw_spellcheck_class_init (GwSpellcheckClass *klass)
{
    //Declarations
    GParamSpec *pspec;
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = gw_spellcheck_set_property;
    object_class->get_property = gw_spellcheck_get_property;
    object_class->finalize = gw_spellcheck_finalize;

    g_type_class_add_private (object_class, sizeof (GwSpellcheckPrivate));

    pspec = g_param_spec_object ("application",
                                 "Application construct prop",
                                 "Set GwSpellcheck's Application",
                                 GW_TYPE_APPLICATION,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_APPLICATION, pspec);
}


static void  
gw_spellcheck_attach_signals (GwSpellcheck *spellcheck)
{
}


static void 
gw_spellcheck_remove_signals (GwSpellcheck *spellcheck)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    LwPreferences *preferences;
    GtkEntry *entry;
    GSource *source;
    int i;

    priv = spellcheck->priv;
    preferences = gw_application_get_preferences (priv->application);
    entry = priv->entry;

    if (priv->signalid[GW_SPELLCHECK_SIGNALID_DICTIONARY] != 0)
    {
      lw_preferences_remove_change_listener_by_schema (
          preferences, 
          LW_SCHEMA_BASE, 
          priv->signalid[GW_SPELLCHECK_SIGNALID_DICTIONARY]
      );
      priv->signalid[GW_SPELLCHECK_SIGNALID_DICTIONARY] = 0;
    }

    if (priv->signalid[GW_SPELLCHECK_SIGNALID_RK_CONV] != 0)
    {
      lw_preferences_remove_change_listener_by_schema (
          preferences, 
          LW_SCHEMA_BASE, 
          priv->signalid[GW_SPELLCHECK_SIGNALID_RK_CONV]
      );
      priv->signalid[GW_SPELLCHECK_SIGNALID_RK_CONV] = 0;
    }

    for (i = 0; i < TOTAL_GW_SPELLCHECK_SIGNALIDS && entry != NULL; i++)
    {
      if (priv->signalid[i] > 0)
      {
        g_signal_handler_disconnect (G_OBJECT (entry), priv->signalid[i]);
        priv->signalid[i] = 0;
      }
    }

    for (i = 0; i < TOTAL_GW_SPELLCHECK_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          priv->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, priv->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      priv->timeoutid[i] = 0;
    }
}


void
gw_spellcheck_set_timeout_threshold (GwSpellcheck *spellcheck, guint threshold)
{
   GwSpellcheckPrivate *priv;

   priv = spellcheck->priv;

   priv->threshold = threshold;
}


void
gw_spellcheck_set_entry (GwSpellcheck *spellcheck, GtkEntry *entry)
{

    GwSpellcheckPrivate *priv;
    LwPreferences *preferences;

    priv = spellcheck->priv;
    preferences = gw_application_get_preferences (priv->application);

    //Remove the old signals
    if (priv->entry != NULL)
    {

      if (priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW] != 0)
        g_signal_handler_disconnect (G_OBJECT (priv->entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);

      if (priv->signalid[GW_SPELLCHECK_SIGNALID_CHANGED] != 0)
        g_signal_handler_disconnect (G_OBJECT (priv->entry), priv->signalid[GW_SPELLCHECK_SIGNALID_CHANGED]);

      if (priv->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] != 0)
        g_signal_handler_disconnect (G_OBJECT (priv->entry), priv->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP]);

      if (priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY] != 0)
        g_signal_handler_disconnect (G_OBJECT (priv->entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY]);

      g_object_remove_weak_pointer (G_OBJECT (priv->entry), (gpointer*) (&(priv->entry)));
    }

    //Set the entry pointer
    priv->entry = entry;

    if (entry == NULL) return;

    g_object_add_weak_pointer (G_OBJECT (priv->entry), (gpointer*) (&(priv->entry)));

    //set the new signals
    priv->signalid[GW_SPELLCHECK_SIGNALID_DICTIONARY] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK_DICTIONARY,
        gw_spellcheck_sync_dictionary_cb,
        spellcheck
    );
    priv->signalid[GW_SPELLCHECK_SIGNALID_RK_CONV] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_ROMAN_KANA,
        gw_spellcheck_sync_rk_conv_cb,
        spellcheck
    );
    priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW] = g_signal_connect_after (
        G_OBJECT (entry), 
        "draw", 
        G_CALLBACK (gw_spellcheck_draw_underline_cb), 
        spellcheck
    );
    priv->signalid[GW_SPELLCHECK_SIGNALID_CHANGED] = g_signal_connect (
        G_OBJECT (entry), 
        "changed", 
        G_CALLBACK (gw_spellcheck_queue_cb), 
        spellcheck
    );

    priv->signalid[GW_SPELLCHECK_SIGNALID_BUTTON_PRESS_EVENT] = g_signal_connect (
        G_OBJECT (entry), 
        "button-press-event", 
        G_CALLBACK (gw_spellcheck_button_press_event_cb), 
        spellcheck
    );

    priv->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] = g_signal_connect (
        G_OBJECT (entry), 
        "populate-popup", 
        G_CALLBACK (gw_spellcheck_populate_popup_cb), 
        spellcheck
    );

    priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY] = g_signal_connect_swapped (
        G_OBJECT (entry), 
        "destroy", 
        G_CALLBACK (gw_spellcheck_remove_signals), 
        spellcheck
    );

    gw_spellcheck_queue (spellcheck);
}


gboolean
gw_spellcheck_clear (GwSpellcheck *spellcheck)
{
    g_return_val_if_fail (spellcheck != NULL, FALSE);

    GwSpellcheckPrivate *priv;
    gboolean changed;

    priv = spellcheck->priv;
    changed = (priv->misspelled != NULL);

    priv->timeout = 0;
    
    if (priv->tolkens != NULL)
      g_strfreev (priv->tolkens); priv->tolkens = NULL;
    if (priv->misspelled != NULL)
      g_list_free (priv->misspelled); priv->misspelled = NULL;

    return changed;
}


gint
gw_spellcheck_get_layout_y_offset (GwSpellcheck *spellcheck)
{
    g_return_val_if_fail (spellcheck != NULL, 0);

    //Declarations
    GwSpellcheckPrivate *priv;
    PangoRectangle rect;
    PangoLayout *layout;
    gint layout_offset;

    //Initializations
    priv = spellcheck->priv;
    layout = gtk_entry_get_layout (priv->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    gtk_entry_get_layout_offsets (priv->entry, NULL, &layout_offset);

    return (layout_offset + 1);
}


gint
gw_spellcheck_get_layout_x_offset (GwSpellcheck *spellcheck)
{
    g_return_val_if_fail (spellcheck != NULL, 0);

    //Declarations
    GwSpellcheckPrivate *priv;
    PangoRectangle rect;
    PangoLayout *layout;
    int layout_offset;

    //Initializations
    priv = spellcheck->priv;
    layout = gtk_entry_get_layout (priv->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    gtk_entry_get_layout_offsets (priv->entry, &layout_offset, NULL);

    return (layout_offset);
}


gboolean
gw_spellcheck_has_hiragana_conversion (GwSpellcheck *spellcheck)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    const gint MAX = 300;
    gchar kana[MAX];
    const gchar *query;
    gboolean has_hiragana_conversion;
    gint rk_conv_setting;
    gboolean want_conv;

    priv = spellcheck->priv;
    rk_conv_setting = priv->rk_conv_setting;
    want_conv = (rk_conv_setting == 0 || (rk_conv_setting == 2 && !lw_util_is_japanese_locale()));
    query = gtk_entry_get_text (priv->entry);
    has_hiragana_conversion = (want_conv && lw_util_str_roma_to_hira (query, kana, MAX));
  
    return has_hiragana_conversion;
}


gboolean
gw_spellcheck_should_check (GwSpellcheck *spellcheck)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    gboolean has_hiragana_conversion;
    gboolean should_check;
    const gchar *query;

    //Initializations
    priv = spellcheck->priv;
    has_hiragana_conversion = gw_spellcheck_has_hiragana_conversion (spellcheck);
    query = gtk_entry_get_text (priv->entry);
    should_check = (priv->handle != NULL && query != NULL && *query != '\0' && !has_hiragana_conversion);

    return should_check;
}


void
gw_spellcheck_queue (GwSpellcheck *spellcheck)
{
    g_return_if_fail (spellcheck != NULL);

    //Declarations
    GwSpellcheckPrivate *priv;
    gboolean should_check;
    gboolean should_redraw;

    //Initializations
    priv = spellcheck->priv;
    should_check = gw_spellcheck_should_check (spellcheck);

    g_return_if_fail (priv->handle != NULL);

    if (should_check)
    {
      priv->timeout = 0;
      if (priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] == 0)
        priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] = g_timeout_add_full (
          G_PRIORITY_LOW, 
          100, (GSourceFunc) 
          gw_spellcheck_update_timeout, 
          spellcheck, 
          NULL
      );
    }

    should_redraw = gw_spellcheck_clear (spellcheck);

    if (should_redraw) 
    {
      gtk_widget_queue_draw (GTK_WIDGET (priv->entry));
    }
}


static gboolean
gw_spellcheck_is_common_script (const gchar *TEXT)
{
    if (TEXT == NULL) return FALSE;

    GUnicodeScript script;
    gunichar c;
    const gchar *ptr;
    gboolean is_script;

    ptr = TEXT;
    is_script = TRUE;

    while (*ptr != '\0' && is_script == TRUE)
    {
      c = g_utf8_get_char (ptr);
      script = g_unichar_get_script (c);
      if (script != G_UNICODE_SCRIPT_COMMON && script != G_UNICODE_SCRIPT_LATIN)
      {
        is_script = FALSE;
      }
      ptr = g_utf8_next_char (ptr);
    }

    return is_script;
}


gboolean
gw_spellcheck_update (GwSpellcheck *spellcheck)
{
    GwSpellcheckPrivate *priv;
    gboolean should_check;
    gboolean should_redraw;
    const gchar *query;
    gchar **iter;

    priv = spellcheck->priv;

    if (priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] == 0)
    {
      return FALSE;
    }
    else if (priv->timeout < priv->threshold) 
    {
      priv->timeout++;
      return TRUE;
    }

    should_check = gw_spellcheck_should_check (spellcheck);
    should_redraw = gw_spellcheck_clear (spellcheck); //Make sure the memory is freed

    if (should_check)
    {
      query = gtk_entry_get_text (priv->entry);
      priv->tolkens = g_strsplit (query, " ", -1);

      for (iter = priv->tolkens; *iter != NULL; iter++)
      {
        if (**iter != '\0' && gw_spellcheck_is_common_script (*iter) && Hunspell_spell (priv->handle, *iter) == 0)
        {
          priv->misspelled = g_list_append (priv->misspelled, *iter);
        }
      }
    }

    if (should_redraw) gtk_widget_queue_draw (GTK_WIDGET (priv->entry));

    priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] = 0;
    priv->timeout = 0;

    return FALSE;
}


void
gw_spellcheck_record_mouse_cordinates (GwSpellcheck *spellcheck, GdkEvent *event)
{
    GwSpellcheckPrivate *priv;
    GtkWidget *toplevel;
    gint toplevel_x, toplevel_y;

    priv = spellcheck->priv;
    toplevel = GTK_WIDGET (gtk_widget_get_ancestor (GTK_WIDGET (priv->entry), GTK_TYPE_WINDOW));

    gdk_window_get_device_position (
      gtk_widget_get_window (GTK_WIDGET (priv->entry)),
      gdk_event_get_device (event),
      &toplevel_x, 
      &toplevel_y, 
      NULL
    );

    gtk_widget_translate_coordinates (
      toplevel, 
      GTK_WIDGET (priv->entry), 
      toplevel_x, 
      toplevel_y, 
      &priv->x, 
      &priv->y
    );
}


static int 
_get_string_index (GtkEntry *entry, int x, int y)
{
    //Declarations
    int layout_index;
    int entry_index;
    int trailing;
    PangoLayout *layout;

    //Initalizations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    if (pango_layout_xy_to_index (layout, x * PANGO_SCALE, y * PANGO_SCALE, &layout_index, &trailing))
      entry_index = gtk_entry_layout_index_to_text_index (GTK_ENTRY (entry), layout_index);
    else
      entry_index = -1;

    return entry_index;
}


void 
gw_spellcheck_populate_popup (GwSpellcheck *spellcheck, GtkMenu *menu)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    GtkWidget *menuitem, *spellmenuitem;
    GtkWidget *spellmenu;

    int index;
    int xoffset, yoffset, x, y;
    int start_offset, end_offset;
    int i;
    gchar **iter;
    gchar **suggestions;
    size_t total_suggestions;

    priv = spellcheck->priv;
    if (priv->tolkens == NULL) return;
    g_return_if_fail (priv->handle != NULL);

    xoffset = gw_spellcheck_get_layout_x_offset (spellcheck);
    yoffset = gw_spellcheck_get_layout_y_offset (spellcheck);
    x = priv->x - xoffset;
    y = priv->y - yoffset; //Since a GtkEntry is single line, we want the y to always be in the area
    index =  _get_string_index (priv->entry, x, y);

    start_offset = 0;
    iter = priv->tolkens;
    while (*iter != NULL && start_offset + strlen(*iter) < index)
    {
      start_offset += strlen(*iter) + 1;
      iter++;
    }
    if (*iter == NULL) return;
    end_offset = start_offset + strlen(*iter);

    if (Hunspell_spell (priv->handle, *iter) == 0)
    {
      total_suggestions = Hunspell_suggest (priv->handle, &suggestions, *iter);
      if (total_suggestions > 0 && suggestions != NULL)
      {
        menuitem = gtk_separator_menu_item_new ();
        gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), menuitem);
        gtk_widget_show (menuitem);

        spellmenu = gtk_menu_new ();
        spellmenuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_SPELL_CHECK, NULL);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (spellmenuitem), spellmenu);
        gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), spellmenuitem);
        gtk_widget_show (spellmenuitem);

        gchar *text = g_strdup_printf (gettext("Add \"%s\" to the dictionary"), *iter);
        if (text != NULL)
        {
          GtkWidget *image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
          menuitem = gtk_image_menu_item_new_with_label (text);
          g_object_set_data_full (G_OBJECT (menuitem), "word", g_strdup (*iter), g_free);
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_spellcheck_add_menuitem_activated_cb), spellcheck);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
          gtk_menu_shell_append (GTK_MENU_SHELL (spellmenu), menuitem);
          g_free (text); text = NULL;
          gtk_widget_show (menuitem);
        }

        menuitem = gtk_separator_menu_item_new ();
        gtk_menu_shell_append (GTK_MENU_SHELL (spellmenu), menuitem);
        gtk_widget_show (menuitem);

        //Menuitems
        for (i = 0; i < total_suggestions; i++)
        {
          menuitem = gtk_menu_item_new_with_label (suggestions[i]);
          g_object_set_data (G_OBJECT (menuitem), "start-offset", GINT_TO_POINTER (start_offset));
          g_object_set_data (G_OBJECT (menuitem), "end-offset", GINT_TO_POINTER (end_offset));
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_spellcheck_menuitem_activated_cb), spellcheck);
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_menu_shell_append (GTK_MENU_SHELL (spellmenu), menuitem);
        }

        Hunspell_free_list (priv->handle, &suggestions, total_suggestions);
      }
    }
}




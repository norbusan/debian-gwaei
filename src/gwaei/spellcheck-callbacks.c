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
//! @file spellcheck-callbacks.c
//!
//! @brief To be written 
//!

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


void gw_spellcheck_menuitem_activated_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    _SpellingReplacementData *srd;
    char *text;
    char *buffer;
    char *replacement;
    int start_offset;
    int end_offset;
    int index;

    //Initializations
    srd = data;
    replacement = srd->replacement_text;
    start_offset = srd->start_offset;
    end_offset = srd->end_offset;
    text = g_strdup (gtk_entry_get_text (GTK_ENTRY (srd->entry)));
    buffer = (char*) malloc (sizeof(char) * (strlen(replacement) + strlen(text)));

    strcpy(buffer, text);
    strcpy (buffer + start_offset, replacement);
    strcat (buffer, text + end_offset);

    index = gtk_editable_get_position (GTK_EDITABLE (srd->entry));
    if (index > end_offset || index > start_offset + strlen(replacement))
      index = index - (end_offset - start_offset) + strlen(replacement);
    gtk_entry_set_text (GTK_ENTRY (srd->entry), buffer);
    gtk_editable_set_position (GTK_EDITABLE (srd->entry), index);

    //Cleanup
    free (buffer);
    g_free (text);
}


static int _get_string_index (GtkEntry *entry, int x, int y)
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


gboolean _get_line_coordinates (GwSpellcheck *spellcheck, int startindex, int endindex, int *x, int *y, int *x2, int *y2)
{
    //Declarations
    int index;
    PangoLayout *layout;
    PangoRectangle rect;
    PangoLayoutIter *iter;
    int xoffset, yoffset;

    //Initializations
    layout = gtk_entry_get_layout (spellcheck->entry);
    iter = pango_layout_get_iter (layout);
    xoffset = gw_spellcheck_get_x_offset (spellcheck);
    yoffset = gw_spellcheck_get_y_offset (spellcheck);
    *x = *y = *x2 = *y2 = 0;

    do {
      index = pango_layout_iter_get_index (iter);
      pango_layout_iter_get_char_extents  (iter, &rect);
      if (index == startindex)
      {
        *x = PANGO_PIXELS (rect.x) + xoffset;
        *y = PANGO_PIXELS (rect.y) + yoffset;
      }
      if (index == endindex - 1)
      {
        *x2 = PANGO_PIXELS (rect.width + rect.x) + xoffset;
        *y2 = PANGO_PIXELS (rect.height + rect.y) + yoffset;
      }
    } while (pango_layout_iter_next_char (iter));

    //Cleanup
    pango_layout_iter_free (iter);

    return (*x > 0 && *y > 0 && *x2 > 0 && *y2 > 0);
}

void _draw_line (cairo_t *cr, int x, int y, int x2, int y2)
{
    //Declarations
    int ydelta;
    int xdelta;
    int i;
    gboolean up;

    //Initializations
    xdelta = 2;
    ydelta = 2;
    up = FALSE;
    y += ydelta;
    x++;

    cairo_set_line_width (cr, 0.8);
    cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 0.8);

    cairo_move_to (cr, x, y2);
    for (i = x + xdelta; i < x2; i += xdelta)
    {
      if (up)
        y2 -= ydelta;
      if (!up)
        y2 += ydelta;
      up = !up;

      cairo_line_to (cr, i, y2);
    }
    cairo_stroke (cr);
}


void gw_spellcheck_populate_cb (GtkEntry *entry, GtkMenu *menu, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    GtkWidget *menuitem;
    char **split;
    char **info;
    char **replacements;
    GList *iter;

    spellcheck = GW_SPELLCHECK (data);
    int index;
    int xpointer, ypointer, xoffset, yoffset, x, y;
    _SpellingReplacementData *srd;
    int start_offset, end_offset;
    int i;

    //Initializations
    gtk_widget_get_pointer (GTK_WIDGET (entry), &xpointer, &ypointer);
    xoffset = gw_spellcheck_get_x_offset (spellcheck);
    yoffset = gw_spellcheck_get_y_offset (spellcheck);
    x = xpointer - xoffset;
    y = yoffset; //Since a GtkEntry is single line, we want the y to always be in the area
    index =  _get_string_index (entry, x, y);

    g_mutex_lock (spellcheck->mutex);
    for (iter = spellcheck->corrections; index > -1 && iter != NULL; iter = iter->next)
    {
      //Create the start and end offsets 
      split = g_strsplit (iter->data, ":", 2);
      info = g_strsplit (split[0], " ", -1); 
      start_offset = (int) g_ascii_strtoull (info[3], NULL, 10);
      end_offset = strlen(info[1]) + start_offset;

      //If the mouse position is between the offsets, create the popup menuitems
      if (index >= start_offset && index <= end_offset)
      {
        replacements = g_strsplit (split[1], ",", -1);

        //Separator
        if (replacements[0] != NULL)
        {
          menuitem = gtk_separator_menu_item_new ();
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        }

        //Menuitems
        for (i = 0; replacements[i] != NULL; i++)
        {
          g_strstrip(replacements[i]);
          menuitem = gtk_menu_item_new_with_label (replacements[i]);
          srd = (_SpellingReplacementData*) malloc (sizeof(_SpellingReplacementData));
          srd->entry = entry;
          srd->start_offset = start_offset;
          srd->end_offset = end_offset;
          srd->replacement_text = g_strdup (replacements[i]);
          g_signal_connect (G_OBJECT (menuitem), "destroy", G_CALLBACK (gw_spellcheck_free_menuitem_data_cb), (gpointer) srd);
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_spellcheck_menuitem_activated_cb), (gpointer) srd);
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        }
        g_strfreev (replacements);
      }
      g_strfreev (split);
      g_strfreev (info);
    }
    g_mutex_unlock (spellcheck->mutex);
}


gboolean gw_spellcheck_draw_underline_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    gint x, y, x2, y2;
    GList *iter;
    char **info;
    char **atoms;
    int start_offset, end_offset;

    //Initializations
    spellcheck = GW_SPELLCHECK (data);

    g_mutex_lock (spellcheck->mutex);
    for (iter = spellcheck->corrections; iter != NULL; iter = iter->next)
    {
      if (iter->data == NULL) continue;

      info = g_strsplit (iter->data, ":", -1);
      atoms = g_strsplit (info[0], " ", -1);

      start_offset = (int) g_ascii_strtoull (atoms[3], NULL, 10);
      end_offset = strlen(atoms[1]) + start_offset;
      start_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), start_offset);
      end_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), end_offset);

      //Calculate the line
      if (_get_line_coordinates (spellcheck, start_offset, end_offset, &x, &y, &x2, &y2))
      {
        _draw_line (cr, x, y, x2, y2);
      }

      g_strfreev (info);
      g_strfreev (atoms);
    }
    g_mutex_unlock (spellcheck->mutex);

    return FALSE;
}


void gw_spellcheck_queue_cb (GtkEditable *editable, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    const char *query;

    //Initializations
    spellcheck = GW_SPELLCHECK (data);
    g_mutex_lock (spellcheck->mutex);
    if (spellcheck->query_text == NULL)
      spellcheck->query_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (editable)));
    query = gtk_entry_get_text (GTK_ENTRY (editable));

    if (strcmp(spellcheck->query_text, query) != 0)
    {
      //Clear out the old links
      while (spellcheck->corrections != NULL)
      {
        g_free (spellcheck->corrections->data);
        spellcheck->corrections = g_list_delete_link (spellcheck->corrections, spellcheck->corrections);
      }

      g_free (spellcheck->query_text);
      spellcheck->query_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (editable)));

      spellcheck->needs_spellcheck = TRUE;
      spellcheck->timeout = 0;
    }
    g_mutex_unlock (spellcheck->mutex);
}


GwSpellcheckStreamWithData* gw_spellcheck_streamwithdata_new (GwSpellcheck *spellcheck, int stream, const char* data, int length, GPid pid)
{
    GwSpellcheckStreamWithData *temp;

    if ((temp = malloc(sizeof(GwSpellcheckStreamWithData))) != NULL)
    {
      temp->spellcheck = spellcheck;
      temp->stream = stream;
      temp->data = malloc(length);
      if (temp->data != NULL) strncpy(temp->data, data, length);
      temp->length = length;
      temp->pid = pid;
    }

    return temp;
}


void gw_spellcheck_streamwithdata_free (GwSpellcheckStreamWithData *swd)
{
    free(swd->data);
    free(swd);
}


static gpointer _infunc (gpointer data)
{
    //Declarations
    GwSpellcheckStreamWithData *swd;
    FILE *file;
    int stream;
    char *text;

    //Initializations
    swd = data;
    stream = swd->stream;
    text = swd->data;
    file = fdopen(stream, "w");

    if (file != NULL)
    {
      if (ferror(file) == 0 && feof(file) == 0)
      {
        fwrite(text, sizeof(char), strlen(text), file);
      }

      fclose(file);
    }

    gw_spellcheck_streamwithdata_free (swd);

    return NULL;
}


static gpointer _outfunc (gpointer data)
{
    //Declarations
    const int MAX = 500;
    GwSpellcheckStreamWithData *swd;
    FILE *file;
    char buffer[MAX];
    GwSpellcheck *spellcheck;

    //Initializations
    swd = data;
    spellcheck = swd->spellcheck;
    file = fdopen (swd->stream, "r");

    if (file != NULL)
    {

      //Clear out the old links
      while (spellcheck->corrections != NULL)
      {
        g_mutex_lock (spellcheck->mutex);
        g_free (spellcheck->corrections->data);
        spellcheck->corrections = g_list_delete_link (spellcheck->corrections, spellcheck->corrections);
        g_mutex_unlock (spellcheck->mutex);
      }

      //Add the new links
      while (file != NULL && ferror(file) == 0 && feof(file) == 0 && fgets(buffer, MAX, file) != NULL)
      {
        g_mutex_lock (spellcheck->mutex);
        if (buffer[0] != '@' && buffer[0] != '*' && buffer[0] != '#' && strlen(buffer) > 1)
          spellcheck->corrections = g_list_append (spellcheck->corrections, g_strdup (buffer));
        g_mutex_unlock (spellcheck->mutex);
      }

      //Cleanup
      fclose (file);
    }

    g_spawn_close_pid (swd->pid);

    gw_spellcheck_streamwithdata_free (swd);

    g_mutex_lock (spellcheck->mutex);
    spellcheck->running_check = FALSE;
    spellcheck->thread = NULL;
    g_mutex_unlock (spellcheck->mutex);

    return NULL;
}


gboolean gw_spellcheck_update_timeout (gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    GwSearchWindow *window;
    GwApplication *application;
    LwPreferences *preferences;

    //Initializaitons
    spellcheck = GW_SPELLCHECK (data);
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (spellcheck->entry), GW_TYPE_SEARCHWINDOW));
    if (window == NULL) return FALSE;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
/*
    //Make sure any previous spellchecks have finished
    g_mutex_lock (spellcheck->mutex);
    if (spellcheck->thread != NULL) 
    {
      g_mutex_unlock (spellcheck->mutex);
      return TRUE;
    }
    g_mutex_unlock (spellcheck->mutex);
*/
    //Sanity check
    if (spellcheck->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] == 0) return TRUE;

    g_mutex_lock (spellcheck->mutex);
    if (spellcheck->running_check == TRUE)
    {
      g_mutex_unlock (spellcheck->mutex);
      return TRUE;
    }
    else if (spellcheck->timeout < 5) {
      spellcheck->timeout++;
      g_mutex_unlock (spellcheck->mutex);
      return TRUE;
    }
    else 
    {
      spellcheck->running_check = TRUE;
      spellcheck->timeout = 0;
      g_mutex_unlock (spellcheck->mutex);
    }

    //Declarations
    gboolean spellcheck_pref;
    int rk_conv_pref;
    gboolean want_conv;
    const char *query;
    gboolean is_convertable_to_hiragana;
    const int MAX = 300;
    char kana[MAX];
    gboolean exists;
    GError *error;

    char *argv[] = { ENCHANT, "-a", "-d", "en", NULL};
    GPid pid;
    int stdin_stream;
    int stdout_stream;
    gboolean success;
    GwSpellcheckStreamWithData *indata;
    GwSpellcheckStreamWithData *outdata;
    
    //Initializations
    rk_conv_pref = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA);
    want_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
    query = gtk_entry_get_text (spellcheck->entry);
    is_convertable_to_hiragana = (want_conv && lw_util_str_roma_to_hira (query, kana, MAX));
    spellcheck_pref = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);
    exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);
    error = NULL;

    //Sanity checks
    if (
      exists == FALSE    || 
      strlen(query) == 0 || 
      !spellcheck_pref   || 
      !spellcheck->sensitive        || 
      !spellcheck->needs_spellcheck || 
      is_convertable_to_hiragana
    )
    {
      spellcheck->running_check = FALSE;
      return TRUE;
    }

    spellcheck->needs_spellcheck = FALSE;

    success = g_spawn_async_with_pipes (
      NULL, 
      argv,
      NULL,
      0,
      NULL,
      NULL,
      &pid,
      &stdin_stream,
      &stdout_stream,
      NULL,
      &error
    );

    if (success)
    {
      indata = gw_spellcheck_streamwithdata_new (spellcheck, stdin_stream, query, strlen(query) + 1, pid);
      outdata = gw_spellcheck_streamwithdata_new (spellcheck, stdout_stream, query, strlen(query) + 1, pid);

      if (indata != NULL && outdata != NULL)
      {
        _infunc ((gpointer) indata);
        spellcheck->thread = g_thread_create (_outfunc, (gpointer) outdata, TRUE, &error);
      }

      if (spellcheck->thread == NULL)
      {
        spellcheck->running_check = FALSE;
      }
    }
    
    gw_application_handle_error (application, NULL, FALSE, &error);

    //gtk_widget_queue_draw (GTK_WIDGET (data));
  
    return TRUE;
}





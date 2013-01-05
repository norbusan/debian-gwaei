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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/spellcheck-private.h>
#include <gwaei/gettext.h>


void
gw_spellcheck_add_menuitem_activated_cb (GtkWidget *widget, gpointer data)
{
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;
    gchar *word;

    spellcheck = GW_SPELLCHECK (data);
    priv = spellcheck->priv;
    word = (gchar*) g_object_get_data (G_OBJECT (widget), "word");

    if (priv->handle == NULL) return;

    Hunspell_add (priv->handle, word);
    gw_spellcheck_queue (spellcheck);
}


void 
gw_spellcheck_menuitem_activated_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;
    const gchar *query;
    const gchar *replacement;
    gchar *buffer;
    gint start_offset;
    gint end_offset;
    gint index;

    //Initializations
    spellcheck = GW_SPELLCHECK (data);
    priv = spellcheck->priv;
    query = gtk_entry_get_text (priv->entry);
    start_offset = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "start-offset"));
    end_offset = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "end-offset"));
    replacement = gtk_menu_item_get_label (GTK_MENU_ITEM (widget));
    buffer = g_new (gchar, strlen(replacement) + strlen(query));
    index = gtk_editable_get_position (GTK_EDITABLE (priv->entry));

    //Build the replacement query string
    strncpy(buffer, query, start_offset);
    strcpy (buffer + start_offset, replacement);
    strcat (buffer, query + end_offset);

    //Update where the cursor should be relative to the replacement word length
    if (index >= start_offset + strlen(replacement))
      index += strlen(buffer) - strlen(query);

    //Showtime
    gtk_entry_set_text (GTK_ENTRY (priv->entry), buffer);
    gtk_editable_set_position (GTK_EDITABLE (priv->entry), index);

    //Cleanup
    g_free (buffer);
}


static gboolean 
gw_spellcheck_get_line_coordinates (GwSpellcheck *spellcheck, int startindex, int endindex, int *x, int *y, int *x2, int *y2)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    int index;
    PangoLayout *layout;
    PangoRectangle rect;
    PangoLayoutIter *iter;
    int xoffset, yoffset;

    //Initializations
    priv = spellcheck->priv;
    layout = gtk_entry_get_layout (priv->entry);
    iter = pango_layout_get_iter (layout);
    xoffset = gw_spellcheck_get_layout_x_offset (spellcheck);
    yoffset = gw_spellcheck_get_layout_y_offset (spellcheck);
    *x = *y = *x2 = *y2 = 0;

    do {
      index = pango_layout_iter_get_index (iter);
      pango_layout_iter_get_char_extents  (iter, &rect);
      if (index == startindex)
      {
        *x = PANGO_PIXELS (rect.x) + xoffset;
        *y = PANGO_PIXELS (rect.y + rect.height) + yoffset;
      }
      if (index == endindex - 1)
      {
        *x2 = PANGO_PIXELS (rect.width + rect.x) + xoffset + 1;
        *y2 = *y;
      }
    } while (pango_layout_iter_next_char (iter));

    //Cleanup
    pango_layout_iter_free (iter);

    return (*x > 0 && *y > 0 && *x2 > 0 && *y2 > 0);
}

static void 
gw_spellcheck_draw_line (cairo_t *cr, int x, int y, int x2, int y2)
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


G_MODULE_EXPORT gboolean 
gw_spellcheck_button_press_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GwSpellcheck *spellcheck;

    spellcheck = GW_SPELLCHECK (data);

    gw_spellcheck_record_mouse_cordinates (spellcheck, event);

    return FALSE;
}


G_MODULE_EXPORT void 
gw_spellcheck_populate_popup_cb (GtkEntry *entry, GtkMenu *menu, gpointer data)
{
    GwSpellcheck *spellcheck;

    spellcheck = GW_SPELLCHECK (data);

    gw_spellcheck_populate_popup (spellcheck, menu);
}


G_MODULE_EXPORT gboolean 
gw_spellcheck_draw_underline_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;
    gint x, y, x2, y2;
    GList *link;
    gchar **iter;
    gint start_offset, end_offset;
    gint start_layout_offset, end_layout_offset;

    //Initializations
    spellcheck = GW_SPELLCHECK (data);
    priv = spellcheck->priv;
    iter = priv->tolkens;
    start_offset = end_offset = 0;

    for (link = priv->misspelled; link != NULL && *iter != NULL; link = link->next)
    {
      //Get the letter offsets
      while (*iter != NULL && *iter != (gchar*) link->data)
      {
        start_offset += strlen(*iter) + 1;
        iter++;
      }
      if (*iter != NULL) end_offset = start_offset + strlen(*iter);
      else end_offset = start_offset;

      //Convert it to the layout offset
      start_layout_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), start_offset);
      end_layout_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), end_offset);

      //Calculate the line
      if (gw_spellcheck_get_line_coordinates (spellcheck, start_layout_offset, end_layout_offset, &x, &y, &x2, &y2))
      {
        gw_spellcheck_draw_line (cr, x, y, x2, y2);
      }
    }

    return FALSE;
}


G_MODULE_EXPORT void 
gw_spellcheck_queue_cb (GtkEditable *editable, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;

    //Initializations
    spellcheck = GW_SPELLCHECK (data);

    gw_spellcheck_queue (spellcheck);
}


gboolean 
gw_spellcheck_update_timeout (gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;

    //Initializaitons
    spellcheck = GW_SPELLCHECK (data);

    return gw_spellcheck_update (spellcheck);
}



G_MODULE_EXPORT void 
gw_spellcheck_sync_rk_conv_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;
    gint request;

    spellcheck = GW_SPELLCHECK (data);
    priv = spellcheck->priv;

    request = lw_preferences_get_int (settings, key);

    priv->rk_conv_setting = request;
}


G_MODULE_EXPORT void
gw_spellcheck_sync_dictionary_cb (GSettings *settings, gchar *key, gpointer data)
{
    GwSpellcheck *spellcheck;

    spellcheck = GW_SPELLCHECK (data);

    gw_spellcheck_load_dictionary (spellcheck);
    gw_spellcheck_queue (spellcheck);
}

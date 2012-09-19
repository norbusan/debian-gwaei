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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static void gw_spellcheck_attach_signals (GwSpellcheck*);
static void gw_spellcheck_remove_signals (GwSpellcheck*);

GwSpellcheck* gw_spellcheck_new (GtkEntry *entry)
{
    GwSpellcheck *temp;

    temp = (GwSpellcheck*) malloc(sizeof(GwSpellcheck));

    if (temp != NULL)
    {
      gw_spellcheck_init (temp, entry);
    }

    return temp;
}

void gw_spellcheck_free (GwSpellcheck *spellcheck)
{
    gw_spellcheck_deinit (spellcheck);
    free (spellcheck);
}


void gw_spellcheck_init (GwSpellcheck *spellcheck, GtkEntry *entry)
{
    spellcheck->entry = entry;
    spellcheck->corrections = NULL;
    spellcheck->mutex = g_mutex_new ();
    spellcheck->needs_spellcheck = FALSE;
    spellcheck->query_text = NULL;
    spellcheck->sensitive = TRUE;
    spellcheck->running_check = FALSE;
    spellcheck->timeout = 0;
    spellcheck->thread = NULL;

    gw_spellcheck_attach_signals (spellcheck);
}


void gw_spellcheck_deinit (GwSpellcheck *spellcheck)
{
    gw_spellcheck_remove_signals (spellcheck);

    if (spellcheck->thread != NULL) g_thread_join (spellcheck->thread);
    spellcheck->thread = NULL;

    g_free (spellcheck->query_text);
    g_mutex_free (spellcheck->mutex);
}


static void  gw_spellcheck_attach_signals (GwSpellcheck *spellcheck)
{
    //Declarations
    GtkEntry *entry;
    int i;

    entry = spellcheck->entry;

    for (i = 0; i < TOTAL_GW_SPELLCHECK_SIGNALIDS; i++)
      spellcheck->signalid[i] = 0;
    for (i = 0; i < TOTAL_GW_SPELLCHECK_TIMEOUTIDS; i++)
      spellcheck->timeoutid[i] = 0;

    spellcheck->signalid[GW_SPELLCHECK_SIGNALID_DRAW] = g_signal_connect_after (
        G_OBJECT (entry), 
        "draw", 
        G_CALLBACK (gw_spellcheck_draw_underline_cb), 
        spellcheck
    );
    spellcheck->signalid[GW_SPELLCHECK_SIGNALID_CHANGED] = g_signal_connect (
        G_OBJECT (entry), 
        "changed", 
        G_CALLBACK (gw_spellcheck_queue_cb), 
        spellcheck
    );
    spellcheck->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] = g_signal_connect (
        G_OBJECT (entry), 
        "populate-popup", 
        G_CALLBACK (gw_spellcheck_populate_cb), 
        spellcheck
    );

    spellcheck->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] = g_timeout_add_full (
        G_PRIORITY_LOW, 
        100, (GSourceFunc) 
        gw_spellcheck_update_timeout, 
        spellcheck, 
        NULL
    );

    g_signal_connect_swapped (G_OBJECT (entry), "destroy", G_CALLBACK (gw_spellcheck_remove_signals), spellcheck);
}


static void gw_spellcheck_remove_signals (GwSpellcheck *spellcheck)
{
    //Declarations
    GtkEntry *entry;
    GSource *source;
    int i;

    entry = spellcheck->entry;

    for (i = 0; i < TOTAL_GW_SPELLCHECK_SIGNALIDS; i++)
    {
      if (spellcheck->signalid[i] > 0)
      {
        g_signal_handler_disconnect (G_OBJECT (entry), spellcheck->signalid[i]);
        spellcheck->signalid[i] = 0;
      }
    }

    for (i = 0; i < TOTAL_GW_SPELLCHECK_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          spellcheck->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, spellcheck->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      spellcheck->timeoutid[i] = 0;
    }
}

void gw_spellcheck_free_menuitem_data_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    _SpellingReplacementData *srd;

    //Initializations
    srd = data;

    //Cleanup
    g_free (srd->replacement_text);
    free (srd);
}


int gw_spellcheck_get_y_offset (GwSpellcheck *spellcheck)
{
    //Declarations
    PangoRectangle rect;
    PangoLayout *layout;

    int allocation_offset;
    int layout_offset;
    int rect_offset;

    //Initializations
    layout = gtk_entry_get_layout (spellcheck->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    rect_offset = rect.height;
    allocation_offset = gtk_widget_get_allocated_height (GTK_WIDGET (spellcheck->entry));
    gtk_entry_get_layout_offsets (spellcheck->entry, NULL, &layout_offset);

    return (((allocation_offset - rect_offset) / 2) - layout_offset);
}


int gw_spellcheck_get_x_offset (GwSpellcheck *spellcheck)
{
    //Declarations
    PangoRectangle rect;
    PangoLayout *layout;
    int layout_offset;

    //Initializations
    layout = gtk_entry_get_layout (spellcheck->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    gtk_entry_get_layout_offsets (spellcheck->entry, &layout_offset, NULL);

    return (layout_offset);
}



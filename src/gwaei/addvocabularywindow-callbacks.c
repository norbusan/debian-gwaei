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
//!  @file addvocabularywindow.c
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/addvocabularywindow-private.h>


G_MODULE_EXPORT void 
gw_addvocabularywindow_add_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;
    GwAddVocabularyWindowClass *klass;
    GtkListStore *wordstore;
    const gchar *kanji, *furigana, *definitions;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    klass = GW_ADDVOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));

    kanji = gw_addvocabularywindow_get_kanji (window);
    furigana = gw_addvocabularywindow_get_furigana (window);
    definitions = gw_addvocabularywindow_get_definitions (window);
    wordstore = gw_addvocabularywindow_get_wordstore (window);
    priv->wordstore = GW_VOCABULARYWORDSTORE (wordstore);

    gw_vocabularywordstore_load (priv->wordstore, NULL);
    gw_vocabularywordstore_new_word (priv->wordstore, &(priv->iter), NULL, kanji, furigana, definitions);

    if (klass->last_selected_list_name != NULL)
      g_free (klass->last_selected_list_name);
    klass->last_selected_list_name = g_strdup (gw_addvocabularywindow_get_list (window));

    priv->valid = TRUE;

    g_signal_emit (G_OBJECT (window), klass->signalid[GW_ADDVOCABULARYWINDOW_CLASS_SIGNALID_WORD_ADDED], 0);

    gtk_widget_destroy (GTK_WIDGET (window));
}


G_MODULE_EXPORT void 
gw_addvocabularywindow_cancel_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    g_return_if_fail (window != NULL);

    gtk_widget_destroy (GTK_WIDGET (window));
}


G_MODULE_EXPORT void
gw_addvocabularywindow_kanji_changed_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    if (priv->kanji_text != NULL) g_free (priv->kanji_text);
    priv->kanji_text = g_strdup (gtk_entry_get_text (priv->kanji_entry));
    g_strstrip (priv->kanji_text);

    gw_addvocabularywindow_validate (window);
}


G_MODULE_EXPORT void
gw_addvocabularywindow_furigana_changed_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    if (priv->furigana_text != NULL) g_free (priv->furigana_text);
    priv->furigana_text = g_strdup (gtk_entry_get_text (priv->furigana_entry));
    g_strstrip (priv->furigana_text);

    gw_addvocabularywindow_validate (window);
}


void gw_addvocabularywindow_definitions_event_after_cb (GtkWidget*, GdkEvent*, gpointer);


G_MODULE_EXPORT void
gw_addvocabularywindow_definitions_paste_done_cb (GtkTextBuffer *buffer,
                                                  GtkClipboard  *clipboard,
                                                  gpointer       data      )
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;
    GtkTextIter start, end;
    gchar *text;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);


    text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
    g_strdelimit (text, "\n", ' ');
    g_strstrip (text);

    gtk_text_buffer_set_text (buffer, text, -1);

    priv->pasted = TRUE;

    g_free (text);
}


G_MODULE_EXPORT void
gw_addvocabularywindow_definitions_event_after_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{

    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    gboolean valid;
    gboolean activate;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    if (priv->pasted == TRUE) { priv->pasted = FALSE; return; }

    buffer = gtk_text_view_get_buffer (priv->definitions_textview);
    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);

    if (priv->definitions_text != NULL) g_free (priv->definitions_text);
    priv->definitions_text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
    activate = (strchr (priv->definitions_text, '\n') != NULL);
    g_strstrip (priv->definitions_text);

    gchar *source_ptr, *target_ptr;
    source_ptr = target_ptr = priv->definitions_text;
    while (*target_ptr != '\0')
    {
      if (*target_ptr == '\n') target_ptr++;
      else if (source_ptr == target_ptr)
      {
        source_ptr = ++target_ptr;
      }
      else
      {
        *(source_ptr++) = *(target_ptr++);
      }
    }
    *source_ptr = '\0';

    valid = gw_addvocabularywindow_validate (window);

    if (activate)
    {
      G_GNUC_EXTENSION g_signal_handlers_block_by_func (widget, gw_addvocabularywindow_definitions_event_after_cb, data);
      gtk_text_buffer_set_text (buffer, priv->definitions_text, -1);
      G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (widget, gw_addvocabularywindow_definitions_event_after_cb, data);
      if (valid)
      {
        gtk_button_clicked (priv->add_button);
      }
    }
}


G_MODULE_EXPORT void
gw_addvocabularywindow_list_changed_cb (GtkWidget *widget, gpointer data)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;
    GtkEntry *entry;

    window = GW_ADDVOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_ADDVOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->vocabulary_list_combobox)));

    if (priv->list_text != NULL) g_free (priv->list_text);
    priv->list_text = g_strdup (gtk_entry_get_text (entry));
    g_strstrip (priv->list_text);

    gw_addvocabularywindow_validate (window);
}


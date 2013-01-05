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
//! @file searchwindow-output.c
//!
//! @brief To be written
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/searchwindow-private.h>

static void gw_searchwindow_append_edict_result (GwSearchWindow*, LwSearch*);
static void gw_searchwindow_append_kanjidict_result (GwSearchWindow*, LwSearch*);
static void gw_searchwindow_append_examplesdict_result (GwSearchWindow*, LwSearch*);
static void gw_searchwindow_append_unknowndict_result (GwSearchWindow*, LwSearch*);

static void
gw_searchwindow_insert_addlink (GwSearchWindow   *window,
                                GtkTextBuffer    *buffer,
                                GtkTextIter      *iter,
                                LwWord     *word   )
{
    //Sanity checks
    g_return_if_fail (window != NULL);
    g_return_if_fail (buffer != NULL);
    g_return_if_fail (word != NULL);

    //Declarations
    GtkTextTag *tag;
    gchar *data;
    
    //Initializations
    tag = gtk_text_buffer_create_tag (buffer, NULL, 
        "rise",   5000, 
        "scale",  0.75, 
        "weight", PANGO_WEIGHT_BOLD,
        NULL);
    data = lw_word_to_string (word);
    g_object_set_data_full (G_OBJECT (tag), "word-data", data, g_free);
    g_object_set_data (G_OBJECT (tag), "buffer", (gpointer) buffer);

    gtk_text_buffer_insert (buffer, iter, " ", -1);
    gtk_text_buffer_insert_with_tags (buffer, iter, "+", -1, tag, NULL);
    gtk_text_buffer_insert (buffer, iter, " ", -1 );
}


void
gw_searchwindow_insert_edict_addlink (GwSearchWindow *window, 
                                      LwResult       *result, 
                                      GtkTextBuffer  *buffer, 
                                      GtkTextIter    *iter)
{
    //Declarations
    gchar *kanji, *furigana, *definitions;
    LwWord *word;

    //Initializations
    kanji = result->kanji_start;
    furigana = result->furigana_start;
    if (furigana == NULL || strlen (furigana) == 0)
      furigana = kanji;
    definitions = g_strjoinv ("/", result->def_start);

    if (definitions != NULL)
    {
      word = lw_word_new ();
      if (word != NULL)
      {
        lw_word_set_kanji (word, kanji);
        lw_word_set_furigana (word, furigana);
        lw_word_set_definitions (word, definitions);

        gw_searchwindow_insert_addlink (window, buffer, iter, word);

        lw_word_free (word);
      }
      g_free (definitions);
    }
}

//!
//! @brief Appends a result to the output
//! @param engine The LwEngine to use for output
//! @param search The data from the LwSearch
//!
void 
gw_searchwindow_append_result (GwSearchWindow *window, LwSearch* search)
{
    //Sanity checks
    g_return_if_fail (window != NULL);
    g_return_if_fail (search != NULL);

    //Declarations
    GType type;

    //Initializations
    type = G_OBJECT_TYPE (search->dictionary);

    if (g_type_is_a (type, LW_TYPE_EDICTIONARY))
      gw_searchwindow_append_edict_result (window, search);
    else if (g_type_is_a (type, LW_TYPE_KANJIDICTIONARY))
        gw_searchwindow_append_kanjidict_result (window, search);
    else if (g_type_is_a (type, LW_TYPE_EXAMPLEDICTIONARY))
      gw_searchwindow_append_examplesdict_result (window, search);
    else if (g_type_is_a (type, LW_TYPE_UNKNOWNDICTIONARY))
      gw_searchwindow_append_unknowndict_result (window, search);
    else
      g_warning ("%s\n", gettext("This is an unknown dictionary type!"));
}


//!
//! @brief PRIVATE FUNCTION. Applies a tag to a section of text
//!
//! @param line An integer showing the line in the buffer to tag
//! @param start_offset the ending character in the line to highlight
//! @param end_offset The ending character in the line to highlight
//! @param search A LwSearch to get general information from
//!
static void 
gw_add_match_highlights (gint line, gint start_offset, gint end_offset, LwSearch* search)
{
    //Sanity check
    g_assert (lw_search_has_data (search));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    LwQuery *query;
    gint match_start_byte_offset;
    gint match_end_byte_offset;
    gint match_character_offset;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    gchar *text;
    GRegex *regex;
    GList *link;
    GMatchInfo *match_info;

    //Initializations
    sdata = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    query = search->query;
    gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, start_offset);
    gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, end_offset);
    text = gtk_text_buffer_get_slice (buffer, &start_iter, &end_iter, FALSE);

    //Look for kanji atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_KANJI, LW_RELEVANCE_LOW);
    while (link != NULL)
    {
      regex = link->data;
      if (regex != NULL && g_regex_match (regex, text, 0, &match_info))
      { 
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (buffer, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info); match_info = NULL;
      }
      link = link->next;
    }

    //Look for furigana atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_FURIGANA, LW_RELEVANCE_LOW);
    while (link != NULL)
    {
      regex = link->data;
      if (regex != NULL && g_regex_match (regex, text, 0, &match_info))
      { 
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (buffer, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info); match_info = NULL;
      }
      link = link->next;
    }

    //Look for romaji atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_ROMAJI, LW_RELEVANCE_LOW);
    while (link != NULL)
    {
      regex = link->data;
      if (regex != NULL && g_regex_match (regex, text, 0, &match_info))
      {
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (buffer, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info); match_info = NULL;
      }
      link = link->next;
    }

    //Look for mix atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_MIX, LW_RELEVANCE_LOW);
    while (link != NULL)
    {
      regex = link->data;
      if (regex != NULL && g_regex_match (regex, text, 0, &match_info))
      {
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (buffer, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info); match_info = NULL;
      }
      link = link->next;
    }

    //Cleanup
    g_free (text); text = NULL;
}


//!
//! @brief PRIVATE FUNCTION. Moves the content insertion mark to another mark's spot
//!
//! @param search A LwSearch pointer to gleam information from
//! @param name The name of the mark to move the content insertion mark to
//!
static void 
gw_shift_stay_mark (LwSearch *search, char *name)
{
    //Sanity check
    g_assert (lw_search_has_data (search));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextMark *mark;
    GtkTextIter iter;

    //Initializations
    sdata = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

    if ((mark = gtk_text_buffer_get_mark (buffer, name)) == NULL)
      gtk_text_buffer_create_mark (buffer, name, &iter, TRUE);
    else
      gtk_text_buffer_move_mark (buffer, mark, &iter);
}


//!
//! @brief PRIVATE FUNCTION.  Updates the position of a mark to accomidate new results.
//!
//! @param search A LwSearch to gleam information from.
//! @param stay_name The name of the mark that stays in place before the new result.
//! @param append_name The name of the mark that moves to the end after the new result is added.
//!
static void 
gw_searchwindow_shift_append_mark (GwSearchWindow *window, LwSearch *search, char *stay_name, char *append_name)
{
    //Sanity check
    g_assert (lw_search_has_data (search));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *stay_mark, *append_mark;

    //Initializations
    sdata = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    stay_mark = gtk_text_buffer_get_mark (buffer, stay_name);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, stay_mark);

    if ((append_mark = gtk_text_buffer_get_mark (buffer, append_name)) == NULL)
      gtk_text_buffer_create_mark (buffer, append_name, &iter, FALSE);
    else
      gtk_text_buffer_move_mark (buffer, append_mark, &iter);
}


//!
//! @brief PRIVATE FUNCTION.  When adding a result to the buffer, it just adds the the kanji/hiragana section
//!
//! This function is made to help readability of edict results since there is a lot of repeating.
//!
//! @param search A LwSearch pointer to use for sdata.
//!
static void 
gw_searchwindow_append_def_same_to_buffer (GwSearchWindow *window, LwSearch* search, LwResult *result)
{
    //Sanity check
    g_assert (lw_search_has_data (search));

    //Declarations
    GwSearchWindowClass *klass;
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextMark *mark;

    //Initializations
    klass = GW_SEARCHWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    sdata = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);

    gw_searchwindow_shift_append_mark (window, search, "previous_result", "new_result");
    if ((mark = gtk_text_buffer_get_mark (buffer, "previous_result")) != NULL)
    {
      GtkTextIter iter;
      gint line, start_offset, end_offset;
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
      line = gtk_text_iter_get_line (&iter);
      start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " /", -1, "entry-header", NULL);
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      //Kanji
      if (result->kanji_start != NULL)
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->kanji_start, -1, "entry-header", NULL);
      //Furigana
      if (result->furigana_start != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "【", -1, "entry-header", NULL);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->furigana_start, -1, "entry-header", NULL);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "】", -1, "entry-header", NULL);
      }
      //Other info
      if (result->classification_start != NULL)
      {
        gtk_text_buffer_insert (buffer, &iter, " ", -1);
        GtkTextIter copy = iter;
        gtk_text_iter_backward_char (&copy);
        gtk_text_buffer_remove_tag_by_name (buffer, "entry-header", &copy, &iter);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->classification_start, -1, "entry-lexicon", NULL);
      }
      if (result->important == TRUE)
      {
        gtk_text_buffer_insert                   (buffer, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Pop"), -1, "entry-popular", NULL);
      }
      end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);

      gw_searchwindow_insert_edict_addlink (window, result, buffer, &iter);
    }

    g_signal_emit (window, 
      klass->signalid[GW_SEARCHWINDOW_CLASS_SIGNALID_WORD_ADDED], 
      g_quark_from_static_string ("edict"), 
      result
    );
}


gboolean lw_search_next_is_same (LwSearch *search, LwResult *current)
{
  //Declarations
  GwSearchData *sdata;
  LwResult *previous;

  //Initializations
  sdata = GW_SEARCHDATA (lw_search_get_data (search));
  previous = gw_searchdata_get_result (sdata);

  return lw_result_is_similar (previous, current);
}


//!
//! @brief Appends an edict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param search A LwSearch to gleam information from.
//!
static void 
gw_searchwindow_append_edict_result (GwSearchWindow *window, LwSearch *search)
{
    //Sanity check
    if (window == NULL || search == NULL) return;
    g_assert (lw_search_has_data (search));

    //Declarations
    GwSearchWindowClass *klass;
    LwResult *result;
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextMark *mark;
    GtkTextIter iter;
    int line, start_offset, end_offset;

    //Initializations
    klass = GW_SEARCHWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    result = lw_search_get_result (search);
    if (result == NULL) return;
    sdata = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);

    if (lw_search_next_is_same (search, result))
    {
      gw_searchwindow_append_def_same_to_buffer (window, search, result);
      gw_searchdata_set_result (sdata, result);
      return;
    }

    gw_searchdata_set_result (sdata, result);

    //Start output
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    line = gtk_text_iter_get_line (&iter);

    //Kanji
    if (result->kanji_start != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->kanji_start, -1, "entry-header", NULL);
    }

    //Furigana
    if (result->furigana_start != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "【", -1, "entry-header", NULL);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->furigana_start, -1, "entry-header", NULL);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "】", -1, "entry-header", NULL);
    }
    //Other info
    if (result->classification_start != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->classification_start, -1, "entry-lexicon", NULL);
    }
    if (result->important == TRUE)
    {
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Pop"), -1, "entry-popular", NULL);
    }

    gw_searchwindow_insert_edict_addlink (window, result, buffer, &iter);

    gw_shift_stay_mark (search, "previous_result");
    start_offset = 0;
    end_offset = gtk_text_iter_get_line_offset (&iter);

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gw_add_match_highlights (line, start_offset, end_offset, search);

    //Definitions
    int i = 0;
    while (result->def_start[i] != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, "      ", -1);

      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->number[i], -1, "comment", NULL);
      gtk_text_buffer_insert                   (buffer, &iter, " ", -1);
      gtk_text_buffer_insert                   (buffer, &iter, result->def_start[i], -1);
      end_offset = gtk_text_iter_get_line_offset (&iter);
      line = gtk_text_iter_get_line (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);
      gtk_text_buffer_insert                   (buffer, &iter, "\n", -1);
      i++;
    }
    gtk_text_buffer_insert (buffer, &iter, " \n", -1);

    g_signal_emit (window, 
      klass->signalid[GW_SEARCHWINDOW_CLASS_SIGNALID_WORD_ADDED], 
      g_quark_from_static_string ("edict"), 
      result
    );
}


static void
gw_searchwindow_insert_kanjidict_addlink (GwSearchWindow *window, LwResult *result, GtkTextBuffer *buffer, GtkTextIter *iter)
{
    //Declarations
    gchar *kanji, *furigana, *definitions;
    LwWord *word;

    //Initializations
    kanji = result->kanji;
    furigana = g_strjoinv (",", result->readings);
    definitions = result->meanings;

    if (furigana != NULL)
    {
      word = lw_word_new ();
      if (word != NULL)
      {
        lw_word_set_kanji (word, kanji);
        lw_word_set_furigana (word, furigana);
        lw_word_set_definitions (word, definitions);

        gw_searchwindow_insert_addlink (window, buffer, iter, word);

        lw_word_free (word);
      }
      g_free (furigana);
    }
}



//!
//! @brief Appends a kanjidict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param search A LwSearch to gleam information from.
//!
static void 
gw_searchwindow_append_kanjidict_result (GwSearchWindow *window, LwSearch *search)
{
    //Sanity check
    g_assert (lw_search_has_data (search));

    //Declarations
    GwSearchWindowClass *klass;
    GwApplication *application;
    GwSearchData *sdata;
    LwResult *result;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *mark;
    gint line, start_offset, end_offset;

    //Initializations
    klass = GW_SEARCHWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    application = gw_window_get_application (GW_WINDOW (window));
    result = lw_search_get_result (search);
    if (result == NULL) return;
    sdata = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);

    gw_searchdata_set_result (sdata, result);

    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

    //Kanji
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->kanji, -1, "entry-grand-header", NULL);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " ", -1, "entry-grand-header", NULL);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    gw_add_match_highlights (line, start_offset, end_offset, search);

    gw_searchwindow_insert_kanjidict_addlink (window, result, buffer, &iter);

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

    //Radicals
    if (result->radicals != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Radicals:"), -1, "entry-bullet", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (buffer, &iter, result->radicals, -1);
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);

      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    }

    //Readings
    if (result->readings[0] != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Readings:"), -1, "entry-bullet", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (buffer, &iter, result->readings[0], -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    }
    if (result->readings[1] != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Name:"), -1, "entry-bullet", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (buffer, &iter, result->readings[1], -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    }
    if (result->readings[2] != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Radical Name:"), -1, "entry-bullet", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (buffer, &iter, result->readings[2], -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    }


    //etc
    gboolean line_started = FALSE;
    if (result->strokes)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Stroke:"), -1, "entry-bullet", NULL);
      gtk_text_buffer_insert (buffer, &iter, result->strokes, -1);
      line_started = TRUE;
    }
    if (result->frequency)
    {
      if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Freq:"), -1, "entry-bullet", NULL);
      gtk_text_buffer_insert (buffer, &iter, result->frequency, -1);
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
    }
    if (result->grade)
    {
      if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Grade:"), -1, "entry-bullet", NULL);
      gtk_text_buffer_insert (buffer, &iter, result->grade, -1);
    }
    if (result->jlpt)
    {
      if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("JLPT:"), -1, "entry-bullet", NULL);
      gtk_text_buffer_insert (buffer, &iter, result->jlpt, -1);
    }

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

    //Meanings
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Meanings:"), -1, "entry-bullet", NULL);
    gtk_text_buffer_insert (buffer, &iter, result->meanings, -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
    gw_add_match_highlights (line, start_offset, end_offset, search);

    gtk_text_buffer_insert (buffer, &iter, "\n \n", -1);

    g_signal_emit (window, 
      klass->signalid[GW_SEARCHWINDOW_CLASS_SIGNALID_WORD_ADDED], 
      g_quark_from_static_string ("kanjidict"), 
      result
    );

    if (result->radicals != NULL)
    {
      GwRadicalsWindow *radicalswindow;
      radicalswindow =  GW_RADICALSWINDOW (gw_application_get_window_by_type (application, GW_TYPE_RADICALSWINDOW));
      if (radicalswindow != NULL && result->radicals != NULL)
      {
        gw_radicalswindow_update_sensitivities (radicalswindow, result->radicals);
      }
    }
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param search A LwSearch to gleam information from.
//!
static void 
gw_searchwindow_append_examplesdict_result (GwSearchWindow *window, LwSearch *search)
{
    //Sanity check
    g_assert (lw_search_has_data (search));

    //Declarations
    GwSearchWindowClass *klass;
    GwSearchData *sdata;
    LwResult *result;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    int line, start_offset, end_offset;
    GtkTextMark *mark;
    GtkTextIter iter;

    //Initializations
    klass = GW_SEARCHWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    result = lw_search_get_result (search);
    if (result == NULL) return;
    sdata = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

    gw_searchdata_set_result (sdata, result);

    if (result->def_start[0] != NULL)
    {
      // TRANSLATORS: The "E" stands for "English"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("E:\t"), -1, "entry-header", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->def_start[0], -1, "entry-header", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);
    }

    if (result->kanji_start != NULL)
    {
      // TRANSLATORS: The "J" stands for "Japanese"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("\nJ:\t"), -1, "entry-header", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->kanji_start, -1, "entry-example-definition", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);
    }

    if (result->furigana_start != NULL)
    {
      // TRANSLATORS: The "D" stands for "Detail"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("\nD:\t"), -1, "entry-header", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, result->furigana_start, -1, "entry-example-definition", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, search);
    }

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    gtk_text_buffer_insert (buffer, &iter, "\n \n", -1);

    g_signal_emit (window, 
      klass->signalid[GW_SEARCHWINDOW_CLASS_SIGNALID_WORD_ADDED], 
      g_quark_from_static_string ("examplesdict"), 
      result
    );

}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly.  This is the fallback safe function for unknown dictionaries.
//!
//! @param search A LwSearch to gleam information from.
//!
static void 
gw_searchwindow_append_unknowndict_result (GwSearchWindow *window, LwSearch *search)
{
    //Sanity check
    g_assert (lw_search_has_data (search));

    //Definitions
    GwSearchWindowClass *klass;
    LwResult *result;
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *mark;
    int line, start_offset, end_offset;


    //Initializations
    klass = GW_SEARCHWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    result = lw_search_get_result (search);
    if (result == NULL) return;
    sdata = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");

    gw_searchdata_set_result (sdata, result);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_insert (buffer, &iter, result->text, -1);
    gtk_text_buffer_insert (buffer, &iter, " ", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    gw_add_match_highlights (line, start_offset, end_offset, search);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    gtk_text_buffer_insert (buffer, &iter, "\n \n", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

    g_signal_emit (window, 
      klass->signalid[GW_SEARCHWINDOW_CLASS_SIGNALID_WORD_ADDED], 
      g_quark_from_static_string ("unknowndict"), 
      result
    );
}


void 
gw_searchwindow_append_kanjidict_tooltip_result (GwSearchWindow *window, LwSearch *search)
{
    if (search == NULL) return;
    //Declarations
    GwSearchWindowPrivate *priv;
    LwResult *result;
    GtkTextView *view;
    gchar *markup;
    gchar *new;
    gchar *base;
    gint x, y, width, height;

    //Declarations
    GtkWidget *child;
    GtkWidget *label;
    GtkWidget *tooltip_window;
    GtkWidget *hbox;
    char *markup2;

    //Initializations
    result = lw_search_get_result (search);
    if (result == NULL) return;
    priv = window->priv;
    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) return;
    markup = g_strdup ("");
    new = NULL;
    base = NULL;
    x = priv->mouse_button_press_root_x + 3;
    y = priv->mouse_button_press_root_y + 3;

    if (result->radicals) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Radicals:"), result->radicals);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (result->readings[0]) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Readings:"), result->readings[0]);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    } 
    if (result->readings[1]) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Readings:"), result->readings[1]);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (result->readings[2]) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s: </b>%s\n", gettext("Radical Name"), result->readings[2]);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }

    if (result->strokes) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Stroke:"), result->strokes);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (result->frequency) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Freq:"), result->frequency);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (result->grade) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Grade:"), result->grade);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (result->jlpt) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("JLPT:"), result->jlpt);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }

    base = markup;
    new = g_markup_printf_escaped ("\n");
    markup = g_strjoin ("", base, new, NULL);
    g_free (base);
    base = NULL;
    g_free (new);
    new = NULL;

    if (result->meanings) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s", gettext("Meanings:"), result->meanings);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }

    markup2 = g_markup_printf_escaped ("<span font=\"KanjiStrokeOrders 80\">%s</span>", result->kanji);
    tooltip_window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));

    if (tooltip_window != NULL) {
      gtk_widget_destroy (tooltip_window);
      tooltip_window = NULL;
    }

    tooltip_window = gtk_window_new (GTK_WINDOW_POPUP);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (tooltip_window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (tooltip_window), TRUE);
    gtk_window_set_accept_focus (GTK_WINDOW (tooltip_window), FALSE);
    gtk_window_set_transient_for (GTK_WINDOW (tooltip_window), NULL);
    gtk_window_set_type_hint (GTK_WINDOW (tooltip_window), GDK_WINDOW_TYPE_HINT_TOOLTIP);
    gtk_widget_set_name (GTK_WIDGET (tooltip_window), "gtk-tooltip");
    gtk_widget_set_tooltip_window (GTK_WIDGET (view), GTK_WINDOW (tooltip_window));
    gtk_window_set_gravity (GTK_WINDOW (tooltip_window), GDK_GRAVITY_NORTH_WEST);
    gtk_window_set_position (GTK_WINDOW (tooltip_window), GTK_WIN_POS_NONE);
    gtk_window_move (GTK_WINDOW (tooltip_window), x, y);

    child = gtk_bin_get_child (GTK_BIN (tooltip_window));
    if (child != NULL) gtk_widget_destroy (GTK_WIDGET (child));

    hbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3));
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
    gtk_container_add (GTK_CONTAINER (tooltip_window), GTK_WIDGET (hbox));

    label = GTK_WIDGET (gtk_label_new (NULL));
    gtk_label_set_markup (GTK_LABEL (label), markup2);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, TRUE, 3);
    gtk_label_set_selectable (GTK_LABEL (label), TRUE);
    gtk_widget_set_can_focus (GTK_WIDGET (label), FALSE);

    label = GTK_WIDGET (gtk_label_new (NULL));
    gtk_label_set_markup (GTK_LABEL (label), markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, TRUE, 0);
    gtk_label_set_selectable (GTK_LABEL (label), TRUE);
    gtk_widget_set_can_focus (GTK_WIDGET (label), FALSE);

    gtk_widget_show_all (GTK_WIDGET (tooltip_window));

    //Move the tooltip onscreen if it is partially off
    width = gtk_widget_get_allocated_width (GTK_WIDGET (tooltip_window));
    height =gtk_widget_get_allocated_height (GTK_WIDGET (tooltip_window));
    if (gdk_screen_width() < x + width)
      x = gdk_screen_width() - width;
    if (gdk_screen_height() < y + height)
      y = gdk_screen_height() - height;
    gtk_window_move (GTK_WINDOW (tooltip_window), x, y);


    //Cleanup
    g_free (markup);
    g_free (markup2);
    lw_result_free (result);
    lw_search_cancel (search);
    lw_search_free (priv->mouse_item);
    priv->mouse_item = NULL;
}


//!
//! @brief Sets the no results page to the output buffer
//!
//! @param search A LwSearch pointer to gleam information from
//!
void 
gw_searchwindow_display_no_results_found_page (GwSearchWindow *window, LwSearch *search)
{
    //Sanity check
    if (search == NULL || search->dictionary == NULL) return;
    if (lw_search_get_progress (search) > 0.0 || lw_search_get_total_results (search) > 0) return; 

    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    priv = window->priv;
    LwDictionaryList *dictionarylist;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkWidget *image = NULL;
    GtkTextIter iter;
    GtkTextChildAnchor *anchor = NULL;
    GtkWidget *label = NULL;
    GtkWidget *box = NULL;
    const gchar *query_text;
    gint i = 0;
    GtkWidget *button = NULL;
    gchar *markup = NULL;
    LwDictionary *dictionary_selected;
    LwDictionary *dictionary;
    GwSearchData *sdata;
    gint position;
    const gchar *shortname;

    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = LW_DICTIONARYLIST (gw_application_get_installed_dictionarylist (application));
    sdata = (GwSearchData*) lw_search_get_data (search);
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    query_text = gtk_entry_get_text (priv->entry);
    dictionary_selected = gw_searchwindow_get_dictionary (window);
    search->current = 0L;
    shortname = lw_dictionary_get_name (dictionary_selected);

    gtk_text_buffer_set_text (buffer, "", -1);

    //Add the title
    gw_searchwindow_append_to_buffer (window, search, "\n", "spacing", NULL, NULL, NULL);

    //Set the header message
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_text_buffer_get_end_iter (buffer, &iter);
    anchor = gtk_text_buffer_create_child_anchor (buffer, &iter);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (view), box, anchor);
    gtk_widget_show (box);

    image = gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_DIALOG);
    gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (image));
    gtk_widget_show (image);

    label = gtk_label_new (NULL);
    char *message = NULL;
    // TRANSLATORS: The argument is the dictionary long name
    message = g_strdup_printf (gettext("Nothing found in the %s Dictionary!"), shortname);
    if (message != NULL)
    {
      markup = g_markup_printf_escaped ("<big><big><b>%s</b></big></big>", message);
      if (markup != NULL)
      {
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (label));
        gtk_widget_show (label);
        g_free (markup);
        markup = NULL;
      }
      g_free (message);
      message = NULL;
    }


    //Linebreak after the image
    gw_searchwindow_append_to_buffer (window, search, "\n \n \n", NULL, NULL, NULL, NULL);


    if (lw_dictionarylist_get_total (dictionarylist) > 1)
    {
      //Add label for links
      box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_text_buffer_get_end_iter (buffer, &iter);
      anchor = gtk_text_buffer_create_child_anchor (buffer, &iter);
      gtk_text_view_add_child_at_anchor (view, box, anchor);
      gtk_widget_show (box);

      label = gtk_label_new (NULL);
      markup = g_markup_printf_escaped ("<b>%s</b>", gettext("Search Other Dictionary: "));
      if (markup != NULL)
      {
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (label));
        gtk_widget_show (label);
        g_free (markup);
        markup = NULL;
      }

      //Add internal dictionary links
      i = 0;
      dictionary = lw_dictionarylist_get_dictionary_by_position (dictionarylist, 0);

      while ((dictionary = lw_dictionarylist_get_dictionary_by_position (dictionarylist, i)) != NULL)
      {
        if (dictionary != dictionary_selected)
        {
          shortname = lw_dictionary_get_name (dictionary);
          button = gtk_button_new_with_label (shortname);
          position = lw_dictionarylist_get_position (dictionarylist, dictionary);
          gtk_widget_set_margin_left (GTK_WIDGET (button), 2);
          gtk_widget_set_margin_right (GTK_WIDGET (button), 2);
          g_object_set_data (G_OBJECT (button), "load-position", GINT_TO_POINTER (position));
          g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_searchwindow_no_results_search_for_dictionary_cb), window);
          gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (button));
          gtk_widget_show (GTK_WIDGET (button));
        }
        i++;
      }

      gw_searchwindow_append_to_buffer (window, search, "\n", NULL, NULL, NULL, NULL);
    }

    //Add label for links
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_text_buffer_get_end_iter (buffer, &iter);
    anchor = gtk_text_buffer_create_child_anchor (buffer, &iter);
    gtk_text_view_add_child_at_anchor (view, box, anchor);
    gtk_widget_show (box);

    label = gtk_label_new (NULL);
    markup = g_markup_printf_escaped ("<b>%s</b>", gettext("Search Online: "));
    if (markup != NULL)
    {
      gtk_label_set_markup (GTK_LABEL (label), markup);
      gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (label));
      gtk_widget_show (label);
      g_free (markup);
      markup = NULL;
    }


    //Add links
    char *website_url_menuitems[] = {
      "Google", "http://www.google.com/?q=%s", "google.png",
      "Goo", "http://dictionary.goo.ne.jp/srch/all/%s/m0u/", "goo.png",
      "Wikipedia", "http://www.wikipedia.org/wiki/%s", "wikipedia.png",
      NULL, NULL, NULL
    };
    i = 0;
    while (website_url_menuitems[i] != NULL)
    {
      //Create handy variables
      char *name = website_url_menuitems[i];
      char *url = g_strdup_printf(website_url_menuitems[i + 1], query_text);
      char *icon_path = website_url_menuitems[i + 2];
#ifndef G_OS_WIN32
      gchar *path = g_build_filename (DATADIR2, PACKAGE, icon_path, NULL);
#else
      gchar *prefix = g_win32_get_package_installation_directory_of_module (NULL);
      gchar *path = g_build_filename (prefix, "share", PACKAGE, icon_path, NULL);
      g_free (prefix);
#endif
      image = NULL;

      //Start creating
      button = gtk_link_button_new_with_label (url, name);
      if (path != NULL)
      {
        image = gtk_image_new_from_file (path);
        //Gtk doesn't use the image anymore by default so we are removing
        //if (image != NULL) gtk_button_set_image (GTK_BUTTON (button), image);
        g_free (path);
        path = NULL;
      }
      gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (button));
      gtk_widget_show (button);
      i += 3;
    }

}

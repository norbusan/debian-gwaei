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


#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/searchwindow-private.h>

static void gw_searchwindow_append_edict_result (GwSearchWindow*, LwSearchItem*);
static void gw_searchwindow_append_kanjidict_result (GwSearchWindow*, LwSearchItem*);
static void gw_searchwindow_append_examplesdict_result (GwSearchWindow*, LwSearchItem*);
static void gw_searchwindow_append_unknowndict_result (GwSearchWindow*, LwSearchItem*);
static void gw_searchwindow_append_less_relevant_header (GwSearchWindow*, LwSearchItem*);
static void gw_searchwindow_append_more_relevant_header (GwSearchWindow*, LwSearchItem*);


//!
//! @brief Appends a result to the output
//! @param engine The LwEngine to use for output
//! @param item The data from the LwSearchItem
//!
void 
gw_searchwindow_append_result (GwSearchWindow *window, LwSearchItem* item)
{
    if (window == NULL || item == NULL) return;

    switch (item->dictionary->type)
    {
      case LW_DICTTYPE_EDICT:
        gw_searchwindow_append_edict_result (window, item);
        break;
      case LW_DICTTYPE_KANJI:
        gw_searchwindow_append_kanjidict_result (window, item);
        break;
      case LW_DICTTYPE_EXAMPLES:
        gw_searchwindow_append_examplesdict_result (window, item);
        break;
      case LW_DICTTYPE_UNKNOWN:
        gw_searchwindow_append_unknowndict_result (window, item);
        break;
      default:
        g_assert_not_reached ();
        break;
    }
}




//!
//! @brief PRIVATE FUNCTION. A Stes the text of the desired mark.
//!
//! @param item A LwSearchItem to gleam information from
//! @param text The desired text to set to the mark
//! @param mark_name The name of the mark to set the new attributes to
//!
//!
static void 
gw_searchwindow_set_header (LwSearchItem *item, char* text, char* mark_name)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *mark;
    gint line;
    char *new_text;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    mark = gtk_text_buffer_get_mark (buffer, mark_name);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    line = gtk_text_iter_get_line (&iter);

    //Move the insertion header to the less relevenant section
    if (strcmp(mark_name, "less_relevant_header_mark") == 0)
    {
      GtkTextMark *target_mark;
      GtkTextIter iter;
      target_mark = gtk_text_buffer_get_mark (buffer, "less_rel_content_insertion_mark");
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, target_mark);
      gtk_text_buffer_move_mark_by_name (buffer, "content_insertion_mark", &iter);
    }

    //Update the header text
    new_text = g_strdup_printf ("%s\n", text);
    if (new_text != NULL)
    {
      GtkTextIter end_iter;
      gtk_text_buffer_get_iter_at_line(buffer, &end_iter, line + 1);
      gtk_text_buffer_delete (buffer, &iter, &end_iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, new_text, -1, "header", "important", NULL);
      g_free (new_text);
      new_text = NULL;
    }
}


//!
//! @brief PRIVATE FUNCTION. Applies a tag to a section of text
//!
//! @param line An integer showing the line in the buffer to tag
//! @param start_offset the ending character in the line to highlight
//! @param end_offset The ending character in the line to highlight
//! @param item A LwSearchItem to get general information from
//!
static void 
gw_add_match_highlights (gint line, gint start_offset, gint end_offset, LwSearchItem* item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    LwQueryLine *ql;
    int match_start_byte_offset;
    int match_end_byte_offset;
    int match_character_offset;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    char *text;
    GRegex *re;
    GRegex ***iter;
    GMatchInfo *match_info;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    ql = item->queryline;
    gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, start_offset);
    gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, end_offset);
    text = gtk_text_buffer_get_slice (buffer, &start_iter, &end_iter, FALSE);

    //Look for kanji atoms
    for (iter = ql->re_kanji; *iter != NULL && **iter != NULL; iter++)
    {
      re = (*iter)[LW_RELEVANCE_LOCATE];
      if (g_regex_match (re, text, 0, &match_info))
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
      }
      g_match_info_free (match_info);
    }

    //Look for furigana atoms
    for (iter = ql->re_furi; *iter != NULL && **iter != NULL; iter++)
    {
      re = (*iter)[LW_RELEVANCE_LOCATE];
      if (g_regex_match (re, text, 0, &match_info))
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
      }
      g_match_info_free (match_info);
    }

    //Look for romaji atoms
    for (iter = ql->re_roma; *iter != NULL; iter++)
    {
      re = (*iter)[LW_RELEVANCE_LOCATE];
      if (re != NULL)
      { 
        if (g_regex_match (re, text, 0, &match_info))
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
        }
        g_match_info_free (match_info);
      }
    }

    //Cleanup
    g_free (text);
}


//!
//! @brief PRIVATE FUNCTION. Moves the content insertion mark to another mark's spot
//!
//! @param item A LwSearchItem pointer to gleam information from
//! @param name The name of the mark to move the content insertion mark to
//!
static void 
gw_shift_stay_mark (LwSearchItem *item, char *name)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextMark *mark;
    GtkTextIter iter;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
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
//! @param item A LwSearchItem to gleam information from.
//! @param stay_name The name of the mark that stays in place before the new result.
//! @param append_name The name of the mark that moves to the end after the new result is added.
//!
static void 
gw_searchwindow_shift_append_mark (GwSearchWindow *window, LwSearchItem *item, char *stay_name, char *append_name)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *stay_mark, *append_mark;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
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
//! @param item A LwSearchItem pointer to use for sdata.
//!
static void 
gw_searchwindow_append_def_same_to_buffer (GwSearchWindow *window, LwSearchItem* item, LwResultLine *resultline)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextMark *mark;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);

    gw_searchwindow_shift_append_mark (window, item, "previous_result", "new_result");
    if ((mark = gtk_text_buffer_get_mark (buffer, "previous_result")) != NULL)
    {
      GtkTextIter iter;
      int line, start_offset, end_offset;
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
      line = gtk_text_iter_get_line (&iter);
      start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " /", -1, "important", NULL);
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      //Kanji
      if (resultline->kanji_start != NULL)
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->kanji_start, -1, "important", NULL);
      //Furigana
      if (resultline->furigana_start != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " [", -1, "important", NULL);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->furigana_start, -1, "important", NULL);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "]", -1, "important", NULL);
      }
      //Other info
      if (resultline->classification_start != NULL)
      {
        gtk_text_buffer_insert (buffer, &iter, " ", -1);
        GtkTextIter copy = iter;
        gtk_text_iter_backward_char (&copy);
        gtk_text_buffer_remove_tag_by_name (buffer, "important", &copy, &iter);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->classification_start, -1, "gray", "italic", NULL);
      }
      if (resultline->important == TRUE)
      {
        gtk_text_buffer_insert                   (buffer, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Pop"), -1, "small", NULL);
      }
      end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);
    }
}


gboolean lw_searchitem_next_is_same (LwSearchItem *item, LwResultLine *current)
{
  //Declarations
  GwSearchData *sdata;
  LwResultLine *previous;

  //Initializations
  sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
  previous = gw_searchdata_get_resultline (sdata);

  return lw_resultline_is_similar (previous, current);
}


//!
//! @brief Appends an edict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
static void 
gw_searchwindow_append_edict_result (GwSearchWindow *window, LwSearchItem *item)
{
    //Sanity check
    if (window == NULL || item == NULL) return;
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    LwResultLine *resultline;
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextMark *mark;
    GtkTextIter iter;
    int line, start_offset, end_offset;

    //Initializations
    resultline = lw_searchitem_get_result (item);
    if (resultline == NULL) return;
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);

    if (lw_searchitem_next_is_same (item, resultline))
    {
      gw_searchwindow_append_def_same_to_buffer (window, item, resultline);
      gw_searchdata_set_resultline (sdata, resultline);
      return;
    }

    gw_searchdata_set_resultline (sdata, resultline);

    switch (resultline->relevance)
    {
      case LW_RESULTLINE_RELEVANCE_HIGH:
        gw_searchwindow_append_more_relevant_header (window, item);
        break;
      case LW_RESULTLINE_RELEVANCE_MEDIUM:
      case LW_RESULTLINE_RELEVANCE_LOW:
        gw_searchwindow_append_less_relevant_header (window, item);
        break;
      default:
        g_assert_not_reached ();
        break;
    }

    //Start output
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    line = gtk_text_iter_get_line (&iter);

    //Kanji
    if (resultline->kanji_start != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->kanji_start, -1, "important", NULL);
    }
    //Furigana
    if (resultline->furigana_start != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " [", -1, "important", NULL);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->furigana_start, -1, "important", NULL);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "]", -1, "important", NULL);
    }
    //Other info
    if (resultline->classification_start != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->classification_start, -1, "gray", "italic", NULL);
    }
    if (resultline->important == TRUE)
    {
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Pop"), -1, "small", NULL);
    }

    gw_shift_stay_mark (item, "previous_result");
    start_offset = 0;
    end_offset = gtk_text_iter_get_line_offset (&iter);

//    gw_searchwindow_insert_resultpopup_button (window, item, resultline, &iter);

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gw_add_match_highlights (line, start_offset, end_offset, item);

    //Definitions
    int i = 0;
    while (resultline->def_start[i] != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, "      ", -1);

      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->number[i], -1, "comment", NULL);
      gtk_text_buffer_insert                   (buffer, &iter, " ", -1);
      gtk_text_buffer_insert                   (buffer, &iter, resultline->def_start[i], -1);
      end_offset = gtk_text_iter_get_line_offset (&iter);
      line = gtk_text_iter_get_line (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);
      gtk_text_buffer_insert                   (buffer, &iter, "\n", -1);
      i++;
    }
    gtk_text_buffer_insert (buffer, &iter, "\n", -1);

}


//!
//! @brief Appends a kanjidict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
static void 
gw_searchwindow_append_kanjidict_result (GwSearchWindow *window, LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwApplication *application;
    GwSearchData *sdata;
    LwResultLine *resultline;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *mark;
    int line, start_offset, end_offset;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    resultline = lw_searchitem_get_result (item);
    if (resultline == NULL) return;
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);

    gw_searchdata_set_resultline (sdata, resultline);

    switch (resultline->relevance)
    {
      case LW_RESULTLINE_RELEVANCE_HIGH:
        gw_searchwindow_append_more_relevant_header (window, item);
        break;
      case LW_RESULTLINE_RELEVANCE_MEDIUM:
      case LW_RESULTLINE_RELEVANCE_LOW:
        gw_searchwindow_append_less_relevant_header (window, item);
        break;
      default:
        g_assert_not_reached ();
        break;
    }

    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

    //Kanji
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->kanji, -1, "large", "center", NULL);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " ", -1, "large", "center", NULL);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    gw_add_match_highlights (line, start_offset, end_offset, item);

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

    //Radicals
    if (resultline->radicals != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Radicals:"), -1, "important", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (buffer, &iter, resultline->radicals, -1);
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

      GwRadicalsWindow *radicalswindow;
      radicalswindow =  GW_RADICALSWINDOW (gw_application_get_window_by_type (application, GW_TYPE_RADICALSWINDOW));
      if (radicalswindow != NULL && resultline->radicals != NULL)
      {
        gw_radicalswindow_set_button_sensitive_when_label_is (radicalswindow, resultline->radicals);
      }
    }

    //Readings
    if (resultline->readings[0] != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Readings:"), -1, "important", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (buffer, &iter, resultline->readings[0], -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    }
    if (resultline->readings[1] != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Name:"), -1, "important", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (buffer, &iter, resultline->readings[1], -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    }
    if (resultline->readings[2] != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Radical Name:"), -1, "important", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (buffer, &iter, resultline->readings[2], -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    }


    //etc
    gboolean line_started = FALSE;
    if (resultline->strokes)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Stroke:"), -1, "important", NULL);
      gtk_text_buffer_insert (buffer, &iter, resultline->strokes, -1);
      line_started = TRUE;
    }
    if (resultline->frequency)
    {
      if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Freq:"), -1, "important", NULL);
      gtk_text_buffer_insert (buffer, &iter, resultline->frequency, -1);
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
    }
    if (resultline->grade)
    {
      if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Grade:"), -1, "important", NULL);
      gtk_text_buffer_insert (buffer, &iter, resultline->grade, -1);
    }
    if (resultline->jlpt)
    {
      if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("JLPT:"), -1, "important", NULL);
      gtk_text_buffer_insert (buffer, &iter, resultline->jlpt, -1);
    }

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

    //Meanings
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Meanings:"), -1, "important", NULL);
    gtk_text_buffer_insert (buffer, &iter, resultline->meanings, -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
    gw_add_match_highlights (line, start_offset, end_offset, item);

    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
static void 
gw_searchwindow_append_examplesdict_result (GwSearchWindow *window, LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    LwResultLine *resultline;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    int line, start_offset, end_offset;
    GtkTextMark *mark;
    GtkTextIter iter;

    //Initializations
    resultline = lw_searchitem_get_result (item);
    if (resultline == NULL) return;
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

    gw_searchdata_set_resultline (sdata, resultline);

    if (resultline->def_start[0] != NULL)
    {
      // TRANSLATORS: The "E" stands for "English"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("E:\t"), -1, "important", "comment", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->def_start[0], -1, "important", NULL, NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);
    }

    if (resultline->kanji_start != NULL)
    {
      // TRANSLATORS: The "J" stands for "Japanese"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("\nJ:\t"), -1, "important", "comment", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->kanji_start, -1, NULL, NULL, NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);
    }

    if (resultline->furigana_start != NULL)
    {
      // TRANSLATORS: The "D" stands for "Detail"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("\nD:\t"), -1, "important", "comment", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->furigana_start, -1, NULL, NULL, NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      gw_add_match_highlights (line, start_offset, end_offset, item);
    }

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly.  This is the fallback safe function for unknown dictionaries.
//!
//! @param item A LwSearchItem to gleam information from.
//!
static void 
gw_searchwindow_append_unknowndict_result (GwSearchWindow *window, LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Definitions
    LwResultLine *resultline;
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *mark;
    int line, start_offset, end_offset;


    //Initializations
    resultline = lw_searchitem_get_result (item);
    if (resultline == NULL) return;
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");

    gw_searchdata_set_resultline (sdata, resultline);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_insert (buffer, &iter, resultline->string, -1);
    gtk_text_buffer_insert (buffer, &iter, " ", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    gw_add_match_highlights (line, start_offset, end_offset, item);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
}


//!
//! @brief Add an header to irrelevant "other" results with number of matches
//!
static void 
gw_searchwindow_append_less_relevant_header (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    int irrelevant;
    char *message;

    //Initializations
    irrelevant = item->total_irrelevant_results;
    message = g_strdup_printf (ngettext("Other Result %d", "Other Results %d", irrelevant), irrelevant);

    if (message != NULL)
    {
      gw_searchwindow_set_header (item, message, "less_relevant_header_mark");
      g_free (message);
    }
}


//!
//! @brief Add an header to relevant "main" results with number of matches
//!
static void 
gw_searchwindow_append_more_relevant_header (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    int relevant;
    char *message;

    //Initializations
    relevant = item->total_relevant_results;
    message = g_strdup_printf (ngettext("Main Result %d", "Main Results %d", relevant), relevant);

    if (message != NULL)
    {
      gw_searchwindow_set_header (item, message, "more_relevant_header_mark");
      g_free (message);
    }
}


void 
gw_searchwindow_append_kanjidict_tooltip_result (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    LwResultLine *resultline;
    GtkTextView *view;
    char *markup;
    char *new;
    char *base;
    int x, y, width, height;

    //Declarations
    GtkWidget *child;
    GtkWidget *label;
    GtkWidget *tooltip_window;
    GtkWidget *hbox;
    char *markup2;

    //Initializations
    resultline = lw_searchitem_get_result (item);
    if (resultline == NULL) return;
    priv = window->priv;
    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) return;
    markup = g_strdup ("");
    new = NULL;
    base = NULL;
    x = priv->mouse_button_press_root_x + 3;
    y = priv->mouse_button_press_root_y + 3;

    if (resultline->radicals) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Radicals:"), resultline->radicals);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (resultline->readings[0]) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Readings:"), resultline->readings[0]);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    } 
    if (resultline->readings[1]) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Readings:"), resultline->readings[1]);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (resultline->readings[2]) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s: </b>%s\n", gettext("Radical Name"), resultline->readings[2]);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }

    if (resultline->strokes) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Stroke:"), resultline->strokes);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (resultline->frequency) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Freq:"), resultline->frequency);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (resultline->grade) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Grade:"), resultline->grade);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }
    if (resultline->jlpt) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("JLPT:"), resultline->jlpt);
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

    if (resultline->meanings) {
      base = markup;
      new = g_markup_printf_escaped ("<b>%s </b>%s", gettext("Meanings:"), resultline->meanings);
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;
    }

    markup2 = g_markup_printf_escaped ("<span font=\"KanjiStrokeOrders 80\">%s</span>", resultline->kanji);
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
    lw_resultline_free (resultline);
    lw_searchitem_cancel_search (item);
    lw_searchitem_free (priv->mouse_item);
    priv->mouse_item = NULL;
}


//!
//! @brief Sets the no results page to the output buffer
//!
//! @param item A LwSearchItem pointer to gleam information from
//!
void 
gw_searchwindow_display_no_results_found_page (GwSearchWindow *window, LwSearchItem *item)
{
    //Sanity check
    if (item == NULL || item->dictionary == NULL) return;
    if (item->status != LW_SEARCHSTATUS_IDLE || item->current == 0L || item->total_results > 0) return; 

    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    priv = window->priv;
    LwDictInfoList *dictinfolist;
    gint32 temp = g_random_int_range (0,9);
    while (temp == priv->previous_tip)
      temp = g_random_int_range (0,9);
    const gint32 TIP_NUMBER = temp;
    priv->previous_tip = temp;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkWidget *image = NULL;
    GtkTextIter iter;
    GtkTextChildAnchor *anchor = NULL;
    GtkWidget *label = NULL;
    GtkWidget *box = NULL;
    char *body = NULL;
    const char *query_text;
    int i = 0;
    GtkWidget *button = NULL;
    char *markup = NULL;
    LwDictInfo *di_selected;
    LwDictInfo *di;
    GwSearchData *sdata;

    application = gw_window_get_application (GW_WINDOW (window));
    dictinfolist = LW_DICTINFOLIST (gw_application_get_dictinfolist (application));
    sdata = (GwSearchData*) lw_searchitem_get_data (item);
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    query_text = gtk_entry_get_text (priv->entry);
    di_selected = gw_searchwindow_get_dictionary (window);
    item->current = 0L;

    gtk_text_buffer_set_text (buffer, "", -1);

    //Add the title
    gw_searchwindow_append_to_buffer (window, item, "\n", "small", NULL, NULL, NULL);

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
    message = g_strdup_printf(gettext("Nothing found in the %s!"), di_selected->longname);
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
    gw_searchwindow_append_to_buffer (window, item, "\n\n\n", NULL, NULL, NULL, NULL);


    if (lw_dictinfolist_get_total (dictinfolist) > 1)
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
      di = lw_dictinfolist_get_dictinfo_by_load_position (dictinfolist, 0);

      while ((di = lw_dictinfolist_get_dictinfo_by_load_position (dictinfolist, i)) != NULL)
      {
        if (di != di_selected)
        {
          button = gtk_button_new_with_label (di->shortname);
          g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_searchwindow_no_results_search_for_dictionary_cb), di);
          gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (button));
          gtk_widget_show (GTK_WIDGET (button));
        }
        i++;
      }

      gw_searchwindow_append_to_buffer (window, item, "\n", NULL, NULL, NULL, NULL);
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
      "Google", "http://www.google.com/window?q=%s", "google.png",
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
      char *path = g_build_filename (DATADIR2, PACKAGE, icon_path, NULL);
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




    gw_searchwindow_append_to_buffer (window, item, "\n\n\n", NULL, NULL, NULL, NULL);




    //Insert the instruction text
    char *tip_header_str = NULL;
    tip_header_str = g_strdup_printf (gettext("gWaei Usage Tip #%d: "), (TIP_NUMBER + 1));
    if (tip_header_str != NULL)
    {
      gw_searchwindow_append_to_buffer (window, item, tip_header_str,
                              "important", NULL, NULL, NULL         );
      g_free (tip_header_str);
      tip_header_str = NULL;
    }
                            
    switch (TIP_NUMBER)
    {
      case 0:
        //Tip 1
        body = g_strdup_printf (gettext("Use the Unknown Character from the Insert menu or toolbar in "
                                "place of unknown Kanji. %s will return results like %s.\n\nKanjipad "
                                "is another option for inputting Kanji characters.  Because of how the "
                                "innards of Kanjipad works, drawing with the correct number of strokes "
                                "and drawing the strokes in the correct direction is very important."),
                                "日.語", "日本語");
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Inputting Unknown Kanji"),
                                "header", "important", NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        if (body != NULL)
        {
          gw_searchwindow_append_to_buffer (window, item, body,
                                  NULL, NULL, NULL, NULL);
          g_free (body);
          body = NULL;
        }
        break;

     case 1:
        //Tip 2
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Getting More Exact Matches"),
                                "important", "header", NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Use the Word-edge Mark and the Not-word-edge Mark from the "
                                "insert menu to get more relevant results.  fish\\b will return results "
                                "like fish and selfish , but not fisherman"),
                                NULL, NULL, NULL, NULL);
        break;

     case 2:
        //Tip 3
        body = g_strdup_printf (gettext("Use the And Character or Or Character to window for "
                                "results that contain a combination of words that might not be "
                                "right next to each other.  cats&dogs will return only results "
                                "that contain both the words cats and dogs like %s does."),
                                "犬猫");
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Searching for Multiple Words"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        if (body != NULL)
        {
          gw_searchwindow_append_to_buffer (window, item, body,
                                  NULL, NULL, NULL, NULL);
          g_free (body);
          body = NULL;
        }
        break;

     case 3:
        //Tip 4
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Make a Vocabulary List"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Specific sections of results can be printed or saved by "
                                "dragging the mouse to highlight them.  Using this in combination "
                                "with the Append command from the File menu or toolbar, quick and "
                                "easy creation of a vocabulary lists is possible."),
                                NULL, NULL, NULL, NULL);
        break;

     case 4:
        //Tip 5
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Why Use the Mouse?"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Typing something will move the focus to the window input "
                                "box.  Hitting the Up or Down arrow key will move the focus to the "
                                "results pane so you can scroll the results.  Hitting Alt-Up or "
                                "Alt-Down will cycle the currently installed dictionaries."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 5:
        //Tip 6
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Get Ready for the JLPT"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("The Kanji dictionary has some hidden features.  One such "
                                "one is the ability to filter out Kanji that don't meet a certain "
                                "criteria.  If you are planning on taking the Japanese Language "
                                "Proficiency Test, using the phrase J# will filter out Kanji not of "
                                "that level for easy study.  For example, J4 will only show Kanji "
                                "that appears on the forth level test.\n\nAlso of interest, the "
                                "phrase G# will filter out Kanji for the grade level a Japanese "
                                "person would study it at in school."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 6:
        //Tip 7
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Just drag words in!"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("If you drag and drop a highlighted word into gWaei's "
                                "window result box, gWaei will automatically start a window "
                                "using that text.  This can be a nice way to quickly look up words "
                                "while browsing webpages. "),
                                NULL, NULL, NULL, NULL         );

        break;

     case 7:
        //Tip 8
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("What does (adj-i) mean?"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("It is part of the terminalogy used by the EDICT group of "
                                "dictionaries to categorize words.  Some are obvious, but there are "
                                "a number that there is no way to know the meaning other than by looking "
                                "it up.\n\ngWaei includes some of the EDICT documentation in its help "
                                "manual.  Click the Dictionary Terminology Glossary menuitem in the "
                                "Help menu to get to it."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 8:
        //Tip 9
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Books are Heavy"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Aways wear a construction helmet when working with books.  "
                                "They are dangerous heavy objects that can at any point fall on and "
                                "injure you.  Please all urge all of your friends to, too.  They will "
                                "thank you later.  Really."),
                                NULL, NULL, NULL, NULL         );
       break;
    }

    gw_searchwindow_append_to_buffer (window, item,
                               "\n\n",
                               NULL, NULL, NULL, NULL         );
}

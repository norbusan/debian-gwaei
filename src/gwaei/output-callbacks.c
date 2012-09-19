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
//! @file src/gtk/output-callbacks.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! This is the gtk version.
//!


#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief PRIVATE FUNCTION. A Stes the text of the desired mark.
//!
//! @param item A LwSearchItem to gleam information from
//! @param text The desired text to set to the mark
//! @param mark_name The name of the mark to set the new attributes to
//!
//!
static void _set_header (LwSearchItem *item, char* text, char* mark_name)
{
  gdk_threads_enter();
    //Declarations
    GObject *results_tb;
    GtkTextIter iter;
    GtkTextMark *mark;
    gint line;
    char *new_text;

    //Initializations
    results_tb = G_OBJECT (item->target_tb);
    mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), mark_name);
    gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, mark);
    line = gtk_text_iter_get_line (&iter);

    //Move the insertion header to the less relevenant section
    if (strcmp(mark_name, "less_relevant_header_mark") == 0)
    {
      GtkTextMark *target_mark;
      GtkTextIter iter;
      target_mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), "less_rel_content_insertion_mark");
      gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, target_mark);
      gtk_text_buffer_move_mark_by_name (GTK_TEXT_BUFFER (results_tb), "content_insertion_mark", &iter);
    }

    //Update the header text
    new_text = g_strdup_printf ("%s\n", text);
    if (new_text != NULL)
    {
      GtkTextIter end_iter;
      gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER (results_tb), &end_iter, line + 1);
      gtk_text_buffer_delete (GTK_TEXT_BUFFER (results_tb), &iter, &end_iter);
      gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, mark);
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (results_tb), &iter, new_text, -1, "header", "important", NULL);
      g_free (new_text);
      new_text = NULL;
    }
  gdk_threads_leave();
}


//!
//! @brief PRIVATE FUNCTION. Applies a tag to a section of text
//!
//! @param line An integer showing the line in the buffer to tag
//! @param start_offset the ending character in the line to highlight
//! @param end_offset The ending character in the line to highlight
//! @param item A LwSearchItem to get general information from
//!
static void _add_match_highlights (gint line, gint start_offset, gint end_offset, LwSearchItem* item)
{
    //Declarations
    GtkTextBuffer *tb;
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
    tb = GTK_TEXT_BUFFER (item->target_tb);
    ql = item->queryline;
    gtk_text_buffer_get_iter_at_line_offset (tb, &start_iter, line, start_offset);
    gtk_text_buffer_get_iter_at_line_offset (tb, &end_iter, line, end_offset);
    text = gtk_text_buffer_get_slice (tb, &start_iter, &end_iter, FALSE);

    //Look for kanji atoms
    for (iter = ql->re_kanji; *iter != NULL && **iter != NULL; iter++)
    {
      re = (*iter)[GW_RELEVANCE_LOCATE];
      if (g_regex_match (re, text, 0, &match_info))
      { 
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (tb, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info);
      }
    }

    //Look for furigana atoms
    for (iter = ql->re_furi; *iter != NULL && **iter != NULL; iter++)
    {
      re = (*iter)[GW_RELEVANCE_LOCATE];
      if (g_regex_match (re, text, 0, &match_info))
      { 
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (tb, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info);
      }
    }

    //Look for romaji atoms
    for (iter = ql->re_roma; *iter != NULL; iter++)
    {
      re = (*iter)[GW_RELEVANCE_LOCATE];
      if (re != NULL && g_regex_match (re, text, 0, &match_info))
      { 
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (tb, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
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
static void _shift_stay_mark (LwSearchItem *item, char *name)
{
    //Declarations
    GObject *results_tb;
    GtkTextMark *mark;
    GtkTextIter iter;

    //Initializations
    results_tb = G_OBJECT (item->target_tb);
    mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, mark);

    if ((mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), name)) == NULL)
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (results_tb), name, &iter, TRUE);
    else
      gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (results_tb), mark, &iter);
}


//!
//! @brief PRIVATE FUNCTION.  Updates the position of a mark to accomidate new results.
//!
//! @param item A LwSearchItem to gleam information from.
//! @param stay_name The name of the mark that stays in place before the new result.
//! @param append_name The name of the mark that moves to the end after the new result is added.
//!
static void _shift_append_mark (LwSearchItem *item, char *stay_name, char *append_name)
{
    //Declarations
    GObject *results_tb;
    GtkTextIter iter;
    GtkTextMark *stay_mark, *append_mark;

    //Initializations
    results_tb = G_OBJECT (item->target_tb);
    stay_mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), stay_name);
    gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, stay_mark);

    if ((append_mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), append_name)) == NULL)
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (results_tb), append_name, &iter, FALSE);
    else
      gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (results_tb), append_mark, &iter);
}


//!
//! @brief PRIVATE FUNCTION.  When adding a result to the buffer, it just adds the the kanji/hiragana section
//!
//! This function is made to help readability of edict results since there is a lot of repeating.
//!
//! @param item A LwSearchItem pointer to use for data.
//!
static void _append_def_same_to_buffer (LwSearchItem* item)
{
    //Declarations
    LwResultLine* resultline;
    GtkTextBuffer *tb;
    GtkTextMark *mark;

    //Initializations
    resultline = item->resultline;
    tb = GTK_TEXT_BUFFER (item->target_tb);

    _shift_append_mark (item, "previous_result", "new_result");
    if ((mark = gtk_text_buffer_get_mark (tb, "previous_result")) != NULL)
    {
      GtkTextIter iter;
      int line, start_offset, end_offset;
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
      line = gtk_text_iter_get_line (&iter);
      start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, " /", -1, "important", NULL);
      gtk_text_buffer_insert (tb, &iter, " ", -1);
      //Kanji
      if (resultline->kanji_start != NULL)
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->kanji_start, -1, "important", NULL);
      //Furigana
      if (resultline->furigana_start != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, " [", -1, "important", NULL);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->furigana_start, -1, "important", NULL);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, "]", -1, "important", NULL);
      }
      //Other info
      if (resultline->classification_start != NULL)
      {
        gtk_text_buffer_insert                   (tb, &iter, " ", -1);
        GtkTextIter copy = iter;
        gtk_text_iter_backward_char (&copy);
        gtk_text_buffer_remove_tag_by_name (tb, "important", &copy, &iter);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->classification_start, -1, "gray", "italic", NULL);
      }
      if (resultline->important == TRUE)
      {
        gtk_text_buffer_insert                   (tb, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Pop"), -1, "small", NULL);
      }
      end_offset = gtk_text_iter_get_line_offset (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
    }
}


//!
//! @brief Appends an edict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
void gw_output_append_edict_results_cb (LwSearchItem *item)
{
  gdk_threads_enter();
    //Some checks
    gboolean furigana_exists, kanji_exists;
    gboolean same_def_totals, same_first_def, same_furigana, same_kanji, skip;
    kanji_exists = (item->resultline->kanji_start != NULL && item->backup_resultline->kanji_start != NULL);
    furigana_exists = (item->resultline->furigana_start != NULL && item->backup_resultline->furigana_start != NULL);
    if (item->resultline->kanji_start == NULL || item->backup_resultline->kanji_start == NULL)
    {
      skip = TRUE;
    }
    else
    {
      same_def_totals = (item->resultline->def_total == item->backup_resultline->def_total);
      same_first_def = (strcmp(item->resultline->def_start[0], item->backup_resultline->def_start[0]) == 0);
      same_furigana = (!furigana_exists ||strcmp(item->resultline->furigana_start, item->backup_resultline->furigana_start) == 0);
      same_kanji = (!kanji_exists || strcmp(item->resultline->kanji_start, item->backup_resultline->kanji_start) == 0);
      skip = FALSE;
    }

    //Begin comparison if possible
    if (!skip && ((same_def_totals) || (same_kanji && same_furigana)) && same_first_def)
    {
      _append_def_same_to_buffer (item);
      gdk_threads_leave ();
      return;
    }

    gboolean remove_last_linebreak = (!skip && same_kanji && same_first_def);

    //Start output
    LwResultLine* rl = item->resultline;

    GtkTextBuffer *tb = GTK_TEXT_BUFFER (item->target_tb);
    GtkTextMark *mark;
    mark = gtk_text_buffer_get_mark (tb, "content_insertion_mark");

    if (remove_last_linebreak)
    {
      GtkTextIter si, ei;
      GtkTextMark *temp_mark;
      if ((temp_mark = gtk_text_buffer_get_mark (tb, "previous_result")) && gtk_text_buffer_get_mark (tb, "note_mark") == NULL)
      {
        gtk_text_buffer_get_iter_at_mark (tb, &si, temp_mark);
        gtk_text_buffer_create_mark (tb, "note_mark", &si, TRUE);
        gtk_text_buffer_get_iter_at_line (tb, &ei, gtk_text_iter_get_line (&si) + 1);
        gtk_text_buffer_delete (tb, &si, &ei);
      }
      gtk_text_buffer_get_iter_at_mark (tb, &ei, mark);
      gtk_text_buffer_get_iter_at_line (tb, &si, gtk_text_iter_get_line (&ei) - 1);
      gtk_text_buffer_delete(tb, &si, &ei);
    }
    else
    {
      GtkTextMark *temp_mark;
      if (temp_mark = gtk_text_buffer_get_mark (tb, "note_mark"))
         gtk_text_buffer_delete_mark (tb, temp_mark);
    }

    int line, start_offset, end_offset;
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
    line = gtk_text_iter_get_line (&iter);
    //Kanji
    if (rl->kanji_start != NULL)
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, rl->kanji_start, -1, "important", NULL);
    //Furigana
    if (rl->furigana_start != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, " [", -1, "important", NULL);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, rl->furigana_start, -1, "important", NULL);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, "]", -1, "important", NULL);
    }
    //Other info
    if (rl->classification_start != NULL)
    {
      gtk_text_buffer_insert (tb, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, rl->classification_start, -1, "gray", "italic", NULL);
    }
    if (rl->important == TRUE)
    {
      gtk_text_buffer_insert (tb, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Pop"), -1, "small", NULL);
    }
    _shift_stay_mark (item, "previous_result");
    start_offset = 0;
    end_offset = gtk_text_iter_get_line_offset (&iter);
    if (!remove_last_linebreak) gtk_text_buffer_insert (tb, &iter, "\n", -1);
    _add_match_highlights (line, start_offset, end_offset, item);

    //Definitions
    int i = 0;
    while (rl->def_start[i] != NULL)
    {
      gtk_text_buffer_insert (tb, &iter, "      ", -1);

      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, rl->number[i], -1, "comment", NULL);
      gtk_text_buffer_insert                   (tb, &iter, " ", -1);
      gtk_text_buffer_insert                   (tb, &iter, rl->def_start[i], -1);
      end_offset = gtk_text_iter_get_line_offset (&iter);
      line = gtk_text_iter_get_line (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
      gtk_text_buffer_insert                   (tb, &iter, "\n", -1);
      i++;
    }
    gtk_text_buffer_insert (tb, &iter, "\n", -1);

  gdk_threads_leave();
}


//!
//! @brief Appends a kanjidict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
void gw_output_append_kanjidict_results_cb (LwSearchItem *item)
{
    LwResultLine* resultline = item->resultline;
    GtkTextBuffer *tb = NULL;
    GtkWidget *tv = NULL;

    if (item->target == GW_TARGET_RESULTS)
    {
  gdk_threads_enter();
      tb = GTK_TEXT_BUFFER (item->target_tb);
      GtkTextIter iter;
      GtkTextMark *mark;
      mark = gtk_text_buffer_get_mark (tb, "content_insertion_mark");
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
      int line, start_offset, end_offset;

      //Kanji
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->kanji, -1, "large", "center", NULL);
      if (item->target == GW_TARGET_RESULTS) gtk_text_buffer_insert_with_tags_by_name (tb, &iter, " ", -1, "large", "center", NULL);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      if (item->target == GW_TARGET_RESULTS)
        _add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_insert (tb, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);

      //Radicals
      if (resultline->radicals != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Radicals:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (tb, &iter, resultline->radicals, -1);
        gtk_text_buffer_insert (tb, &iter, " ", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == GW_TARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);

        gtk_text_buffer_insert (tb, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);

        gw_radsearchtool_set_button_sensitive_when_label_is (resultline->radicals);
      }

      //Readings
      if (resultline->readings[0] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Readings:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (tb, &iter, resultline->readings[0], -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == GW_TARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (tb, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }
      if (resultline->readings[1] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Name:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (tb, &iter, resultline->readings[1], -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == GW_TARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (tb, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }
      if (resultline->readings[2] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Radical Name:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (tb, &iter, resultline->readings[2], -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == GW_TARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (tb, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }


      //etc
      gboolean line_started = FALSE;
      if (resultline->strokes)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Stroke:"), -1, "important", NULL);
        gtk_text_buffer_insert (tb, &iter, resultline->strokes, -1);
        line_started = TRUE;
      }
      if (resultline->frequency)
      {
        if (line_started) gtk_text_buffer_insert (tb, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Freq:"), -1, "important", NULL);
        gtk_text_buffer_insert (tb, &iter, resultline->frequency, -1);
        gtk_text_buffer_insert (tb, &iter, " ", -1);
      }
      if (resultline->grade)
      {
        if (line_started) gtk_text_buffer_insert (tb, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Grade:"), -1, "important", NULL);
        gtk_text_buffer_insert (tb, &iter, resultline->grade, -1);
      }
      if (resultline->jlpt)
      {
        if (line_started) gtk_text_buffer_insert (tb, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("JLPT:"), -1, "important", NULL);
        gtk_text_buffer_insert (tb, &iter, resultline->jlpt, -1);
      }

      gtk_text_buffer_insert (tb, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);

      //Meanings
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Meanings:"), -1, "important", NULL);
      gtk_text_buffer_insert (tb, &iter, resultline->meanings, -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      if (item->target == GW_TARGET_RESULTS)
        _add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_insert (tb, &iter, "\n\n", -1);
  gdk_threads_leave ();
    }
    
    if (item->target == GW_TARGET_KANJI && (tv = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_RESULTS))) != NULL)
    {
      char *markup;
      char *new;
      char *base;
      char *linebreak;
      gboolean first = TRUE;

      markup = g_strdup ("");
      new = NULL;
      base = NULL;
      linebreak = NULL;

      if (resultline->radicals) {
        first = FALSE;
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Radicals:"), resultline->radicals);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }
      if (resultline->readings[0]) {
        first = FALSE;
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Readings:"), resultline->readings[0]);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      } 
      if (resultline->readings[1]) {
        first = FALSE;
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Readings:"), resultline->readings[1]);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }
      if (resultline->readings[2]) {
        first = FALSE;
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

      char *markup2;
      markup2 = g_markup_printf_escaped ("<span font=\"KanjiStrokeOrders 100\">%s</span>", resultline->kanji);
      GtkWidget *child;

      GtkWidget *window = GTK_WIDGET (gtk_widget_get_tooltip_window (tv));
      if (window != NULL) {
        child = gtk_bin_get_child (window);
        if (child != NULL) gtk_widget_destroy (GTK_WIDGET (child));

        GtkWidget *hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 3));
        gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
        gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (hbox));

        GtkWidget *label = GTK_WIDGET (gtk_label_new (NULL));
        gtk_label_set_markup (GTK_LABEL (label), markup2);
        gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, TRUE, 0);

        label = GTK_WIDGET (gtk_label_new (NULL));
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, TRUE, 0);

        gtk_widget_show_all (hbox);
      }

      g_free (markup);
      g_free (markup2);
    }
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
void gw_output_append_examplesdict_results_cb (LwSearchItem *item)
{
  gdk_threads_enter();
      //Declarations
      LwResultLine* resultline;
      GtkTextBuffer *tb;
      int line, start_offset, end_offset;
      GtkTextMark *mark;
      GtkTextIter iter;

      //Initializations
      resultline = item->resultline;
      tb = GTK_TEXT_BUFFER (item->target_tb);
      mark = gtk_text_buffer_get_mark (tb, "content_insertion_mark");
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);

      if (resultline->def_start[0] != NULL)
      {
        // TRANSLATORS: The "E" stands for "English"
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("E:\t"), -1, "important", "comment", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->def_start[0], -1, "important", NULL, NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
        _add_match_highlights (line, start_offset, end_offset, item);
      }

      if (resultline->kanji_start != NULL)
      {
        // TRANSLATORS: The "J" stands for "Japanese"
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("\nJ:\t"), -1, "important", "comment", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->kanji_start, -1, NULL, NULL, NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
        _add_match_highlights (line, start_offset, end_offset, item);
      }

      if (resultline->furigana_start != NULL)
      {
        // TRANSLATORS: The "D" stands for "Detail"
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("\nD:\t"), -1, "important", "comment", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->furigana_start, -1, NULL, NULL, NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
        _add_match_highlights (line, start_offset, end_offset, item);
      }

      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
      gtk_text_buffer_insert (tb, &iter, "\n\n", -1);
  gdk_threads_leave();
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly.  This is the fallback safe function for unknown dictionaries.
//!
//! @param item A LwSearchItem to gleam information from.
//!
void gw_output_append_unknowndict_results_cb (LwSearchItem *item)
{
  gdk_threads_enter();
      LwResultLine* resultline = item->resultline;
      GtkTextBuffer *tb = GTK_TEXT_BUFFER (item->target_tb);
      GtkTextIter iter;
      GtkTextMark *mark;
      mark = gtk_text_buffer_get_mark (tb, "content_insertion_mark");

      int line, start_offset, end_offset;

      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (tb, &iter, resultline->string, -1);
      if (item->target == GW_TARGET_RESULTS) gtk_text_buffer_insert (tb, &iter, " ", -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
      gtk_text_buffer_insert (tb, &iter, "\n\n", -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
  gdk_threads_leave();
}




//!
//! @brief Add an header to irrelevant "other" results with number of matches
//!
void gw_output_append_less_relevant_header_cb (LwSearchItem *item)
{
    int irrelevant = item->total_irrelevant_results;
    char *message = g_strdup_printf (ngettext("Other Result %d", "Other Results %d", irrelevant), irrelevant);
    if (message != NULL)
    {
      _set_header (item, message, "less_relevant_header_mark");
      g_free (message);
    }
}


//!
//! @brief Add an header to relevant "main" results with number of matches
//!
void gw_output_append_more_relevant_header_cb (LwSearchItem *item)
{
    int relevant = item->total_relevant_results;
    char *message = g_strdup_printf (ngettext("Main Result %d", "Main Results %d", relevant), relevant);
    if (message != NULL)
    {
      _set_header (item, message, "more_relevant_header_mark");
      g_free (message);
    }
}


//!
//! @brief Sets up the interface before each search begins
//!
//! @param item A LwSearchItem pointer to get information from
//!
void gw_output_pre_search_prep_cb (LwSearchItem *item)
{
    gw_main_initialize_buffer_by_searchitem (item);
    item->target_tb = (gpointer) gw_common_get_gobject_by_target (item->target);
    item->target_tv = (gpointer) gw_common_get_widget_by_target (item->target);
}


//!
//! @brief The details to be taken care of after a search is finished
//!
//! This is the function that takes care of things such as hiding progressbars,
//! changing action verbs to past verbs (Searching... vs Found) and for displaying
//! "no results found" pages.  Before this function is called, the searchitem search
//! status changes from GW_SEARCH_SEARCHING to GW_SEARCH_FINISHING.
//!
//! @param item A LwSearchItem pointer to get information from
//!
void gw_output_after_search_cleanup_cb (LwSearchItem *item)
{
    //Finish up
    if (item->total_results == 0 &&
        item->target != GW_TARGET_KANJI && item->status != GW_SEARCH_CANCELING)
    {
      gw_main_display_no_results_found_page (item);
    }
}



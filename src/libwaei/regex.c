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
//!  @file src/regex.c
//!
//!  @brief Compiled often used regex expressions
//!
//!  Functions that deal with regex operations. This is also where constant
//!  regrexs are stored to improve search speed.
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/libwaei.h>


static gboolean _regex_expressions_initialized = FALSE;
GRegex *lw_re[GW_RE_TOTAL + 1];

/*


  { GW_RE_FILENAME_GZ, "\\.gz$", GW_REGEX_EFLAGS_EXIST },

  { GW_RE_WORD_I_ADJ_PASTFORM,    "\\B((かった))$", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_WORD_I_ADJ_NEGATIVE,    "\\B((くない))$", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_WORD_I_ADJ_TE_FORM,     "\\B((くて))$", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_WORD_I_ADJ_CAUSATIVE,   "\\B((くさせる))$", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_WORD_I_ADJ_CONDITIONAL, "\\B((ければ))$", GW_REGEX_EFLAGS_LOCATE },


  { GW_RE_WORD_NA_ADJ_PASTFORM,    "\\B((だった))$", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_WORD_NA_ADJ_NEGATIVE,    "\\B((ではない)|(じゃない))$", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_WORD_NA_ADJ_TE_FORM,     "\\B((で))$", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_WORD_NA_ADJ_CAUSATIVE,   "\\B((にさせる))$", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_WORD_NA_ADJ_CONDITIONAL, "\\B((であれば))$", GW_REGEX_EFLAGS_LOCATE },

    //Verb forms
    regcomp (re_verb_presentform, "\\B((ます))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_politepast, "\\B((ました))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_pastform_negative, "\\B((なかった))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_pastform, "\\B((った)|(いた)|(いだ)|(した)|(んだ)|(えた))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_negative, "\\B((わない)|(かない)|(がない)|(さない)|(たない)|(なない)|(まない)|(いない))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_te_form, "\\B((って)|(いて)|(いで)|(して)|(んで))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_potention, "\\B((える)|(ける)|(げる)|(せる)|(てる)|(ねる)|(べる)|(める)|(れる)|(いられる)|(えられる)|(いれる))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_causative, "\\B((させる)|(わせる)|(かせる)|(がせる)|(なせる)|(たせる)|(ばせる)|ませる(らせる)|(いさせる)|())$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_conditional, "\\B((すれば)|(くれば)|(であれば)|(えば)|(けば)|(げば)|(せば)|(てば)|(ねば)|(べば)|(めば)|(れば)|(いれば)|(れば))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_negative_conditional, "\\B((なければ))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_imperative, "\\B((しろ)|(せよ)|(こい)|(くれ)|(ませ)|(であれ)|(え)|(け)|(せ)|(て)|(ね)|(べ)|(め)|(れ)|(いろ)|(えろ))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_passive, "\\B((される)|(こられる)|(われる)|(かれる)|(がれる)|(される)|(たれる)|(なれる)|(ばれる)|(まれる)|(られる)|(いられる)|(えられる))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_volitional, "\\B((しよう)|(せよう)|(こよう)|(だろう)|(ましょう)|(おう)|(こう)|(ごう)|(そう)|(とう)|(のう)|(ぼう)|(もう)|(ろう)|(いよう)|(よう))$", GW_REGEX_EFLAGS_EXIST);
*/



//!
//! @brief Initializes often used prebuilt regex expressions
//!
void lw_regex_initialize ()
{
    if (_regex_expressions_initialized) return;

    //Declarations
    GError *error;
    int i;
    
    //Initializations
    error = NULL;

    //Setup the built in regexes
    for (i = 0; i < GW_RE_TOTAL; i++)
    {
      switch (i)
      {
        case GW_RE_NUMBER:
          lw_re[i] = g_regex_new ("[a-zA-Z][0-9]{1,4}",  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, &error);
          break;
        case GW_RE_STROKES:
          lw_re[i] = g_regex_new ("S[0-9]{1,2}",  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, &error);
          break;
        case GW_RE_GRADE:
          lw_re[i] = g_regex_new ("G[0-9]{1,2}",  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, &error);
          break;
        case GW_RE_FREQUENCY:
          lw_re[i] = g_regex_new ("F[0-9]{1,4}",  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, &error);
          break;
        case GW_RE_JLPT:
          lw_re[i] = g_regex_new ("J[0-4]{1,1}",  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, &error);
          break;
        default:
          g_assert_not_reached();
      }
    }
    lw_re[i] = NULL;

    if (error != NULL)
    {
       fprintf (stderr, "Unable to read file: %s\n", error->message);
       g_error_free (error);
    }

    _regex_expressions_initialized = TRUE;
}


//!
//! @brief Frees often used prebuilt regex expressions
//!
void lw_regex_free ()
{
    if (_regex_expressions_initialized == FALSE) return;

    //Declarations
    int i;

    //Setup the built in regexes
    for (i = 0; i < GW_RE_TOTAL; i++)
    {
      g_regex_unref (lw_re[i]);
      lw_re[i] = NULL;
    }

    _regex_expressions_initialized = FALSE;
}


//!
//! @brief Builds a regex for finding kanji by relevance
//!
//! @param subject The query text to insert into the regex
//! @param relevance How relevant a result to search for
//! @param flags GRegexMatchFlags to apply to the regex compilation.  
//! @returns A newly allocated GRegex that needs to be freed with g_regex_unref ()
//! 
GRegex* lw_regex_kanji_new (const char *subject, const LwEngine ENGINE, const LwRelevance RELEVANCE, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return NULL;

    //Declarations
    GRegex *re;
    char *format;
    char *expression;

    switch (RELEVANCE)
    {
      case GW_RELEVANCE_HIGH:
        if (ENGINE == GW_ENGINE_EXAMPLES)
          format = "%s";
        else if (ENGINE == GW_ENGINE_KANJI)
          format = "^(%s)$";
        else
          format = "^(無|不|非|お|御|)(%s)$";
        expression = g_strdup_printf(format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_MEDIUM:
        if (ENGINE == GW_ENGINE_KANJI)
          format = "%s";
        else
          format = "^(お|を|に|で|は|と|)(%s)(で|が|の|を|に|で|は|と|$)";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_LOW:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        break;
      case GW_RELEVANCE_LOCATE:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, error);
        break;
      default:
        g_assert_not_reached();
    }

    return re;
}


//!
//! @brief Builds a regex for finding furigana by relevance
//!
//! @param subject The query text to insert into the regex
//! @param relevance How relevant a result to search for
//! @returns A newly allocated GRegex that needs to be freed with g_regex_unref ()
//! 
GRegex* lw_regex_furi_new (const char *subject, const LwEngine ENGINE, const LwRelevance RELEVANCE, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return NULL;

    //Declarations
    GRegex *re;
    char *format;
    char *expression;

    switch (RELEVANCE)
    {
      case GW_RELEVANCE_HIGH:
        if (ENGINE == GW_ENGINE_EXAMPLES)
          format = "%s";
        else if (ENGINE == GW_ENGINE_KANJI)
          format = "(^|\\s)%s(\\s|$)";
        else
          format = "^(お|)(%s)$";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_MEDIUM:
        if (ENGINE == GW_ENGINE_KANJI)
          format = "(^|\\s)(%s)(\\s|$)";
        else
          format = "(^お|を|に|で|は|と)(%s)(で|が|の|を|に|で|は|と|$)";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_LOW:
        re = g_regex_new (subject, GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        break;
      case GW_RELEVANCE_LOCATE:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, error);
        break;
      default:
        g_assert_not_reached();
    }

    return re;
}


//!
//! @brief Builds a regex for finding romaji by relevance
//!
//! @param subject The query text to insert into the regex
//! @param relevance How relevant a result to search for
//! @returns A newly allocated GRegex that needs to be freed with g_regex_unref ()
//! 
GRegex* lw_regex_romaji_new (const char *subject, const LwEngine ENGINE, const LwRelevance RELEVANCE, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return NULL;

    //Declarations
    GRegex *re;
    char *format;
    char *expression;

    switch (RELEVANCE)
    {
      case GW_RELEVANCE_HIGH:
        if (ENGINE == GW_ENGINE_EXAMPLES)
          format = "%s";
        else if (ENGINE == GW_ENGINE_KANJI)
          format = "\\{(%s)\\}";
        else
          format = "(^|\\)|/|^to |\\) )(%s)(\\(|/|$|!| \\()";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_MEDIUM:
        if (ENGINE == GW_ENGINE_KANJI)
          format = "\\b(%s)\\b";
        else
          format = "(\\) |/)((\\bto )|(\\bto be )|(\\b))(%s)(( \\([^/]+\\)/)|(/))";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_LOW:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        break;
      case GW_RELEVANCE_LOCATE:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, error);
        break;
      default:
        g_assert_not_reached();
    }

    return re;
}


//!
//! @brief Builds a regex for finding mix by relevance
//!
//! @param subject The query text to insert into the regex
//! @param relevance How relevant a result to search for
//! @returns A newly allocated GRegex that needs to be freed with g_regex_unref ()
//! 
GRegex* lw_regex_mix_new (const char *subject, const LwEngine ENGINE, const LwRelevance RELEVANCE, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return NULL;

    //Declarations
    GRegex *re;
    char *format;
    char *expression;

    switch (RELEVANCE)
    {
      case GW_RELEVANCE_HIGH:
        format =  "(^|\\b)(%s)(\\b)";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_MEDIUM:
        if (ENGINE == GW_ENGINE_KANJI)
          format = "\\{(%s)\\}";
        else
          format = "(\\) |/)((\\bto )|(\\bto be )|(\\b))(%s)(( \\([^/]+\\)/)|(/))";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_LOW:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        break;
      case GW_RELEVANCE_LOCATE:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, error);
        break;
      default:
        g_assert_not_reached();
    }

    return re;
}


//!
//! @brief Builds a regex for finding mix by relevance
//!
//! @param subject The query text to insert into the regex
//! @param relevance How relevant a result to search for
//! @returns A newly allocated GRegex that needs to be freed with g_regex_unref ()
//! 
GRegex* lw_regex_new (const char *subject, const LwEngine ENGINE, const LwRelevance RELEVANCE, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return NULL;

    //Declarations
    GRegex *re;
    char *format;
    char *expression;

    switch (RELEVANCE)
    {
      case GW_RELEVANCE_HIGH:
        format = "\\b(%s)\\b";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_MEDIUM:
        format = "\\b(%s)\\b";
        expression = g_strdup_printf (format, subject);
        re = g_regex_new (expression,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        g_free (expression);
        break;
      case GW_RELEVANCE_LOW:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_EXIST_FLAGS, error);
        break;
      case GW_RELEVANCE_LOCATE:
        re = g_regex_new (subject,  GW_RE_COMPILE_FLAGS, GW_RE_LOCATE_FLAGS, error);
        break;
      default:
        g_assert_not_reached();
    }

    return re;
}




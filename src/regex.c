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
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>


gboolean gw_global_regex_expressions_initialized = FALSE;

struct _GwRegexInfo {
  GwInitialDictonaryRegexIndex index;
  char *expression;
  int flags;
};
typedef struct _GwRegexInfo GwRegexInfo;

static GwRegexInfo regex_info_array[] = 
{
  { GW_RE_DICT_ENGLISH, "English", GW_REGEX_EFLAGS_EXIST },
  { GW_RE_DICT_RADICAL, "Radical", GW_REGEX_EFLAGS_EXIST },
  { GW_RE_DICT_KANJI, "Kanji", GW_REGEX_EFLAGS_EXIST },
  { GW_RE_DICT_PLACES, "Names", GW_REGEX_EFLAGS_EXIST },
  { GW_RE_DICT_NAMES, "Places", GW_REGEX_EFLAGS_EXIST },
  { GW_RE_DICT_MIX, "Mix", GW_REGEX_EFLAGS_EXIST },

  // Searches for this numeric values in Kanji and Mix need to be \\b
  // in order to avoid false positive hits (XJ07337, I5g19.1, U59f6, DG1, DF1, etc)
  { GW_RE_QUERY_STROKES, "\\bS[0-9]{1,2}", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_QUERY_GRADE, "\\bG[0-9]{1,2}", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_QUERY_FREQUENCY, "\\bF[0-9]{1,4}", GW_REGEX_EFLAGS_LOCATE },
  { GW_RE_QUERY_JLPT, "\\bJ[0-4]{1,1}", GW_REGEX_EFLAGS_LOCATE },

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
  { -1, NULL, -1}
};

/*
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
void gw_regex_initialize_constant_regular_expressions ()
{
    if (gw_global_regex_expressions_initialized == TRUE) return;

    int i;
    for (i = 0; regex_info_array[i].index != -1; i++)
    {
      if (regex_info_array[i].expression == NULL)
      {
        gw_re[regex_info_array[i].index] = NULL;
      }
      else
      {
        gw_re[regex_info_array[i].index] = (regex_t*) malloc (sizeof(regex_t));
        regcomp (gw_re[regex_info_array[i].index], regex_info_array[i].expression, regex_info_array[i].flags);
      }
    }
    gw_global_regex_expressions_initialized = TRUE;
}


//!
//! @brief Frees often used prebuilt regex expressions
//!
void gw_regex_free_constant_regular_expressions ()
{
    if (gw_global_regex_expressions_initialized == FALSE) return;
    int i;
    for (i = 0; regex_info_array[i].index != -1; i++)
    {
      if (&regex_info_array[i] != NULL)
      {
        regfree(gw_re[regex_info_array[i].index]);
        gw_re[regex_info_array[i].index] = NULL;
      }
    }
    gw_global_regex_expressions_initialized = FALSE;
}


//!
//! @brief Builds a regex for a pattern and returns pointers to matches
//!
//! Function takes searches for the pattern in a string and then returns
//! pointers to the beginning and end of matches. As is expected, this kind of search
//! should be avoided when possible because it is slow.
//!
//! @param string a constant string to be searched
//! @param pattern a pattern string to search for
//! @param start a character pointer array for match starts points
//! @param end a character pointer array for match end points
//! @return Returns true when pattern is found.
//!
gboolean gw_regex_locate_boundary_byte_pointers (const char *string, char  *pattern,
                                                 char      **start,  char **end     )
{
    regex_t re;
    int status;
    int eflags = REG_EXTENDED | REG_ICASE;
   
    if ((status = regcomp(&re, pattern, eflags)) == 0)
    {
      size_t nmatch = 1;
      regmatch_t pmatch[nmatch];
      if ((status = regexec(&re, string, nmatch, pmatch, 0)) == 0)
      {
        *start = (char*) (string + pmatch[0].rm_so);
        *end = (char*) (string + pmatch[0].rm_eo);
      }
      regfree(&re);
    }
    return !status; 
} 


//!
//! @brief A function used to quickly locate the position of a pattern in a haystack
//!
//! Mostly this function is used for located where matches are a string so the approprate
//! highlighting can be applied to them.
//!
//! @param string the current position in the line
//! @param line_start the start of the line to calculate the offset against
//! @param re_locate A compiled regex to use
//! @param start the start offset
//! @param start the end offset
//! @return Returns a new position in the line after the match
//!
char* gw_regex_locate_offset (char *string, char *line_start, regex_t *re_locate,
                              gint *start,  gint    *end                         )

{
    if (string == NULL) return NULL;

    //Force regex to stop searching at line breaks
    char *string_ptr = string;
    char temp;
    while(*string_ptr != '\n' && *string_ptr != '\0')
      string_ptr++;
    temp = *string_ptr;
    *string_ptr = '\0';

    size_t nmatch = 1;
    regmatch_t pmatch[nmatch];

    int status;
    if ((status = regexec(re_locate, string, 1, pmatch, 0)) == 0)
    {
      *start = g_utf8_pointer_to_offset (line_start, string + pmatch[0].rm_so);
      *end = g_utf8_pointer_to_offset (line_start, string + pmatch[0].rm_eo);
      *string_ptr = temp;
      return (string + pmatch[0].rm_eo);
    }
    else
    {
      *string_ptr = temp;
      return NULL;
    }
}


//!
//! @brief Regex for determining highly relevent kanji atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a kanji one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_kanji_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "((^無)|(^不)|(^非)|(^)|(^お)|(^御))(");
    strcat (expression, string);
    strcat (expression, ")(($))");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining mediumly relevent kanji atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a kanji one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_kanji_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "((^)|(^お)|(を)|(に)|(で)|(は)|(と))(");
    strcat (expression, string);
    strcat (expression, ")((で)|(が)|(の)|(を)|(に)|(で)|(は)|(と)|($))");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining highly relevent furigana atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a furigana one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_furi_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "^((お)|())(");
    strcat (expression, string);
    strcat (expression, ")$");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining mediumly relevent furigana atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a furigana one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_furi_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "((^)|(^お)|(を)|(に)|(で)|(は)|(と))(");
    strcat (expression, string);
    strcat (expression, ")((で)|(が)|(の)|(を)|(に)|(で)|(は)|(と)|($))");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining highly relevent romaji atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a romaji one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_roma_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "(^|\\)|(/)|(^to )|\\) )(");
    strcat (expression, string);
    strcat (expression, ")(\\(|/|$|!| \\()");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining mediumly relevent romaji atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a romaji one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_roma_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "\\{(");
    strcat (expression, string);
    strcat (expression, ")\\}|(\\) |/)((\\bto )|(\\bto be )|(\\b))(");
    strcat (expression, string);
    strcat (expression, ")(( \\([^/]+\\)/)|(/))|(\\[)(");
    strcat (expression, string);
    strcat (expression, ")(\\])|^(");
    strcat (expression, string);
    strcat (expression, ")\\b");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining highly relevent mix atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a mix one. This means
//! there is kanji/hiragana/romaji mingled throughout.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_mix_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "(\\b(");
    strcat (expression, string);
    strcat (expression, ")\\b|^(");
    strcat (expression, string);
    strcat (expression, "))");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining mediumly relevent mix atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a mix one. This means
//! there is kanji/hiragana/romaji mingled throughout.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_mix_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "\\{(");
    strcat (expression, string);
    strcat (expression, ")\\}|(\\) |/)((to )|(to be )|())(");
    strcat (expression, string);
    strcat (expression, ")(( \\([^/]+\\)/)|(/))|(\\[)(");
    strcat (expression, string);
    strcat (expression, ")(\\])|^(");
    strcat (expression, string);
    strcat (expression, ")\\b");
    return regcomp (regex, expression, flags);
}

//!
//! @brief Regex for determining exact regexes
//!
//! To be written
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_exact_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "^(");
    strcat (expression, string);
    strcat (expression, ")$");
    return regcomp (regex, expression, flags);
}

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
//! @file utilities.c
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include <glib.h>
#include <glib/gstdio.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <libwaei/libwaei.h>


//!
//! @brief Creates an allocated path to a file.  If the FILENAME is NULL,
//!        then it is just a path to a directory.  It must be freed with
//!        g_free
//! @param PATH Used to determine which folder path to return
//! @param FILENAME The filename to be appended to the path or NULL
//! @returns Returns a constant that should be freed with g_free
//!
gchar* 
lw_util_build_filename (const LwFolderPath PATH, const char *FILENAME) 
{
    g_assert (PATH >= 0 && PATH < TOTAL_LW_PATHS);

    //Declarations
    char *base;
    char *folder;
    char *path;
    
    base = g_build_filename (g_get_user_config_dir (), PACKAGE, NULL);
    folder = NULL;
    path = NULL;

    switch (PATH)
    {
      case LW_PATH_BASE:
        folder = g_strdup (base);
        path = g_strdup (base);
        break;
      case LW_PATH_DICTIONARY:
        folder = g_build_filename (base, "dictionaries", NULL);
        path = g_build_filename (base, "dictionaries", FILENAME, NULL);
        break;
      case LW_PATH_VOCABULARY:
        folder = g_build_filename (base, "vocabulary", NULL);
        path = g_build_filename (base, "vocabulary", FILENAME, NULL);
        break;
      case LW_PATH_PLUGIN:
        folder = g_build_filename (base, "plugins", NULL);
        path = g_build_filename (base, "plugins", FILENAME, NULL);
        break;
      case LW_PATH_CACHE:
        folder = g_build_filename (base, "cache", NULL);
        path = g_build_filename (base, "cache", FILENAME, NULL);
        break;
      case LW_PATH_INDEX:
        folder = g_build_filename (base, "index", NULL);
        path = g_build_filename (base, "index", FILENAME, NULL);
        break;
      default:
        g_assert_not_reached ();
        folder = NULL;
        path = NULL;
        break;
    }
    g_free (base);

    g_mkdir_with_parents (folder, 0755);

    g_free (folder);

    return path;
}


//!
//! @brief Gets the compression type as a string
//! @param COMPRESSION The LwCompression type to use
//! @returns A constant string that should not be freed
//!
const char* 
lw_util_get_compressionname (const LwCompression COMPRESSION)
{
    char *type;

    switch (COMPRESSION)
    {
/*
      case LW_COMPRESSION_ZIP:
        g_error ("currently unsupported compression type\n");
        return "zip";
*/
      case LW_COMPRESSION_GZIP:
        type = "gz";
        break;
      case LW_COMPRESSION_NONE:
        type = "uncompressed";
        break;
      default:
        g_assert_not_reached ();
        type = NULL;
        break;
    }

    return type;
}


//!
//! @brief Gets the encoding type as a string
//! @param ENCODING The LwEncoding type to use
//! @returns A constant string that should not be freed
//!
const char* 
lw_util_get_encodingname (const LwEncoding ENCODING)
{
    char *type;

    switch (ENCODING)
    {
      case LW_ENCODING_EUC_JP:
        type = "EUC-JP";
        break;
      case LW_ENCODING_SHIFT_JS:
        type = "Shift-JS";
        break;
      case LW_ENCODING_UTF8:
        type = "UTF-8";
        break;
      default:
        g_assert_not_reached ();
        type = NULL;
        break;
    }

    return type;
}


//!
//! @brief Convenience function for seeing if a string is hiragana
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see lw_util_is_katakana_str ()
//! @see lw_util_is_kanji_str ()
//! @see lw_util_is_romaji_str ()
//!
gboolean 
lw_util_is_hiragana_str (const char *input)
{
    //Declarations
    gboolean is_consistant;
    gunichar character;
    GUnicodeScript script;
    const char *ptr;

    //Initializations
    is_consistant = TRUE;
    
    //Loop over the string checking for characters inconsistant with the script
    for (ptr = input; *ptr != '\0' && is_consistant; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script != G_UNICODE_SCRIPT_HIRAGANA &&
          script != G_UNICODE_SCRIPT_COMMON)
        is_consistant = FALSE;
    }

    return is_consistant;
}


//!
//! @brief Convenience function for seeing if a string is katakana
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see lw_util_is_hiragana_str ()
//! @see lw_util_is_kanji_str ()
//! @see lw_util_is_romaji_str ()
//!
gboolean 
lw_util_is_katakana_str (const char *input)
{
    //Declarations
    gboolean is_consistant;
    gunichar character;
    GUnicodeScript script;
    const char *ptr;

    //Initializations
    is_consistant = TRUE;
    
    //Loop over the string checking for characters inconsistant with the script
    for (ptr = input; *ptr != '\0' && is_consistant; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script != G_UNICODE_SCRIPT_KATAKANA &&
          script != G_UNICODE_SCRIPT_COMMON)
        is_consistant = FALSE;
    }

    return is_consistant;
}


//!
//! @brief Convenience function for seeing if a string is furigana
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see lw_util_is_hiragana_str ()
//! @see lw_util_is_kanji_str ()
//! @see lw_util_is_romaji_str ()
//!
gboolean 
lw_util_is_furigana_str (const char *input)
{
    return (lw_util_is_katakana_str (input) || lw_util_is_hiragana_str (input));
}



//!
//! @brief Convenience function for seeing if a string *starts* with kanji
//! @param input The string to check
//! @return Returns true if the function things this is a kanji string
//! @see lw_util_is_hiragana_str ()
//! @see lw_util_is_katakana_str ()
//! @see lw_util_is_romaji_str ()
//!
gboolean 
lw_util_is_kanji_ish_str (const char *input)
{
    //Declarations
    gboolean is_consistant;
    gunichar character;
    GUnicodeScript script;
    const char *ptr;

    //Initializations
    is_consistant = TRUE;
    
    //Loop over the string checking for characters inconsistant with the script
    for (ptr = input; *ptr != '\0' && is_consistant; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script != G_UNICODE_SCRIPT_HAN &&
          script != G_UNICODE_SCRIPT_HIRAGANA &&
          script != G_UNICODE_SCRIPT_KATAKANA &&
          script != G_UNICODE_SCRIPT_COMMON)
        is_consistant = FALSE;
    }

    return is_consistant;
}

//!
//! @brief Convenience function for seeing if a string is kanji
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see lw_util_is_hiragana_str ()
//! @see lw_util_is_katakana_str ()
//! @see lw_util_is_romaji_str ()
//!
gboolean 
lw_util_is_kanji_str (const char *input)
{
    //Declarations
    gboolean is_consistant;
    gunichar character;
    GUnicodeScript script;
    const char *ptr;

    //Initializations
    is_consistant = TRUE;
    
    //Loop over the string checking for characters inconsistant with the script
    for (ptr = input; *ptr != '\0' && is_consistant; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script != G_UNICODE_SCRIPT_HAN &&
          script != G_UNICODE_SCRIPT_COMMON)
        is_consistant = FALSE;
    }

    return is_consistant;
}


//!
//! @brief Convenience function for seeing if a string is romaji
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see lw_util_is_hiragana_str ()
//! @see lw_util_is_katakana_str ()
//! @see lw_util_is_kanji_str ()
//!
gboolean 
lw_util_is_romaji_str (const char *input)
{
    //Declarations
    gboolean is_consistant;
    gunichar character;
    GUnicodeScript script;
    const char *ptr;

    //Initializations
    is_consistant = TRUE;
    
    //Loop over the string checking for characters inconsistant with the script
    for (ptr = input; *ptr != '\0' && is_consistant; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script != G_UNICODE_SCRIPT_LATIN && 
          script != G_UNICODE_SCRIPT_COMMON)
        is_consistant = FALSE;
    }

    return is_consistant;
}


//!
//! @brief Checks if a given string is yojijukugo (a four kanji phrase)
//! @param INPUT
//! @returns Returns TRUE if the text looks to be yojijukugo.  FALSE if it isn't.
//!
gboolean 
lw_util_is_yojijukugo_str (const char* INPUT)
{
  return (g_utf8_strlen (INPUT, -1) == 4 && lw_util_is_kanji_str (INPUT));
}


//!
//! @brief Shifts kana characters in a specific direction
//! @param input The string to shift
//! @param shift How much to shift by
//! @see lw_util_str_shift_hira_to_kata ()
//! @see lw_util_str_shift_kata_to_hira ()
//!
void 
lw_util_shift_kana_chars_in_str_by (char *input, int shift)
{
    //Setup
    char *input_ptr;
    input_ptr = input;

    char output[strlen(input) + 1];
    char *output_ptr;
    output_ptr = output;

    gunichar unic;
    unic = g_utf8_get_char(input_ptr);

    gint offset = 0;

    //Start the conversion
    while (*input_ptr != '\0')
    {
      if (unic >= 0x3041 && unic <= 0x30ff &&
          unic + shift >= 0x3041 && unic + shift <= 0x30ff &&
          unic != L'ー') {
        offset = g_unichar_to_utf8((unic + shift), output_ptr);
      }
      else {
        offset = g_unichar_to_utf8((unic), output_ptr);
      }
      output_ptr = output_ptr + offset;

      input_ptr = g_utf8_next_char(input_ptr);
      unic = g_utf8_get_char(input_ptr);
    }
    *output_ptr = *input_ptr;

    strcpy(input, output);
}


//!
//! @brief Convenience function to shift hiragana to katakana
//!
//! @param input The string to shift
//! @see lw_util_shift_kana_chars_in_str_by ()
//! @see lw_util_str_shift_kata_to_hira ()
//!
void 
lw_util_str_shift_hira_to_kata (char input[])
{
    lw_util_shift_kana_chars_in_str_by (input, (L'ア' - L'あ'));
}


//!
//! @brief Convenience function to shift katakana to hiragana
//!
//! @param input The string to shift
//! @see lw_util_shift_kana_chars_in_str_by ()
//! @see lw_util_str_shift_hira_to_kata ()
//!
void 
lw_util_str_shift_kata_to_hira (char input[])
{
    lw_util_shift_kana_chars_in_str_by (input, (L'あ' - L'ア'));
}


//!
//! @brief Gets the next hiragana equivalent char pointer in a string
//!
//! This function returns the hiragana equivalent and skips the romanji equivalent
//! forward in the string.  This function is used for romaji->hiragana conversions.
//!
//! @param input The string to jump around
//! @return where the next hiragana equivalent character would start
//!
const char* 
lw_util_next_hira_char_from_roma (const char *input)
{
    const char *input_ptr;
    input_ptr = input;

    int total_n = 0;

    //Cautiously up a character
    if (*input_ptr == 'n')
      total_n++;
    if (*input_ptr != '\0')
      input_ptr++;
    if (*input_ptr == '\0')
      return input_ptr;

    //Loop until we hit a hiragana character ending
    while ( (
            //General conditional for hiragana processing
              *(input_ptr    ) != '\0' &&
              *(input_ptr - 1) != 'a'  &&
              *(input_ptr - 1) != 'i'  &&
              *(input_ptr - 1) != 'u'  &&
              *(input_ptr - 1) != 'e'  &&
              *(input_ptr - 1) != 'o'  &&
              *(input_ptr - 1) != *input_ptr  &&
              *(input_ptr - 1) != '-'  &&
              total_n < 3
            ) ||
            //Special conditional for symbolic n processing
            (
              *(input_ptr    ) != '\0' &&
              *(input_ptr - 1) == 'n'  &&
              *(input_ptr    ) == 'n'  &&
              total_n < 3
            )
          )
    {
      //Update the n count
      if (*(input_ptr - 1) == 'n')
        total_n++;
      else
        total_n = 0;


      if (*(input_ptr - 1) == 'n' &&
              *(input_ptr) != 'a'  &&
              *(input_ptr) != 'i'  &&
              *(input_ptr) != 'u'  &&
              *(input_ptr) != 'e'  &&
              *(input_ptr) != 'o'  &&
              *(input_ptr) != 'y'
         )
        break;

      //Increment
      input_ptr++;
    }
    if (*(input_ptr - 1) == 'n' &&
            *(input_ptr) == 'n'
       )
      input_ptr++;


    return input_ptr;
}


//!
//! @brief Converts a romaji string to hiragana.
//! @param input The source romaji string
//! @param output The string to write the hiragana equivalent to
//! @return Returns null on error/end
//!
gchar* 
lw_util_roma_char_to_hira (const gchar *input, gchar *output)
{
    //Set up the input pointer
    const char *input_ptr;
    input_ptr = input;

    //Make sure the output pointer is initialized
    output[0] = '\0';
    
    //Set up the buffer variables
    char buffer[] = "           ";
    char *buffer_ptr = buffer;

    //Copy the next hiragana char written in romaji to the buffer
    while ( 
            (
              *input_ptr != '\0' &&
              *input_ptr != 'a'  &&
              *input_ptr != 'i'  &&
              *input_ptr != 'u'  &&
              *input_ptr != 'e'  &&
              *input_ptr != 'o'  &&
              *input_ptr != '-'  && 
              *input_ptr != *(input_ptr + 1)  && 
              *(buffer_ptr + 1) == ' '                
            ) 
          )
    {
      *buffer_ptr = *input_ptr;
      input_ptr++;
      buffer_ptr++;
    }
    *buffer_ptr = *input_ptr;
    buffer_ptr++;
    *buffer_ptr = '\0';

    //HACK!!!!!!!!!!!!!!!!
    if (buffer[0] == 'n' &&
              buffer[1] != 'a'  &&
              buffer[1] != 'i'  &&
              buffer[1] != 'u'  &&
              buffer[1] != 'e'  &&
              buffer[1] != 'o'  &&
              buffer[1] != 'y'
       )
    {
       buffer[1] = '\0';
    }
    else if ( buffer[0] != 'a'  &&
              buffer[0] != 'i'  &&
              buffer[0] != 'u'  &&
              buffer[0] != 'e'  &&
              buffer[0] != 'o'  &&
              buffer[0] != 'n'  &&
              strlen(input) == 1
            )
    {
       return NULL;
    }
    //HACK!!!!!!!!!!!!!!!!


    //Reset the buffer pointer
    buffer_ptr = buffer;


    //
    //Start main lookup for conversion
    //

    if (strcmp(buffer_ptr, "n") == 0)
       strcpy(output, "ん");


    else if (strlen(buffer_ptr) == 1 &&
             buffer_ptr[0] != 'a'    &&
             buffer_ptr[0] != 'i'    &&
             buffer_ptr[0] != 'u'    &&
             buffer_ptr[0] != 'e'    &&
             buffer_ptr[0] != 'o'    &&
             buffer_ptr[0] != '-'    &&
             buffer_ptr[0] != 'y'    &&
             input_ptr[1] != '\0'      )
       strcpy(output, "っ");

    else if (strcmp(buffer_ptr, "a") == 0)
       strcpy(output, "あ");
    else if (strcmp(buffer_ptr, "i") == 0)
       strcpy(output, "い");
    else if (strcmp(buffer_ptr, "u") == 0)
       strcpy(output, "う");
    else if (strcmp(buffer_ptr, "e") == 0)
       strcpy(output, "え");
    else if (strcmp(buffer_ptr, "o") == 0)
       strcpy(output, "お");


    else if (strcmp(buffer_ptr, "ka") == 0 || strcmp(buffer_ptr, "ca") == 0)
       strcpy(output, "か");
    else if (strcmp(buffer_ptr, "ki") == 0 || strcmp(buffer_ptr, "ci") == 0)
       strcpy(output, "き");
    else if (strcmp(buffer_ptr, "ku") == 0 || strcmp(buffer_ptr, "cu") == 0)
       strcpy(output, "く");
    else if (strcmp(buffer_ptr, "ke") == 0 || strcmp(buffer_ptr, "ce") == 0)
       strcpy(output, "け");
    else if (strcmp(buffer_ptr, "ko") == 0 || strcmp(buffer_ptr, "co") == 0)
       strcpy(output, "こ");

    else if (strcmp(buffer_ptr, "kya") == 0 || strcmp(buffer_ptr, "cya") == 0)
       strcpy(output, "きゃ");
    else if (strcmp(buffer_ptr, "kyu") == 0 || strcmp(buffer_ptr, "cyu") == 0)
       strcpy(output, "きゅ");
    else if (strcmp(buffer_ptr, "kyo") == 0 || strcmp(buffer_ptr, "cyo") == 0)
       strcpy(output, "きょ");

    else if (strcmp(buffer_ptr, "ga") == 0)
       strcpy(output, "が");
    else if (strcmp(buffer_ptr, "gi") == 0)
       strcpy(output, "ぎ");
    else if (strcmp(buffer_ptr, "gu") == 0)
       strcpy(output, "ぐ");
    else if (strcmp(buffer_ptr, "ge") == 0)
       strcpy(output, "げ");
    else if (strcmp(buffer_ptr, "go") == 0)
       strcpy(output, "ご");

    else if (strcmp(buffer_ptr, "gya") == 0)
       strcpy(output, "ぎゃ");
    else if (strcmp(buffer_ptr, "gyu") == 0)
       strcpy(output, "ぎゅ");
    else if (strcmp(buffer_ptr, "gyo") == 0)
       strcpy(output, "ぎょ");


    else if (strcmp(buffer_ptr, "sa") == 0)
       strcpy(output, "さ");
    else if (strcmp(buffer_ptr, "si") == 0 || strcmp(buffer_ptr, "shi") == 0)
       strcpy(output, "し");
    else if (strcmp(buffer_ptr, "su") == 0)
       strcpy(output, "す");
    else if (strcmp(buffer_ptr, "se") == 0)
       strcpy(output, "せ");
    else if (strcmp(buffer_ptr, "so") == 0)
       strcpy(output, "そ");

    else if (strcmp(buffer_ptr, "sya") == 0 || strcmp(buffer_ptr, "sha") == 0)
       strcpy(output, "しゃ");
    else if (strcmp(buffer_ptr, "syu") == 0 || strcmp(buffer_ptr, "shu") == 0)
       strcpy(output, "しゅ");
    else if (strcmp(buffer_ptr, "syo") == 0 || strcmp(buffer_ptr, "sho") == 0)
       strcpy(output, "しょ");

    else if (strcmp(buffer_ptr, "za") == 0)
       strcpy(output, "ざ");
    else if (strcmp(buffer_ptr, "zi") == 0 || strcmp(buffer_ptr, "ji") == 0)
       strcpy(output, "じ");
    else if (strcmp(buffer_ptr, "zu") == 0)
       strcpy(output, "ず");
    else if (strcmp(buffer_ptr, "ze") == 0)
       strcpy(output, "ぜ");
    else if (strcmp(buffer_ptr, "zo") == 0)
       strcpy(output, "ぞ");

    else if (strcmp(buffer_ptr, "zya") == 0 || strcmp(buffer_ptr, "jya") == 0
                                            || strcmp(buffer_ptr, "ja" ) == 0 )
       strcpy(output, "じゃ");
    else if (strcmp(buffer_ptr, "zyu") == 0 || strcmp(buffer_ptr, "jyu") == 0
                                            || strcmp(buffer_ptr, "ju" ) == 0 )
       strcpy(output, "じゅ");
    else if (strcmp(buffer_ptr, "zyo") == 0 || strcmp(buffer_ptr, "jyo") == 0
                                            || strcmp(buffer_ptr, "jo" ) == 0 )
       strcpy(output, "じょ");


    else if (strcmp(buffer_ptr, "ta") == 0)
       strcpy(output, "た");
    else if (strcmp(buffer_ptr, "ti") == 0 || strcmp(buffer_ptr, "chi") == 0)
       strcpy(output, "ち");
    else if (strcmp(buffer_ptr, "tu") == 0 || strcmp(buffer_ptr, "tsu") == 0)
       strcpy(output, "つ");
    else if (strcmp(buffer_ptr, "te") == 0)
       strcpy(output, "て");
    else if (strcmp(buffer_ptr, "to") == 0)
       strcpy(output, "と");

    else if (strcmp(buffer_ptr, "tya") == 0 || strcmp(buffer_ptr, "cha") == 0)
       strcpy(output, "ちゃ");
    else if (strcmp(buffer_ptr, "tyu") == 0 || strcmp(buffer_ptr, "chu") == 0)
       strcpy(output, "ちゅ");
    else if (strcmp(buffer_ptr, "tyo") == 0 || strcmp(buffer_ptr, "cho") == 0)
       strcpy(output, "ちょ");

    else if (strcmp(buffer_ptr, "da") == 0)
       strcpy(output, "だ");
    else if (strcmp(buffer_ptr, "di") == 0)
       strcpy(output, "ぢ");
    else if (strcmp(buffer_ptr, "du") == 0 || strcmp(buffer_ptr, "dsu") == 0)
       strcpy(output, "づ");
    else if (strcmp(buffer_ptr, "de") == 0)
       strcpy(output, "で");
    else if (strcmp(buffer_ptr, "do") == 0)
       strcpy(output, "ど");

    else if (strcmp(buffer_ptr, "dya") == 0)
       strcpy(output, "ぢゃ");
    else if (strcmp(buffer_ptr, "dyu") == 0)
       strcpy(output, "ぢゅ");
    else if (strcmp(buffer_ptr, "dyo") == 0)
       strcpy(output, "ぢょ");


    else if (strcmp(buffer_ptr, "na") == 0)
       strcpy(output, "な");
    else if (strcmp(buffer_ptr, "ni") == 0)
       strcpy(output, "に");
    else if (strcmp(buffer_ptr, "nu") == 0)
       strcpy(output, "ぬ");
    else if (strcmp(buffer_ptr, "ne") == 0)
       strcpy(output, "ね");
    else if (strcmp(buffer_ptr, "no") == 0)
       strcpy(output, "の");

    else if (strcmp(buffer_ptr, "nya") == 0)
       strcpy(output, "にゃ");
    else if (strcmp(buffer_ptr, "nyu") == 0)
       strcpy(output, "にゅ");
    else if (strcmp(buffer_ptr, "nyo") == 0)
       strcpy(output, "にょ");


    else if (strcmp(buffer_ptr, "ha") == 0)
       strcpy(output, "は");
    else if (strcmp(buffer_ptr, "hi") == 0)
       strcpy(output, "ひ");
    else if (strcmp(buffer_ptr, "hu") == 0 || strcmp(buffer_ptr, "fu") == 0)
       strcpy(output, "ふ");
    else if (strcmp(buffer_ptr, "he") == 0)
       strcpy(output, "へ");
    else if (strcmp(buffer_ptr, "ho") == 0)
       strcpy(output, "ほ");

    else if (strcmp(buffer_ptr, "hya") == 0)
       strcpy(output, "ひゃ");
    else if (strcmp(buffer_ptr, "hyu") == 0)
       strcpy(output, "ひゅ");
    else if (strcmp(buffer_ptr, "hyo") == 0)
       strcpy(output, "ひょ");
   
    else if (strcmp(buffer_ptr, "ba") == 0)
       strcpy(output, "ば");
    else if (strcmp(buffer_ptr, "bi") == 0)
       strcpy(output, "び");
    else if (strcmp(buffer_ptr, "bu") == 0)
       strcpy(output, "ぶ");
    else if (strcmp(buffer_ptr, "be") == 0)
       strcpy(output, "べ");
    else if (strcmp(buffer_ptr, "bo") == 0)
       strcpy(output, "ぼ");

    else if (strcmp(buffer_ptr, "bya") == 0)
       strcpy(output, "びゃ");
    else if (strcmp(buffer_ptr, "byu") == 0)
       strcpy(output, "びゅ");
    else if (strcmp(buffer_ptr, "byo") == 0)
       strcpy(output, "びょ");

    else if (strcmp(buffer_ptr, "pa") == 0)
       strcpy(output, "ぱ");
    else if (strcmp(buffer_ptr, "pi") == 0)
       strcpy(output, "ぴ");
    else if (strcmp(buffer_ptr, "pu") == 0)
       strcpy(output, "ぷ");
    else if (strcmp(buffer_ptr, "pe") == 0)
       strcpy(output, "ぺ");
    else if (strcmp(buffer_ptr, "po") == 0)
       strcpy(output, "ぽ");

    else if (strcmp(buffer_ptr, "pya") == 0)
       strcpy(output, "ぴゃ");
    else if (strcmp(buffer_ptr, "pyu") == 0)
       strcpy(output, "ぴゅ");
    else if (strcmp(buffer_ptr, "pyo") == 0)
       strcpy(output, "ぴょ");


    else if (strcmp(buffer_ptr, "ma") == 0)
       strcpy(output, "ま");
    else if (strcmp(buffer_ptr, "mi") == 0)
       strcpy(output, "み");
    else if (strcmp(buffer_ptr, "mu") == 0)
       strcpy(output, "む");
    else if (strcmp(buffer_ptr, "me") == 0)
       strcpy(output, "め");
    else if (strcmp(buffer_ptr, "mo") == 0)
       strcpy(output, "も");

    else if (strcmp(buffer_ptr, "mya") == 0)
       strcpy(output, "みゃ");
    else if (strcmp(buffer_ptr, "myu") == 0)
       strcpy(output, "みゅ");
    else if (strcmp(buffer_ptr, "myo") == 0)
       strcpy(output, "みょ");


    else if (strcmp(buffer_ptr, "ya") == 0)
       strcpy(output, "や");
    else if (strcmp(buffer_ptr, "yu") == 0)
       strcpy(output, "ゆ");
    else if (strcmp(buffer_ptr, "yo") == 0)
       strcpy(output, "よ");


    else if (strcmp(buffer_ptr, "ra") == 0 || strcmp(buffer_ptr, "la") == 0)
       strcpy(output, "ら");
    else if (strcmp(buffer_ptr, "ri") == 0 || strcmp(buffer_ptr, "li") == 0)
       strcpy(output, "り");
    else if (strcmp(buffer_ptr, "ru") == 0 || strcmp(buffer_ptr, "lu") == 0)
       strcpy(output, "る");
    else if (strcmp(buffer_ptr, "re") == 0 || strcmp(buffer_ptr, "le") == 0)
       strcpy(output, "れ");
    else if (strcmp(buffer_ptr, "ro") == 0 || strcmp(buffer_ptr, "lo") == 0)
       strcpy(output, "ろ");

    else if (strcmp(buffer_ptr, "rya") == 0 || strcmp(buffer_ptr, "lya") == 0)
       strcpy(output, "りゃ");
    else if (strcmp(buffer_ptr, "ryu") == 0 || strcmp(buffer_ptr, "lyu") == 0)
       strcpy(output, "りゅ");
    else if (strcmp(buffer_ptr, "ryo") == 0 || strcmp(buffer_ptr, "lyo") == 0)
       strcpy(output, "りょ");


    else if (strcmp(buffer_ptr, "wa") == 0)
       strcpy(output, "わ");
    else if (strcmp(buffer_ptr, "wi") == 0)
       strcpy(output, "うぃ");
    else if (strcmp(buffer_ptr, "we") == 0)
       strcpy(output, "うぇ");
    else if (strcmp(buffer_ptr, "wo") == 0)
       strcpy(output, "を");

    else if (strcmp(buffer_ptr, "va") == 0)
       strcpy(output, "う゛ぁ");
    else if (strcmp(buffer_ptr, "vi") == 0)
       strcpy(output, "う゛ぃ");
    else if (strcmp(buffer_ptr, "ve") == 0)
       strcpy(output, "う゛ぇ");
    else if (strcmp(buffer_ptr, "vo") == 0)
       strcpy(output, "う゛ぉ");


    else if (strcmp(buffer_ptr, "xa") == 0)
       strcpy(output, "ぁ");
    else if (strcmp(buffer_ptr, "xi") == 0)
       strcpy(output, "ぃ");
    else if (strcmp(buffer_ptr, "xu") == 0)
       strcpy(output, "ぅ");
    else if (strcmp(buffer_ptr, "xe") == 0)
       strcpy(output, "ぇ");
    else if (strcmp(buffer_ptr, "xo") == 0)
       strcpy(output, "ぉ");


    else if (strcmp(buffer_ptr, "fa") == 0)
       strcpy(output, "ふぁ");
    else if (strcmp(buffer_ptr, "fi") == 0)
       strcpy(output, "ふぃ");
    else if (strcmp(buffer_ptr, "fe") == 0)
       strcpy(output, "ふぇ");
    else if (strcmp(buffer_ptr, "fo") == 0)
       strcpy(output, "ふぉ");
   

    else if (strcmp(buffer_ptr, "-") == 0)
       strcpy(output, "ー");

    else
    {
      input_ptr = NULL;
      return NULL;
    }

    return output;
}


//!
//! @brief Convenience function to convert romaji to hiragana
//!
//! @param input The string to shift.
//! @param output the string to output the changes to.
//! @param max The max length of the string to output to.
//! @see lw_util_shift_kana_chars_in_str_by ()
//! @see lw_util_str_shift_hira_to_kata ()
//!
gboolean 
lw_util_str_roma_to_hira (const gchar* input, gchar* output, gint max)
{
    //Sanity checks
    g_return_val_if_fail (input != NULL, FALSE);
    g_return_val_if_fail (output != NULL, FALSE);

    //Declarations
    const gchar *input_ptr;
    gchar *kana_ptr;
    gint leftover;

    //Initializations
    input_ptr = input;
    kana_ptr = output;
    *kana_ptr = '\0';
    leftover = max;

    //Try converting to hiragana
    while (leftover-- > 0)
    {
      kana_ptr = lw_util_roma_char_to_hira (input_ptr, kana_ptr);
      if (kana_ptr == NULL || input_ptr == NULL)
        break;
      input_ptr = lw_util_next_hira_char_from_roma (input_ptr);
      if (kana_ptr == NULL || input_ptr == NULL)
        break;

      kana_ptr = &kana_ptr[strlen(kana_ptr)];
    }

    return (input_ptr != NULL && strlen (input_ptr) == 0);
}


//!
//! @brief Prepare an input query string
//!
//! Run some checks and transformation on a string before using it for a search :
//!  * Check for badly encoded UTF-8 or invalid character
//!  * Replace halfwidth japanese characters with their normal wide counterpart
//!
//! @param input an utf8 encoded string to prepare
//! @param strip if true remove leading and trailing spaces
//! @return a newly allocated utf8 encoded string or NULL if text was too.
//!         If the result is non-NULL it must be freed with g_free().
gchar* 
lw_util_prepare_query (const char* input, gboolean strip)
{
    //Sanity check
    g_assert (input != NULL);

    //Declarations
    char *output;
    char *buffer;

    //Initializations
    output = lw_util_sanitize_input (input, strip);

    if(lw_util_contains_halfwidth_japanese (output) == TRUE)
    {
      buffer = output;
      output = lw_util_enlarge_halfwidth_japanese (buffer);
      g_free (buffer);
    }

    return output;
}


//!
//! @brief Sanitize an input string
//!
//! This function will check if the input string is a valid utf-8 sequence,
//! it will then normalize this string in the Normalization Form Canonical Composition,
//! then replace the bytes of unprintable unicode glyphe (like control codepoint) with spaces,
//! and finally will remove leading and trailing spaces if asked to.
//!
//! @param text an utf8 encoded string to sanitize
//! @param strip if true remove leading and trailing spaces
//! @return a newly allocated sanitized utf8 encoded string or NULL if text was too.
//!         If the result is non-NULL it must be freed with g_free(). 
//!
gchar* 
lw_util_sanitize_input (const char *text, gboolean strip)
{
    //Sanity Check
    g_assert (text != NULL);
    
    // First validate the utf8 input data
    char *end; // pointer to the valid end of utf8, it is before or at the end of *text 
    if (!g_utf8_validate(text, -1, (const char **) &end))
      *end = '\0'; // uh oh, was not valid utf8, let's put a stop at the last valid position
      
    // Then let's normalize utf8 : there can be several encodings for same glyph, 
    // let's always use the same for the sake of consistency
    // see http://library.gnome.org/devel/glib/stable/glib-Unicode-Manipulation.html#g-utf8-normalize
    // and http://en.wikipedia.org/wiki/Unicode_normalization
    char *ntext = g_utf8_normalize (text, -1,  G_NORMALIZE_NFC ); // this allocate a new char*
    
    // for each unicode symbol replace unprintable symbol bytes with spaces  
    char *ptr = ntext;  // pointer to the start of current glyph
    char *next = NULL;  // pointer to the start of original next glyph
    while (*ptr != '\0') 
    {
      next = g_utf8_next_char (ptr);
      if (!g_unichar_isprint (g_utf8_get_char (ptr)) )
        strncpy(ptr, "         ", next - ptr);
      ptr = next;
    }

    if(strip)    
      g_strstrip (ntext); // no new allocation, just modifying the string
   
    return ntext;
}

//!
//! @brief Check if an input string contains char from the halfwidth japanese unicode range
//!
//! This function iterate over the string to check if it contains char from the halfwidth japanese unicode range
//! (from U+FF61 HALFWIDTH IDEOGRAPHIC FULL STOP to U+FF9F HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK).
//!
//! @param text an utf8 encoded string
//! @return TRUE if the string contains is not null and contains a halfwidth japanese char
//!
gboolean 
lw_util_contains_halfwidth_japanese (const gchar* text)
{
    //Sanity check
    if(text == NULL)
      return FALSE;

    //Declarations
    gunichar ucp;
    const gchar *ptr;

    //Initializations
    ptr = text; 

    while (*ptr != '\0')
    {
      ucp = g_utf8_get_char (ptr);
      if(ucp >=0xFF61 && ucp <=0xFF9F) // Halfwidth block
        return TRUE;
      ptr = g_utf8_next_char (ptr);
    }

    return FALSE;
}

//!
//! @brief Replace all halfwidth japanese char with their ordinary wide equivalent
//!
//! This function create a copy of the text where char from the halfwidth japanese unicode range
//! (from U+FF61 HALFWIDTH IDEOGRAPHIC FULL STOP to U+FF9F HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK)
//! are replaced by their ordinary wide equivalent.
//!
//! @param text an utf8 encoded string with halfwidth japanese char to expand.
//! @return a newly allocated utf8 encoded string without halfwidth japanese char ; or NULL if text was too.
//!         If the result is non-NULL it must be freed with g_free().
//!
gchar* 
lw_util_enlarge_halfwidth_japanese (const gchar* text)
{
    if(text == NULL)
        return NULL;

    //Declarations
    const gchar *ptr;
    GString* nstr;
    gunichar ucp;

    //Initializations
    ptr = text; 
    nstr = g_string_new( NULL );
    ucp = 0;

    while (*ptr != '\0')
    {
      ucp = g_utf8_get_char (ptr);
      if(ucp >=0xFF61 && ucp <=0xFF9F) // Halfwidth block
      {
        // The G_NORMALIZE_ALL will remove the narrow modifier and return the normal wided char
        // We know halfwidth char from this range take 3 bytes in utf8
        char *nch = g_utf8_normalize (ptr, 3,  G_NORMALIZE_ALL ); // this allocate a new char*
        g_string_append (nstr, nch);
        g_free(nch);
      }
      else // just appending the char to the result
      {
        g_string_append_unichar (nstr, ucp);
      }

      ptr = g_utf8_next_char (ptr); // next !
    }

    return g_string_free(nstr, FALSE);
}


//!
//! @brief Checks for a Japanese local messages setting
//! @returns A boolean stating whether the locale is a Japanese utf8 one
//! @return Returns true if it is a japanese local
//!
gboolean 
lw_util_is_japanese_locale ()
{
    return (setlocale(LC_ALL, NULL) != NULL &&
             (
               strcmp(setlocale(LC_ALL, NULL), "ja_JP.UTF8")  == 0 ||
               strcmp(setlocale(LC_ALL, NULL), "ja_JP.UTF-8") == 0 ||
               strcmp(setlocale(LC_ALL, NULL), "ja_JP.utf8")  == 0 ||
               strcmp(setlocale(LC_ALL, NULL), "ja_JP.utf-8") == 0 ||
               strcmp(setlocale(LC_ALL, NULL), "ja_JP")       == 0 ||
               strcmp(setlocale(LC_ALL, NULL), "ja")          == 0 ||
               strcmp(setlocale(LC_ALL, NULL), "japanese")    == 0
             )
           );
}


//!
//! @brief Returns the furigana atoms in a string as an array of atoms
//! @param string The string to get the furigana atoms from
//! @returns An allocated array of strings that should be freed with g_strfreev()
//!
char** 
lw_util_get_romaji_atoms_from_string (const char *string)
{
    //Declarations
    gunichar character;
    GUnicodeScript script;
    char *buffer;
    const char *string_ptr;
    char *buffer_ptr;
    const char* delimitor;
    char **string_array;
    gboolean new_atom_start;
    int offset;
    gboolean first_romaji_found;

    //Initializations;
    delimitor = "&";
    buffer = (char*) malloc(sizeof(char) * (strlen(string) * 2) + 1); //max size is if there is a delimitor for every character
    buffer[0] = '\0';
    string_ptr = string;
    buffer_ptr = buffer;
    new_atom_start = FALSE;
    first_romaji_found = FALSE;

    //Create a string with only furigana atoms delimited by the delimitor
    while (*string_ptr != '\0')
    {
      character = g_utf8_get_char (string_ptr);
      script = g_unichar_get_script (character);
      if (strncmp(buffer_ptr, delimitor, strlen(delimitor)) == 0)
      {
        new_atom_start = TRUE;
        string_ptr = g_utf8_next_char (string_ptr);
      }
      else if (script == G_UNICODE_SCRIPT_LATIN || script == G_UNICODE_SCRIPT_COMMON)
      {
        if (new_atom_start && buffer_ptr != buffer && first_romaji_found == FALSE)
        {
          new_atom_start = FALSE;
          strcpy(buffer_ptr, delimitor);
          buffer_ptr += strlen(delimitor);
        }

        first_romaji_found = TRUE;
        offset = g_unichar_to_utf8 (character, buffer_ptr);
        buffer_ptr += offset;
        *buffer_ptr = '\0';
      }
      else
      {
        new_atom_start = TRUE;
      }
      string_ptr = g_utf8_next_char (string_ptr);
    }
    *buffer_ptr = '\0';

    //Convert the string into an array of strings
    string_array = g_strsplit (buffer, delimitor, -1);
    
    //Cleanup
    free(buffer);

    //Finish
    return string_array;
}


//!
//! @brief Returns the furigana atoms in a string as an array of atoms
//! @param string The string to get the furigana atoms from
//! @returns An allocated array of strings that should be freed with g_strfreev()
//!
char** 
lw_util_get_furigana_atoms_from_string (const char *string)
{
    //Declarations
    gunichar character;
    GUnicodeScript script;
    char *buffer;
    const char *string_ptr;
    char *buffer_ptr;
    const char* delimitor;
    char **string_array;
    int offset;
    gboolean new_atom_start;

    //Initializations;
    delimitor = "&";
    buffer = (char*) malloc(sizeof(char) * (strlen(string) * 2) + 1); //max size is if there is a delimitor for every character
    buffer[0] = '\0';
    string_ptr = string;
    buffer_ptr = buffer;
    new_atom_start = FALSE;

    //Create a string with only furigana atoms delimited by the delimitor
    while (*string_ptr != '\0')
    {
      character = g_utf8_get_char (string_ptr);
      script = g_unichar_get_script (character);
      if (script == G_UNICODE_SCRIPT_KATAKANA || script == G_UNICODE_SCRIPT_HIRAGANA)
      {
        if (new_atom_start && buffer_ptr != buffer)
        {
          new_atom_start = FALSE;
          strcpy(buffer_ptr, delimitor);
          buffer_ptr += strlen(delimitor);
        }

        offset = g_unichar_to_utf8 (character, buffer_ptr);
        buffer_ptr += offset;
        *buffer_ptr = '\0';
      }
      else
      {
        new_atom_start = TRUE;
      }
      string_ptr = g_utf8_next_char (string_ptr);
    }
    *buffer_ptr = '\0';

    //Convert the string into an array of strings
    string_array = g_strsplit (buffer, delimitor, -1);
    
    //Cleanup
    free(buffer);

    //Finish
    return string_array;
}


//!
//! @brief Converts the arguments passed to the program into a query search string
//! @param argc The argc argument passed to main
//! @param argv The argv argument passed to main
//! @returns An allocated string that must be freed with g_free
//!
gchar* 
lw_util_get_query_from_args (int argc, char** argv)
{
    //Sanity check
    if (argc < 2) return NULL;

    //Declarations
    char *text;
    char *query;
    int i;
    int length;
    char *ptr;

    //Initializations
    text = NULL;
    query = NULL;
    length = 0;

    //Get the required length of the combined string
    for (i = 1; i < argc; i++) 
    {
      length += strlen (argv[i]) + 1;
    }

    //Allocate it and set up the iterator
    text = (char*) malloc(length * sizeof(char) + 1);
    if (text == NULL) return NULL;
    ptr = text;

    //Copy the argument words into the query
    for (i = 1; i < argc; i++)
    { 
      strcpy(ptr, argv[i]);
      ptr += strlen(argv[i]);

      if (i == argc - 1) break;

      strcpy(ptr, " ");
      ptr += strlen(" ");
    }

     query = lw_util_prepare_query (text, FALSE);

    //Cleanup
    if (text != NULL) free (text);

    return query;
}


gchar*
lw_strjoinv (gchar delimitor, gchar** array, gint array_length)
{
    g_assert (array != NULL);

    //Declarations
    gint text_length;
    gint delimitor_length;
    gchar *text, *src_ptr, *tgt_ptr;
    gint i;

    //Initializations
    text_length = 0;
    delimitor_length = sizeof (delimitor);
    i = 0;

    //Calculate the needed size
    while (i < array_length)
    {
      src_ptr = *(array + i);
      if (src_ptr != NULL) 
      {
        text_length += strlen (src_ptr);
      }
      text_length += delimitor_length;
      i++;
    }

    text = g_new (gchar, text_length);

    //Concatinate the strings
    if (text != NULL)
    {
      tgt_ptr = text;
      i = 0;

      while (i < array_length)
      {
        src_ptr = *(array + i);
        if (src_ptr != NULL)
          while (*src_ptr != '\0')
            *(tgt_ptr++) = *(src_ptr++);
        *(tgt_ptr++) = delimitor;
        i++;
      }
      *(--tgt_ptr) = '\0';
    }

    return text;
}


gchar*
lw_util_collapse_string (const gchar *text)
{
    gchar *buffer;
    gchar *target_ptr;
    const gchar *source_ptr;
    gunichar c;
    gint bytes;

    buffer = g_new (gchar, strlen(text) + 1);

    if (buffer != NULL)
    {
      source_ptr = text;
      target_ptr = buffer;

      while (*source_ptr != '\0')
      {
        c = g_unichar_tolower (g_utf8_get_char (source_ptr));

        if (!g_unichar_ispunct (c) && !g_unichar_isspace (c))
        {
          bytes = g_unichar_to_utf8 (c, target_ptr);
          target_ptr += bytes;
        }

        source_ptr = g_utf8_next_char (source_ptr);
      }
      *target_ptr = '\0';
    }

    return buffer;
}


static gboolean
lw_util_script_changed (GUnicodeScript p, GUnicodeScript n, gboolean split_kanji_furigana)
{
    //Declarations
    gboolean has_common;
    gboolean has_changed;
    gboolean is_japanese_p;
    gboolean is_japanese_n;
    gboolean is_japanese_change;

    
    //Initializations;
    has_common = (p == G_UNICODE_SCRIPT_INVALID_CODE ||	n == G_UNICODE_SCRIPT_COMMON || p == G_UNICODE_SCRIPT_COMMON);
    has_changed = (p != n && !has_common);
    is_japanese_p = (p == G_UNICODE_SCRIPT_HAN || p == G_UNICODE_SCRIPT_HIRAGANA || p == G_UNICODE_SCRIPT_KATAKANA);
    is_japanese_n = (n == G_UNICODE_SCRIPT_HAN || n == G_UNICODE_SCRIPT_HIRAGANA || n == G_UNICODE_SCRIPT_KATAKANA);
    is_japanese_change = (is_japanese_p && is_japanese_n  && p != n);


    if (is_japanese_change) return (split_kanji_furigana);
    else return (has_changed);
}


gchar*
lw_util_delimit_script_changes (const gchar *DELIMITOR, const gchar* TEXT, gboolean split_kanji_furigana)
{
    //Sanity check
    g_return_val_if_fail (DELIMITOR != NULL && TEXT != NULL, NULL);

    //Declarations
    gchar *buffer;
    gint count;
    const gchar *source_ptr;
    gchar *target_ptr;
    GUnicodeScript this_script, previous_script;
    gunichar c;
    gboolean script_changed;
    gint delimitor_length;

    //Initializations
    count = 0;
    delimitor_length = strlen (DELIMITOR);
    this_script = previous_script = G_UNICODE_SCRIPT_INVALID_CODE;
    for (source_ptr = TEXT; *source_ptr != '\0'; source_ptr = g_utf8_next_char (source_ptr))
    {
      c = g_utf8_get_char (source_ptr);
      this_script = g_unichar_get_script (c);
      script_changed = lw_util_script_changed (previous_script, this_script, split_kanji_furigana);

      if (script_changed)
      {
				count++;
      }

      previous_script = this_script;
    }

    buffer = g_new (gchar, strlen(TEXT) + (delimitor_length * count) + 1);
		if (buffer != NULL)
		{
      target_ptr = buffer;
			*buffer = '\0';
			this_script = previous_script = G_UNICODE_SCRIPT_INVALID_CODE;
			for (source_ptr = TEXT; *source_ptr != '\0'; source_ptr = g_utf8_next_char (source_ptr))
			{
				c = g_utf8_get_char (source_ptr);
				this_script = g_unichar_get_script (c);
        script_changed = lw_util_script_changed (previous_script, this_script, split_kanji_furigana);

        if (script_changed)
				{
					strcpy(target_ptr, DELIMITOR);
					target_ptr += delimitor_length;
				}
        target_ptr += g_unichar_to_utf8 (c, target_ptr);
        *target_ptr = '\0';

				previous_script = this_script;
			}
		}

    return buffer;
}


gchar*
lw_util_delimit_whitespace (const gchar *DELIMITOR, const gchar* TEXT)
{
    //Sanity check
    g_return_val_if_fail (DELIMITOR != NULL && TEXT != NULL, NULL);

    //Declarations
    gchar *buffer;
    gint count;
    const gchar *source_ptr;
    gchar *target_ptr;
    gunichar c;
    gint delimitor_length;
    gboolean inserted_delimitor;
    gboolean is_whitespace;

    //Initializations
    count = 0;
    delimitor_length = strlen(DELIMITOR);
    inserted_delimitor = FALSE;
    for (source_ptr = TEXT; *source_ptr != '\0'; source_ptr = g_utf8_next_char (source_ptr))
    {
      c = g_utf8_get_char (source_ptr);
      is_whitespace = g_unichar_isspace(c);

      if (is_whitespace)
      {
        count++;
      }
    }
    buffer = g_new (gchar, strlen(TEXT) + (delimitor_length * count) + 1);
    target_ptr = buffer;

    //Create the delimited string
    if (buffer != NULL)
    {
      for (source_ptr = TEXT; *source_ptr != '\0'; source_ptr = g_utf8_next_char (source_ptr))
      {
        c = g_utf8_get_char (source_ptr);
        is_whitespace = g_unichar_isspace(c);

        if (is_whitespace && !inserted_delimitor)
        {
          strcpy(target_ptr, DELIMITOR);
          target_ptr += delimitor_length;
          inserted_delimitor = TRUE;
        }
        else if (!is_whitespace)
        {
          target_ptr += g_unichar_to_utf8 (c, target_ptr);
          *target_ptr = '\0';
          inserted_delimitor = FALSE;
        }
      }
    }

    return buffer;
}


gchar*
lw_util_delimit_radicals (const gchar *DELIMITOR, const gchar* TEXT)
{
    //Sanity check
    g_return_val_if_fail (DELIMITOR != NULL && TEXT != NULL, NULL);

    //Declarations
    gchar *buffer;
    gint count;
    const gchar *source_ptr;
    gchar *target_ptr;
    gunichar c;
    gint delimitor_length;
    GUnicodeScript script;
    GUnicodeScript previous_script;

    //Initializations
    count = 0;
    delimitor_length = strlen(DELIMITOR);
    previous_script = G_UNICODE_SCRIPT_INVALID_CODE;

    for (source_ptr = TEXT; *source_ptr != '\0'; source_ptr = g_utf8_next_char (source_ptr))
    {
      c = g_utf8_get_char (source_ptr);
      script = g_unichar_get_script (c);

      if (previous_script == G_UNICODE_SCRIPT_HAN && script == previous_script)
      {
        count++;
      }
      previous_script = script;
    }

    buffer = g_new (gchar, strlen(TEXT) + (delimitor_length * count) + 1);
    target_ptr = buffer;
    previous_script = G_UNICODE_SCRIPT_INVALID_CODE;

    //Create the delimited string
    if (buffer != NULL)
    {
      for (source_ptr = TEXT; *source_ptr != '\0'; source_ptr = g_utf8_next_char (source_ptr))
      {
        c = g_utf8_get_char (source_ptr);
        script = g_unichar_get_script (c);

        if (previous_script == G_UNICODE_SCRIPT_HAN && script == previous_script)
        {
          strcpy(target_ptr, DELIMITOR);
          target_ptr += delimitor_length;
        }

        target_ptr += g_unichar_to_utf8 (c, target_ptr);
        *target_ptr = '\0';

        previous_script = script;
      }
    }

    return buffer;
}


GRegex*
lw_regex_new (const gchar *PATTERN, const gchar *EXPRESSION, GError **error)
{
    //Declarations
    GRegex *regex;
    gchar *expression;

    //Initializations
    expression = g_strdup_printf (PATTERN, EXPRESSION);
    regex = NULL;

    if (expression != NULL)
    {
      regex = g_regex_new (expression,  (G_REGEX_CASELESS | G_REGEX_OPTIMIZE), 0, error);
      g_free (expression); expression = NULL;
    }

    return regex;
}



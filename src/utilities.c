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
//! @file src/utilities.c
//!
//! @brief Generic utility functions
//!
//! Holds some basic functions that are extremely handy for gWaei.
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/utilities.h>

static GwRunmode run_mode;
static char *gw_waei_folder_path = NULL;


//!
//! @brief Sets the runmode of the program
//!
//! The runmode describes tells what interface the program will be using to
//! primarily interact with the user.  The two possible choices are GTK or
//! the console.  QT is currently a red herring.
//!
//! @param argc Standard main arguments
//! @param argv Standard main arguments
//!
void gw_util_initialize_runmode (int argc, char *argv[])
{
   char *call_ptr = argv[0];
   call_ptr = &call_ptr[strlen(call_ptr)];
   while (call_ptr != argv[0] && !G_IS_DIR_SEPARATOR (*call_ptr))
     call_ptr--;
   if (G_IS_DIR_SEPARATOR (*call_ptr))
     call_ptr++;

   gboolean ncurses_switch = FALSE;
   //char *ptr = argv[0];
   int i = 0;
   while (i < argc && ncurses_switch == FALSE)
   {
     //WARNING this is a lazy implimentation...
     ncurses_switch = (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--ncurses") == 0);
     i++;
   }

   if (strcmp (call_ptr, "waei") == 0)
   {
     if (ncurses_switch)
     {
       run_mode = GW_NCURSES_RUNMODE;
     }
     else
     {
       run_mode = GW_CONSOLE_RUNMODE;
     }
   }
#ifdef WITH_GTK
   else if (strcmp (call_ptr, "gwaei") == 0)
   {
     run_mode = GW_GTK_RUNMODE;
   }
#endif
#ifdef WITH_QT
   else if (strcmp (call_ptr, "kwaei") == 0)
   {
     run_mode = GW_QT_RUNMODE;
   }
#endif
   else
   {
     run_mode = GW_CONSOLE_RUNMODE;
   }
}


//!
//! @brief Gets the runmode of the program
//!
//! The runmode describes tells what interface the program will be using to
//! primarily interact with the user.  The two possible outputs:
//! GW_CONSOLE_RUNMODE or GW_GTK_RUNMODE.  QT is currently a red herring.
//!
//! @return Returns the current runmode as an integer
//!
GwRunmode gw_util_get_runmode ()
{
    return run_mode;
}


//!
//! @brief Gets the location of the waei dictionary folder
//!
//! @param a pre-allocated string to write into
//! @param an integer representing a color
//! @return Always returns true
//!
const char* gw_util_get_waei_directory() 
{
    if (gw_waei_folder_path != NULL)
    {
      g_free (gw_waei_folder_path);
      gw_waei_folder_path = NULL;
    }

    gw_waei_folder_path = g_build_filename (g_get_home_dir (), ".waei", NULL);

    if(gw_waei_folder_path != NULL && g_mkdir_with_parents (gw_waei_folder_path, 0755) == -1)
    {
      return NULL;
    }
    
    return gw_waei_folder_path;
}


//!
//! @brief Makes sure that all of the characters are in a specific range
//!
//! This function was made for the general purpose of quickly figuring out if
//! a character string is all katakana, hiragana, or romaji.
//!
//! @param input The string to check
//! @param start_unic_boundary the lower bound
//! @param end_unic_boundary the upper bound
//! @return Returns true if it is in the range
//! @see gw_util_is_hiragana_str ()
//! @see gw_util_is_katakana_str ()
//! @see gw_util_is_kanji_str ()
//! @see gw_util_is_romaji_str ()
//!
gboolean gw_util_all_chars_are_in_range (char input[],
                                         int  start_unic_boundary,
                                         int  end_unic_boundary   )
{
    //Setup
    char *input_ptr;
    input_ptr = input;

    gunichar unic;
    unic = g_utf8_get_char(input_ptr);

    gboolean is_in_boundary;
    is_in_boundary = (unic >= start_unic_boundary && unic <= end_unic_boundary);

    //Check
    while (*input_ptr != '\0' && is_in_boundary)
    {
      input_ptr = g_utf8_next_char(input_ptr);
      unic = g_utf8_get_char(input_ptr);
      is_in_boundary = ((unic >= start_unic_boundary && unic <= end_unic_boundary) || unic == '.'
                                                                                   || unic == '*'
                                                                                   || (unic >= 0 && unic <= 9)
                                                                                   || unic == '-'
                                                                                   || unic == '^'
                                                                                   || unic == '$'
                                                                                   || unic == L'ー'
                                                                                   || unic == '+'
                                                                                   || unic == '?'
                                                                                   || unic == '{'
                                                                                   || unic == '/'
                                                                                   || unic == '\\'
                                                                                   || unic == '!'
                                                                                   || unic == '{'
                                                                                   || unic == '}'
                                                                                   || unic == '['
                                                                                   || unic == ']'
                                                                                   || unic == '&'
                                                                                   || unic == '|'
                                                                                   || unic == ' '
                                                                                   || unic == '('
                                                                                   || unic == ')');
    }

    //Return Results
    return (input[0] != '\0' && *input_ptr == '\0');
}


//!
//! @brief Convenience function for seeing if a string is hiragana
//!
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see gw_util_all_chars_are_in_range ()
//! @see gw_util_is_katakana_str ()
//! @see gw_util_is_kanji_str ()
//! @see gw_util_is_romaji_str ()
//!
gboolean gw_util_is_hiragana_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'ぁ', L'ん');
}


//!
//! @brief Convenience function for seeing if a string is katakana
//!
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see gw_util_all_chars_are_in_range ()
//! @see gw_util_is_hiragana_str ()
//! @see gw_util_is_kanji_str ()
//! @see gw_util_is_romaji_str ()
//!
gboolean gw_util_is_katakana_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'ァ', L'ー');
}


//!
//! @brief Convenience function for seeing if a string is furigana
//!
//! Furigana are the characters containing both hiragana and katakana.
//!
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see gw_util_all_chars_are_in_range ()
//! @see gw_util_is_hiragana_str ()
//! @see gw_util_is_kanji_str ()
//! @see gw_util_is_romaji_str ()
//!
gboolean gw_util_is_furigana_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'ぁ', L'ー');
}



//!
//! @brief Convenience function for seeing if a string *starts* with kanji
//!
//! The point of the function is to find a word that starts with kanji but
//! may also have hiragana in it somewheres.
//!
//! @param input The string to check
//! @return Returns true if the function things this is a kanji string
//! @see gw_util_all_chars_are_in_range ()
//! @see gw_util_is_hiragana_str ()
//! @see gw_util_is_katakana_str ()
//! @see gw_util_is_romaji_str ()
//!
gboolean gw_util_is_kanji_ish_str (char input[])
{
    gboolean hira = gw_util_is_hiragana_str (input);
    gboolean kata = gw_util_is_katakana_str (input);
    gboolean roma = gw_util_is_romaji_str (input);
    gboolean kanji = gw_util_is_kanji_str (input);

    return (!hira && !kata && !roma && !kanji);
}

//!
//! @brief Convenience function for seeing if a string is kanji
//!
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see gw_util_all_chars_are_in_range ()
//! @see gw_util_is_hiragana_str ()
//! @see gw_util_is_katakana_str ()
//! @see gw_util_is_romaji_str ()
//!
gboolean gw_util_is_kanji_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'ー', 0xFF00);
}


//!
//! @brief Convenience function for seeing if a string is romaji
//!
//! @param input The string to check
//! @return Returns true if it is in the range
//! @see gw_util_all_chars_are_in_range ()
//! @see gw_util_is_hiragana_str ()
//! @see gw_util_is_katakana_str ()
//! @see gw_util_is_kanji_str ()
//!
gboolean gw_util_is_romaji_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'A', L'ž');
}


//!
//! @brief Shifts the characters in a specific direction
//!
//! This function is used for hiragana to katakana conversions and vice versa.
//!
//! @param input The string to shift
//! @param shift How much to shift by
//! @see gw_util_str_shift_hira_to_kata ()
//! @see gw_util_str_shift_kata_to_hira ()
//!
void gw_util_shift_all_chars_in_str_by (char input[], int shift)
{
    //Setup
    char *input_ptr;
    input_ptr = input;

    char output[MAX_QUERY];
    char *output_ptr;
    output_ptr = output;

    gunichar unic;
    unic = g_utf8_get_char(input_ptr);

    gint offset = 0;

    //Start the conversion
    while (*input_ptr != '\0')
    {
      if (unic == L'ー')
        offset = g_unichar_to_utf8((unic), output_ptr);
      else
        offset = g_unichar_to_utf8((unic + shift), output_ptr);
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
//! @see gw_util_shift_all_chars_in_str_by ()
//! @see gw_util_str_shift_kata_to_hira ()
//!
void gw_util_str_shift_hira_to_kata (char input[])
{
    gw_util_shift_all_chars_in_str_by (input, (L'ア' - L'あ'));
}


//!
//! @brief Convenience function to shift katakana to hiragana
//!
//! @param input The string to shift
//! @see gw_util_shift_all_chars_in_str_by ()
//! @see gw_util_str_shift_hira_to_kata ()
//!
void gw_util_str_shift_kata_to_hira (char input[])
{
    gw_util_shift_all_chars_in_str_by (input, (L'あ' - L'ア'));
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
char* gw_util_next_hira_char_from_roma (char *input)
{
    char *input_ptr;
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
//!
//! Attempts to convert a romaji string to hiragana.
//!
//! @param input The source romaji string
//! @param input The string to write the hiragana equivalent to
//! @return Returns null on error/end
//!
char* gw_util_roma_char_to_hira (char *input, char *output)
{
    //Set up the input pointer
    char *input_ptr;
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

    else return NULL;

    return output;
}


//!
//! @brief Convenience function to convert romaji to hiragana
//!
//! @param input The string to shift
//! @see gw_util_shift_all_chars_in_str_by ()
//! @see gw_util_str_shift_hira_to_kata ()
//!
gboolean gw_util_str_roma_to_hira (char* input, char* output, int max)
{
    //Try converting to hiragana
    char *input_ptr = input;
    char *kana_ptr = output;
    *kana_ptr = '\0';
    int leftover = max;
    while (leftover-- > 0)
    {
      kana_ptr = gw_util_roma_char_to_hira (input_ptr, kana_ptr);
      if (kana_ptr == NULL || input_ptr == NULL)
        break;
      input_ptr = gw_util_next_hira_char_from_roma (input_ptr);
      if (kana_ptr == NULL || input_ptr == NULL)
        break;

      kana_ptr = &kana_ptr[strlen(kana_ptr)];
    }

    return (input_ptr != NULL && strlen (input_ptr) == 0);
}


//!
//! @brief Checks for a Japanese ctype localization setting
//!
//! @return Returns true if it is a japanese ctype
//!
gboolean gw_util_is_japanese_ctype ()
{
    return (setlocale(LC_CTYPE, NULL) != NULL &&
            (
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP.UTF8")  == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP.UTF-8") == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP.utf8")  == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP.utf-8") == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP")       == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja")          == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "japanese")    == 0
            )
           );
}


//!
//! @brief Checks for a Japanese local messages setting
//!
//! Basically this checks if the interface is supposed to come out to be
//! Japanese in the program or not.
//!
//! @return Returns true if it is a japanese local
//!
gboolean gw_util_is_japanese_locale()
{
    return ( setlocale(LC_MESSAGES, NULL) != NULL &&
             (
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP.UTF8")  == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP.UTF-8") == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP.utf8")  == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP.utf-8") == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP")       == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja")          == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "japanese")    == 0
             )
           );
}


//!
//! @brief Attempting to force a japanese CTYPE for better regex handling
//!
//! @return Returns whether the attempt was successful or not
//!
gboolean gw_util_force_japanese_locale ()
{
    //Try forcing a correct or better setting
    //This will have nice effects like antialiased text
    if ( setlocale(LC_CTYPE, "ja_JP.UTF8")  == NULL &&
         setlocale(LC_CTYPE, "ja_JP.utf8")  == NULL &&
         setlocale(LC_CTYPE, "ja_JP.UTF-8") == NULL &&
         setlocale(LC_CTYPE, "ja_JP.utf-8") == NULL &&
         setlocale(LC_CTYPE, "ja_JP")       == NULL &&
         setlocale(LC_CTYPE, "ja")          == NULL &&
         setlocale(LC_CTYPE, "japanese")    == NULL    )
      //All failed, go for the default
      setlocale(LC_CTYPE, "");

    //printf("Set CTYPE locale to %s\n", setlocale(LC_CTYPE, NULL));

    if ( setlocale(LC_COLLATE, "ja_JP.UTF8")  == NULL &&
         setlocale(LC_COLLATE, "ja_JP.utf8")  == NULL &&
         setlocale(LC_COLLATE, "ja_JP.UTF-8") == NULL &&
         setlocale(LC_COLLATE, "ja_JP.utf-8") == NULL &&
         setlocale(LC_COLLATE, "ja_JP")       == NULL &&
         setlocale(LC_COLLATE, "ja")          == NULL &&
         setlocale(LC_COLLATE, "japanese")    == NULL    )
      //All failed, go for the default
      setlocale(LC_COLLATE, "");
}



//!
//! @brief A function that gets the default pref hardwired in.
//!
//! This function is made to take things like gconf out of the equation
//! so that the program can run even if it doesn't have any preferences
//! stored anywhere.
//!
//! @param value The pre-allocated string to copy the value to
//! @param key the key of the wanted preference
//! @param n the radix of the size of the value array
//!
void gw_util_strncpy_fallback_from_key (char *value, const char *key, int n)
{
    if (strcmp (GCKEY_GW_ENGLISH_SOURCE, key) == 0)
      strncpy (value, GW_ENGLISH_URI_FALLBACK, n);
    else if (strcmp (GCKEY_GW_KANJI_SOURCE, key) == 0)
      strncpy (value, GW_KANJI_URI_FALLBACK, n);
    else if (strcmp (GCKEY_GW_NAMES_SOURCE, key) == 0)
      strncpy (value, GW_NAMES_URI_FALLBACK, n);
    else if (strcmp (GCKEY_GW_RADICALS_SOURCE, key) == 0)
      strncpy (value, GW_RADICALS_URI_FALLBACK, n);
    else if (strcmp (GCKEY_GW_EXAMPLES_SOURCE, key) == 0)
      strncpy (value, GW_EXAMPLES_URI_FALLBACK, n);

    else if (strcmp (GCKEY_GW_MATCH_FG, key) == 0)
      strncpy (value, GW_MATCH_FG_FALLBACK, n);
    else if (strcmp (GCKEY_GW_MATCH_BG, key) == 0)
      strncpy (value, GW_MATCH_BG_FALLBACK, n);
    else if (strcmp (GCKEY_GW_HEADER_FG, key) == 0)
      strncpy (value, GW_HEADER_FG_FALLBACK, n);
    else if (strcmp (GCKEY_GW_HEADER_BG, key) == 0)
      strncpy (value, GW_HEADER_BG_FALLBACK, n);
    else if (strcmp (GCKEY_GW_COMMENT_FG, key) == 0)
      strncpy (value, GW_COMMENT_FG_FALLBACK, n);

    else
      strncpy (value, "", n);
}


//!
//! @brief Creates an allocated string from the arguments passed to the program
//!
//! @param argc Standard main argument
//! @param argv Standard main argument
//!
//! @return Returns an allocated string containing the query text.  Should be freed with g_free ()
//!
char *gw_util_strdup_args_to_query (int argc, char *argv[])
{
      int i = 1;
      char *query_text_data = NULL;
      char *query_temp = NULL;
      //query_text_data = g_strdup_printf ("");
      while (i < argc)
      {
        //First loop
        if (query_text_data == NULL)
        {
          query_text_data = g_strdup_printf("%s", argv[i]);
        }
        //Next loops
        else
        {
          query_temp = g_strdup_printf("%s %s", query_text_data, argv[i]);
          if (query_text_data != NULL)
          {
            g_free (query_text_data);
            query_text_data = NULL;
          }
          query_text_data = query_temp;
          query_temp = NULL;
        }
        i++;
      }
      if (query_temp != NULL)
      {
        g_free (query_temp);
        query_temp = NULL;
      }

      return query_text_data;
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
char* gw_util_sanitize_input(char *text, gboolean strip)
{
  if(text == NULL)
    return NULL;
  
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
    g_strstrip(ntext); // no new allocation, just modifying the string
   
  return ntext;
}

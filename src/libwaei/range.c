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
//!  @file dictionary.c
//!
//!  @brief LwDictionary objects represent a loaded dictionary that the program
//!         can use to carry out searches.  You can uninstall dictionaries
//!         by using the object, but you cannot install them. LwDictInst
//!         objects exist for that purpose.
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/gettext.h>
#include <libwaei/range.h>


LwRange* 
lw_range_new_from_pattern (const gchar* PATTERN)
{
    //Sanity checks
    g_return_val_if_fail (lw_range_pattern_is_valid (PATTERN) != FALSE, NULL);

    //Declarations
    LwRange *range;
    const gchar *ptr;
    const gchar *endptr;
    gint temp;

    //Initializations
    range = g_new0 (LwRange, 1);
    if (range == NULL) return NULL;
    ptr = endptr = PATTERN;

    while ((*endptr < '0' || *endptr > '9') && *endptr != '\0') endptr++;
    if (endptr == PATTERN || *endptr == '\0') return NULL;
    range->identifier = g_strndup (ptr, endptr - PATTERN);
    ptr = endptr;
    while (*endptr != '-' && *endptr != '\0') endptr++;
    range->lower = range->higher = (gint) g_ascii_strtoll (ptr, (gchar**) &endptr, 10);
    if (*endptr == '-') range->higher = (gint) g_ascii_strtoll (endptr + 1, NULL, 10);
    
    if (range->lower > range->higher)
    {
      temp = range->lower;
      range->lower = range->higher;
      range->higher = temp;
      temp = 0;
    }

    return range;
}

void
lw_range_free (LwRange *range)
{
    if (range == NULL) return;
    
    g_free (range);
}


gboolean
lw_range_pattern_is_valid (const gchar *TEXT)
{
    //Sanity checks
    g_return_val_if_fail (TEXT != NULL, FALSE);

    return g_regex_match_simple ("^[a-zA-Z][0-9]+(-[0-9]+|)$", TEXT, 0, 0);
}


gboolean
lw_range_string_is_in_range (LwRange *range, const gchar *NUMBER)
{
    //Sanity checks
    g_return_val_if_fail (range != NULL, FALSE);
    g_return_val_if_fail (NUMBER != NULL, FALSE);

    //Declarations
    gint number;

    //Initializations
    number = (gint) g_ascii_strtoll (NUMBER, NULL, 10);

    return lw_range_int_is_in_range (range, number);
}


gboolean
lw_range_int_is_in_range (LwRange *range, gint number)
{
    //Sanity checks
    g_return_val_if_fail (range != NULL, FALSE);

    return (number >= range->lower && number <= range->higher);
}


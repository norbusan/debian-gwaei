#ifndef LW_DICTINFOLIST_INCLUDED
#define LW_DICTINFOLIST_INCLUDED
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
//! @file src/include/libwaei/dictlist.h
//!
//! @brief To be written.
//!
//! To be written.
//!

#include <libwaei/dictinfo.h>

#define LW_DICTINFOLIST(object) (LwDictInfoList*) object

#define EXTENDS_LW_DICTINFOLIST \
  GList *list; \
  GMutex *mutex; \
  int max;

//!
//! @brief Primitive for storing lists of dictionaries
//!
struct _LwDictInfoList
{
  EXTENDS_LW_DICTINFOLIST
};
typedef struct _LwDictInfoList LwDictInfoList;


LwDictInfoList* lw_dictinfolist_new (const int, LwPreferences*);
void lw_dictinfolist_free (LwDictInfoList*);
void lw_dictinfolist_init (LwDictInfoList*, const int, LwPreferences*);
void lw_dictinfolist_deinit (LwDictInfoList*);

void lw_dictinfolist_add_dictionary (LwDictInfoList*, const LwDictType, const char*);

LwDictInfo* lw_dictinfolist_get_dictinfo (LwDictInfoList*, const LwDictType, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_by_filename (LwDictInfoList*, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_by_idstring (LwDictInfoList*, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_fuzzy (LwDictInfoList*, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_by_load_position (LwDictInfoList*, int);
gboolean lw_dictinfolist_check_if_loaded (LwDictInfoList*, const LwDictType, const char*);
void lw_dictinfolist_update_load_orders (LwDictInfoList*);
int lw_dictinfolist_get_total (LwDictInfoList*);
void lw_dictinfolist_clear (LwDictInfoList*);

void lw_dictinfolist_preform_postprocessing_by_name (LwDictInfoList*, char*, GError**);
void lw_dictinfolist_load_dictionary_order_from_pref (LwDictInfoList*, LwPreferences*);
void lw_dictinfolist_save_dictionary_order_pref (LwDictInfoList*, LwPreferences*);

void lw_dictinfolist_reload (LwDictInfoList*);




#endif

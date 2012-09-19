#ifndef GW_DICTLIST_OBJECT_INCLUDED
#define GW_DICTLIST_OBJECT_INCLUDED
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

#define GW_DICTLIST_MAX_DICTIONARIES 20

//!
//! @brief Primitive for storing lists of dictionaries
//!
struct _LwDictList
{
    GList *list;      //!< GList of the installed dictionaries
    GList *selected;  //!< Pointer to the currently selected dictionary in the GList
    GMutex *mutex;
};
typedef struct _LwDictList LwDictList;


GList* lw_dictinfolist_get_list (void);
GList* lw_dictinfolist_get_selected (void);
LwDictInfo* lw_dictinfolist_get_selected_dictinfo (void);


void lw_dictinfolist_initialize (void);
void lw_dictinfolist_free ();

LwDictInfo* lw_dictinfolist_get_dictinfo (const LwEngine, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_by_filename (const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_by_idstring (const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_fuzzy (const char*);
GList* lw_dictinfolist_get_dict_by_load_position (int);
GList* lw_dictinfolist_set_selected_by_load_position (int);
gboolean lw_dictinfolist_check_if_loaded (const LwEngine, const char*);
void lw_dictinfolist_update_load_orders (void);
int lw_dictinfolist_get_total (void);

void lw_dictinfolist_preform_postprocessing_by_name (char*, GError**);
void lw_dictinfolist_load_dictionary_order_from_pref (void);
void lw_dictinfolist_save_dictionary_order_pref (void);




#endif

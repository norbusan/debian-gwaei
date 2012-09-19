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
//! @file src/include/gwaei/dictlist-object.h
//!
//! @brief To be written.
//!
//! To be written.
//!

#include <gwaei/dictinfo-object.h>

//!
//! @brief Primitive for storing lists of dictionaries
//!
struct _GwDictList
{
    GList *list;      //!< GList of the installed dictionaries
    GList *selected;  //!< Pointer to the currently selected dictionary in the GList
};
typedef struct _GwDictList GwDictList;


GList* gw_dictlist_get_list (void);
GwDictInfo* gw_dictlist_get_dictinfo_by_id (GwDictId);
int gw_dictlist_get_total (void);
GList* gw_dictlist_get_selected (void);


void gw_dictionaries_initialize_dictionary_list (void);
GwDictInfo* gw_dictlist_get_dictinfo_by_name (const char*);
GwDictInfo* gw_dictlist_get_dictinfo_by_alias (const char*);
GList* gw_dictlist_get_dict_by_load_position (int);
GList* gw_dictlist_set_selected_by_load_position (int);
GwDictStatus gw_dictlist_dictionary_get_status_by_id (GwDictId);
gboolean gw_dictlist_check_if_loaded_by_name (char*);

void gw_dictlist_preform_postprocessing_by_name (char*, GError**);
int gw_dictlist_get_total_with_status (GwDictStatus);

void gw_dictlist_free ();



#endif

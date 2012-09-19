#ifndef GW_HISTORYLIST_HEADER_INCLUDED
#define GW_HISTORYLIST_HEADER_INCLUDED 
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
//! @file src/include/libwaei/historylist.h
//!
//! @brief To be written.
//!
//! To be written.
//!

//!
//! Historylist targets
//!

#include <libwaei/searchitem.h>

typedef enum {
  GW_HISTORYLIST_RESULTS,
  GW_HISTORYLIST_KANJI
} LwHistoryListTarget;


//!
//! @brief Primitive for storing search items in intelligent ways
//!
struct _LwHistoryList {
    GList *back;           //!< A GList of past search items
    GList *forward;        //!< A GList where past search items get stacked when the user goes back.
    LwSearchItem *current; //!< The current search before it gets pushed only into a history list.
};
typedef struct _LwHistoryList LwHistoryList;

void lw_historylist_initialize (void);
void lw_historylist_free (void);

//Methods
LwHistoryList* lw_historylist_get_list(const int);
LwSearchItem* lw_historylist_get_current (const int);
GList* lw_historylist_get_back_history (const int);
GList* lw_historylist_get_forward_history (const int);
GList* lw_historylist_get_combined_history_list (const int);
void lw_historylist_add_searchitem_to_history (const int, LwSearchItem*);
void lw_historylist_go_back_by_target (const int);
void lw_historylist_go_forward_by_target (const int);


#endif

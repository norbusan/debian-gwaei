#ifndef LW_HISTORY_INCLUDED
#define LW_HISTORY_INCLUDED 
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
//! @file src/include/libwaei/history.h
//!
//! @brief To be written.
//!
//! To be written.
//!

//!
//! Historylist targets
//!

#define LW_HISTORYLIST(object) (LwHistory*) object
#include <libwaei/searchitem.h>


//!
//! @brief Primitive for storing search items in intelligent ways
//!
struct _LwHistory {
    GList *back;           //!< A GList of past search items
    GList *forward;        //!< A GList where past search items get stacked when the user goes back.
    int max;
};
typedef struct _LwHistory LwHistory;

LwHistory* lw_history_new (const int);
void lw_history_free (LwHistory*);
void lw_history_init (LwHistory*, const int);
void lw_history_deinit (LwHistory*);

//Methods
GList* lw_history_get_back_list (LwHistory*);
GList* lw_history_get_forward_list (LwHistory*);
GList* lw_history_get_combined_list (LwHistory*);
void lw_history_clear_forward_list (LwHistory*);
void lw_history_clear_back_list (LwHistory*);

void lw_history_add_searchitem (LwHistory*, LwSearchItem*);

gboolean lw_history_has_back (LwHistory*);
gboolean lw_history_has_forward (LwHistory*);
LwSearchItem* lw_history_go_back (LwHistory*, LwSearchItem*);
LwSearchItem* lw_history_go_forward (LwHistory*, LwSearchItem*);


#endif

#ifndef LW_SEARCHITEM_INCLUDED
#define LW_SEARCHITEM_INCLUDED
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
//! @file searchitem.h
//!
//! @brief To be written.
//!
//! To be written.
//!

#include <stdio.h>

#include <libwaei/queryline.h>
#include <libwaei/resultline.h>
#include <libwaei/dictinfo.h>


#define LW_SEARCHITEM(object) (LwSearchItem*) object
#define LW_SEARCHITEM_DATA_FREE_FUNC(object) (LwSearchItemDataFreeFunc)object
#define LW_HISTORY_TIME_TO_RELEVANCE 20

//!
//! @brief Search status types
//!
typedef enum
{
  LW_SEARCHSTATUS_IDLE,
  LW_SEARCHSTATUS_SEARCHING,
  LW_SEARCHSTATUS_CANCELING
} LwSearchStatus;

typedef void(*LwSearchItemDataFreeFunc)(gpointer);

//!
//! @brief Primitive for storing search item information
//!
//Object
struct _LwSearchItem {
    LwQueryLine* queryline;                 //!< Result line to store parsed result
    LwDictInfo* dictionary;                 //!< Pointer to the dictionary used

    FILE* fd;                               //!< File descriptor for file search position
    GThread *thread;                        //!< Thread the search is processed in
    GMutex *mutex;                          //!< Mutext to help ensure threadsafe operation

    LwSearchStatus status;                  //!< Used to test if a search is in progress.
    char *scratch_buffer;                   //!< Scratch space
    long current;                           //!< Current line in the dictionary file
    int history_relevance_idle_timer;       //!< Helps determine if something is added to the history or not

    int total_relevant_results;             //!< Total results guessed to be highly relevant to the query
    int total_irrelevant_results;           //!< Total results guessed to be vaguely relevant to the query
    int total_results;                      //!< Total results returned from the search

    GList *results_high;                    //!< Buffer storing mediumly relevant result for later display
    GList *results_medium;                  //!< Buffer storing mediumly relevant result for later display
    GList *results_low;                     //!< Buffer storing lowly relevant result for later display

    LwResultLine* resultline;               //!< Result line to store parsed result

    gpointer data;                 //!< Pointer to a buffer that stays constant unlike when the target attribute is used
    LwSearchItemDataFreeFunc free_data_func;
};
typedef struct _LwSearchItem LwSearchItem;

//Methods
LwSearchItem* lw_searchitem_new (const char*, LwDictInfo*, LwPreferences*, GError**);
void lw_searchitem_free (LwSearchItem*);
void lw_searchitem_init (LwSearchItem*, const char*, LwDictInfo*, LwPreferences*, GError**);
void lw_searchitem_deinit (LwSearchItem*);

void lw_searchitem_cleanup_search (LwSearchItem*);
void lw_searchitem_clear_results (LwSearchItem*);
void lw_searchitem_prepare_search (LwSearchItem*);

gboolean lw_searchitem_run_comparison (LwSearchItem*, const LwRelevance);
gboolean lw_searchitem_is_equal (LwSearchItem*, LwSearchItem*);
gboolean lw_searchitem_has_history_relevance (LwSearchItem*, gboolean);
void lw_searchitem_increment_history_relevance_timer (LwSearchItem*);

void lw_searchitem_set_data (LwSearchItem*, gpointer, LwSearchItemDataFreeFunc);
gpointer lw_searchitem_get_data (LwSearchItem*);
void lw_searchitem_free_data (LwSearchItem*);
gboolean lw_searchitem_has_data (LwSearchItem*);

gboolean lw_searchitem_should_check_results (LwSearchItem*);
LwResultLine* lw_searchitem_get_result (LwSearchItem*);
void lw_searchitem_parse_result_string (LwSearchItem*);
void lw_searchitem_cancel_search (LwSearchItem*);

void lw_searchitem_lock_mutex (LwSearchItem*);
void lw_searchitem_unlock_mutex (LwSearchItem*);

double lw_searchitem_get_progress (LwSearchItem*);



#endif

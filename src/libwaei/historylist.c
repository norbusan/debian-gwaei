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
//! @file src/historylist-object.c
//!
//! @brief Search item and history management
//!
//! Functions and objects to create search items and manage them.
//!


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>


static LwHistoryList *results_history;
static LwHistoryList *kanji_history;


//!
//! @brief Returns the private historylist of the history.c file
//!
//! The two options here are GW_HISTORYLIST_RESULTS to get the results
//! history list and GW_HISTORYLIST_KANJI to get the kanji history list.
//!
//! @param TARGET The target who's history list we want.
//!
LwHistoryList* lw_historylist_get_list (const int TARGET)
{
    if (TARGET == GW_HISTORYLIST_RESULTS)
      return results_history;
    else if (TARGET == GW_HISTORYLIST_KANJI)
      return kanji_history;
    else
      return NULL;
}


//!
//! @brief Creates a new LwHistoryList object. 
//!
//! Creates a new history list object which null pointers
//! a a back, forward history list and a current LwSearchItem.
//!
//! @return Returns the allocated LwHistoryList object.
//!
LwHistoryList* lw_historylist_new()
{
    LwHistoryList *temp;
    if ((temp = malloc(sizeof(LwHistoryList))) != NULL)
    {
      temp->back = NULL;
      temp->forward = NULL;
      temp->current = NULL;
    }

    return temp;
}


//!
//! @brief Clears the forward history of the desired target.
//!
void lw_historylist_clear_forward_history (const int TARGET)
{
    //Declarations
    LwHistoryList *hl;
    LwSearchItem *item;
    GList *iter;

    //Initializations
    hl = lw_historylist_get_list (TARGET);

    //Free the data of the list
    for (iter = hl->forward; iter != NULL; iter = iter->next)
    {
      item = (LwSearchItem*) iter->data;
      lw_searchitem_free(item);
      iter->data = NULL;
    }

    //Free the list itself
    g_list_free (hl->forward);
    hl->forward = NULL;
}


//!
//! @brief Clears the back history of the desired target.
//!
void lw_historylist_clear_back_history (const int TARGET)
{
    //Declarations
    LwHistoryList *hl;
    LwSearchItem *item;
    GList *iter;

    //Initializations
    hl = lw_historylist_get_list (TARGET);

    //Free the data of the list
    for (iter = hl->back; iter != NULL; iter = iter->next)
    {
      item = (LwSearchItem*) iter->data;
      lw_searchitem_free(item);
      iter->data = NULL;
    }

    //Free the list itself
    g_list_free (hl->back);
    hl->back = NULL;
}




//!
//! @brief Gets the back history of the target history list
//!
//! @see lw_historylist_get_forward_history ()
//! @see lw_historylist_get_current ()
//! @return Returns a GList containing the LwSearchItem back history
//!
GList* lw_historylist_get_back_history (const int TARGET)
{
    LwHistoryList *hl = lw_historylist_get_list (TARGET);
    return hl->back;
}


//!
//! @brief Gets the forward history of the target history list
//!
//! @see lw_historylist_get_back_history ()
//! @see lw_historylist_get_current ()
//! @return Returns a GList containing the LwSearchItem forward history
//!
GList* lw_historylist_get_forward_history (const int TARGET)
{
    LwHistoryList *list = lw_historylist_get_list (TARGET);
    return list->forward;
}


//!
//! @brief Gets the current search item of the user
//!
//! This is the search item of the current search.  It doesn't get lumped
//! into a history list until the user hits the back button or does another
//! search. At program start, it has the special value of null which causes
//! many GUI elements to become disabled.
//!
//! @see lw_historylist_get_back_history ()
//! @see lw_historylist_get_forward_history ()
//! @return Returns a GList containing the LwSearchItem forward history
//!
LwSearchItem* lw_historylist_get_current (const int TARGET)
{
    LwHistoryList *list = lw_historylist_get_list (TARGET);
    return list->current;
}


//!
//! @brief Concatinates together a copy of the back and forward histories
//!
//! This function was made with the idea of easily preparing a history list
//! for a history menu which doesn't care about separating each list.
//!
//! @see lw_historylist_get_back_history ()
//! @see lw_historylist_get_forward_history ()
//! @return Returns an allocated GList containing the back and forward history
//!
GList* lw_historylist_get_combined_history_list (const int TARGET)
{
    LwHistoryList *hl = lw_historylist_get_list (TARGET);
    GList *back_copy = g_list_copy (hl->back);

    GList *out = NULL;
    out = g_list_copy (hl->forward);
    out = g_list_reverse (out);
    out = g_list_concat (out, back_copy);

    return out;
}


//!
//! @brief Moves an item to the back history
//!
//! The current variable has its LwSearchItem moved into the backhistory list.  The
//! forward history is also cleared at this time.
//!
void lw_historylist_add_searchitem_to_history (const int TARGET, LwSearchItem *item)
{ 
    LwHistoryList *hl = lw_historylist_get_list (TARGET);
    lw_historylist_clear_forward_history(TARGET);

    if (g_list_length(hl->back) >= 20)
    {
      GList* last = g_list_last (hl->back); 
      lw_searchitem_free(last->data);
      hl->back = g_list_delete_link(hl->back, last);
    }
    hl->back = g_list_prepend(hl->back, item);
}


//!
//! @brief Flopps the history stack 1 item in the desired direction
//!
//! Data is shifted between the forward, current, and back variables. If current is
//! null, the list just fills it in rather than shifting the data around.
//!
static void _shift_history_by_target (const int TARGET, GList **from, GList **to)
{
    LwHistoryList *hl = lw_historylist_get_list (TARGET);
    LwSearchItem **current = &(hl->current);

    //Handle the current searchitem if it exists
    if (*current != NULL)
    {
      if ((*current)->total_results)
        *to = g_list_prepend (*to, *current);
      else
        lw_searchitem_free (*current);
      *current = NULL;
    }

    //Shift the top back searchitem to current (which is now empty)
    GList *item = *from;
    *from = g_list_remove_link (*from, item);
    *current = item->data;

    if (g_list_length (*from) == 0)
      *from = NULL;
}


//!
//! @brief Go back 1 in history
//!
//! @param TARGET the target that should have it's history list adjusted
//!
void lw_historylist_go_back_by_target (const int TARGET)
{ 
    LwHistoryList *hl = lw_historylist_get_list (TARGET);
    _shift_history_by_target (TARGET, &(hl->back), &(hl->forward));
}


//!
//! @brief Go formward 1 in history
//!
//! @param TARGET the target that should have it's history list adjusted
//!
void lw_historylist_go_forward_by_target (const int TARGET)
{ 
    LwHistoryList *hl = lw_historylist_get_list (TARGET);
    _shift_history_by_target (TARGET, &(hl->forward), &(hl->back));
}


//!
//! @brief Prepare the historylists for the desired widgets
//!
//! Currently there is the results history list and the mostly unused
//! kanji history list for the sidebar.
//!
void lw_historylist_initialize ()
{
    results_history = lw_historylist_new();
    kanji_history   = lw_historylist_new();
}

void lw_historylist_free ()
{
    //Free the results history list
    lw_historylist_clear_forward_history (GW_HISTORYLIST_RESULTS);
    lw_historylist_clear_back_history (GW_HISTORYLIST_RESULTS);
    lw_searchitem_free (results_history->current);
    results_history->current = NULL;
    free (results_history);
    results_history = NULL;

    //Free the kanji history list
    lw_historylist_clear_forward_history (GW_HISTORYLIST_KANJI);
    lw_historylist_clear_back_history (GW_HISTORYLIST_KANJI);
    lw_searchitem_free (kanji_history->current);
    kanji_history->current = NULL;
    free (kanji_history);
    kanji_history = NULL;
}



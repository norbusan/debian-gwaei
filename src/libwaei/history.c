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
//! @file history.c
//!


#include "../private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>


//!
//! @brief Creates a new LwHistory object
//! @param MAX The maximum items you want in the list before old ones are deleted
//! @return An allocated LwHistory that will be needed to be freed by lw_history_free.
//!
LwHistory* 
lw_history_new (const int MAX)
{
    LwHistory *temp;

    temp = (LwHistory*) malloc(sizeof(LwHistory));

    if (temp != NULL)
    {
      lw_history_init (temp, MAX);
    }

    return temp;
}


//!
//! @brief Releases a LwHistory object from memory.
//! @param list A LwHistory object created by lw_history_new.
//!
void 
lw_history_free (LwHistory *list)
{
    lw_history_deinit (list);
    free (list);
}


//!
//! @brief Used to initialize the memory inside of a new LwHistory
//!        object.  Usually lw_history_new calls this for you.  It is also 
//!        used in class implimentations that extends LwHistory.
//! @param list The LwHistory object to initialize the memory of.
//! @param MAX The maximum items you want in the list before old ones are deleted
//!
void 
lw_history_init (LwHistory *list, const int MAX)
{
    list->back = NULL;
    list->forward = NULL;
    list->max = MAX;
}


//!
//! @brief Used to free the memory inside of a LwHistory object.
//!         Usually lw_history_free calls this for you.  It is also used
//!         in class implimentations that extends LwHistory.
//! @param list The LwHistory object to have it's inner memory freed.
//!
void 
lw_history_deinit (LwHistory *list)
{
    lw_history_clear_forward_list (list);
    lw_history_clear_back_list (list);
}


//!
//! @brief Clears the forward history of the desired target.
//!
void 
lw_history_clear_forward_list (LwHistory *list)
{
    //Declarations
    LwSearchItem *item;
    GList *iter;

    //Free the data of the list
    for (iter = list->forward; iter != NULL; iter = iter->next)
    {
      item = (LwSearchItem*) iter->data;
      if (item != NULL)
        lw_searchitem_free (item);
      iter->data = NULL;
    }

    //Free the list itself
    g_list_free (list->forward);
    list->forward = NULL;
}


//!
//! @brief Clears the back history of the desired target.
//!
void 
lw_history_clear_back_list (LwHistory *list)
{
    //Declarations
    LwSearchItem *item;
    GList *iter;

    //Free the data of the list
    for (iter = list->back; iter != NULL; iter = iter->next)
    {
      item = (LwSearchItem*) iter->data;
      if (item != NULL)
        lw_searchitem_free (item);
      iter->data = NULL;
    }

    //Free the list itself
    g_list_free (list->back);
    list->back = NULL;
}


//!
//! @brief Gets the back history of the target history list
//! @return Returns a GList containing the LwSearchItem back history
//!
GList* 
lw_history_get_back_list (LwHistory *list)
{
    return list->back;
}


//!
//! @brief Gets the forward history of the target history list
//! @return Returns a GList containing the LwSearchItem forward history
//!
GList* 
lw_history_get_forward_list (LwHistory *list)
{
    return list->forward;
}


//!
//! @brief Concatinates together a copy of the back and forward histories
//!
//! This function was made with the idea of easily preparing a history list
//! for a history menu which doesn't care about separating each list.
//!
//! @see lw_history_get_back_list ()
//! @see lw_history_get_forward_list ()
//! @return Returns an allocated GList containing the back and forward history
//!
GList* 
lw_history_get_combined_list (LwHistory *list)
{
    //Declarations
    GList *combined;
    
    //Initializations
    combined = NULL;
    combined = g_list_copy (list->forward);
    combined = g_list_reverse (combined);
    combined = g_list_concat (combined, g_list_copy (list->back));

    return combined;
}


//!
//! @brief Moves an item to the back history
//!
void 
lw_history_add_searchitem (LwHistory *list, LwSearchItem *item)
{ 
    //Declarations
    GList *link;

    //Clear the forward history
    lw_history_clear_forward_list (list);

    list->back = g_list_prepend (list->back, item);

    //Make sure the list hasn't gotten too long
    if (g_list_length (list->back) >= list->max)
    {
      link = g_list_last (list->back); 
      lw_searchitem_free (LW_SEARCHITEM (link->data));
      list->back = g_list_delete_link (list->back, link);
    }
}


//!
//! @brief Returns true if it is possible to go forward on a history list
//!
gboolean 
lw_history_has_forward (LwHistory *list)
{
    return (list->forward != NULL);
}


//!
//! @brief Returns true if it is possible to go back on a history list
//!
gboolean 
lw_history_has_back (LwHistory *list)
{
    return (list->back != NULL);
}


//!
//! @brief Go back 1 in history
//!
LwSearchItem* 
lw_history_go_back (LwHistory *list, LwSearchItem *pushed)
{ 
    //Sanity check
    if (!lw_history_has_back (list)) return pushed;

    //Declarations
    GList *link;
    LwSearchItem *popped;

    if (pushed != NULL)
    {
      list->forward = g_list_append (list->forward, pushed);
    }

    link = g_list_last (list->back); 
    popped = LW_SEARCHITEM (link->data);
    list->back = g_list_delete_link (list->back, link);

    return popped;
}


//!
//! @brief Go forward 1 in history
//!
LwSearchItem* 
lw_history_go_forward (LwHistory *list, LwSearchItem *pushed)
{ 
    //Sanity check
    if (!lw_history_has_forward (list)) return pushed;

    //Declarations
    GList *link;
    LwSearchItem *popped;

    if (pushed != NULL)
    {
      list->back = g_list_append (list->back, pushed);
    }

    link = g_list_last (list->forward); 
    popped = LW_SEARCHITEM (link->data);
    list->forward = g_list_delete_link (list->forward, link);

    return popped;
}


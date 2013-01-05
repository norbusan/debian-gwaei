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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>

#include <libwaei/history-private.h>
#include <libwaei/gettext.h>

G_DEFINE_TYPE (LwHistory, lw_history, G_TYPE_OBJECT)

typedef enum
{
  PROP_0,
  PROP_MAX_SIZE
} LwHistoryProps;

//!
//! @brief Creates a new LwHistory object
//! @param MAX The maximum items you want in the history before old ones are deleted
//! @return An allocated LwHistory that will be needed to be freed by lw_history_free.
//!
LwHistory* 
lw_history_new (gint max_size)
{
    LwHistory *history;

    //Initializations
    history = LW_HISTORY (g_object_new (LW_TYPE_HISTORY,
                                       "max-size", max_size,
                                       NULL));
    return history;
}


static void 
lw_history_init (LwHistory *history)
{
    history->priv = LW_HISTORY_GET_PRIVATE (history);
    memset(history->priv, 0, sizeof(LwHistoryPrivate));

    history->priv->time_delta = 20;
}


static void 
lw_history_finalize (GObject *object)
{
    //Declarations
    LwHistory *history;

    //Initalizations
    history = LW_HISTORY (object);

    lw_history_clear_forward_list (history);
    lw_history_clear_back_list (history);

    G_OBJECT_CLASS (lw_history_parent_class)->finalize (object);
}


static void 
lw_history_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    //Declarations
    LwHistory *history;
    LwHistoryPrivate *priv;

    //Initializations
    history = LW_HISTORY (object);
    priv = history->priv;

    switch (property_id)
    {
      case PROP_MAX_SIZE:
        priv->max = g_value_get_int (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void 
lw_history_get_property (GObject      *object,
                         guint         property_id,
                         GValue       *value,
                         GParamSpec   *pspec)
{
    //Declarations
    LwHistory *history;
    LwHistoryPrivate *priv;

    //Initializations
    history = LW_HISTORY (object);
    priv = history->priv;

    switch (property_id)
    {
      case PROP_MAX_SIZE:
        g_value_set_int (value, priv->max);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void
lw_history_class_init (LwHistoryClass *klass)
{
    //Declarations
    GParamSpec *pspec;
    GObjectClass *object_class;
    LwHistoryClass *history_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = lw_history_set_property;
    object_class->get_property = lw_history_get_property;
    object_class->finalize = lw_history_finalize;

    history_class = LW_HISTORY_CLASS (klass);

    history_class->signalid[LW_HISTORY_CLASS_SIGNALID_CHANGED] = g_signal_new (
        "changed",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
        G_STRUCT_OFFSET (LwHistoryClass, changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );

    history_class->signalid[LW_HISTORY_CLASS_SIGNALID_BACK] = g_signal_new (
        "back",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
        G_STRUCT_OFFSET (LwHistoryClass, back),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );

    history_class->signalid[LW_HISTORY_CLASS_SIGNALID_FORWARD] = g_signal_new (
        "forward",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
        G_STRUCT_OFFSET (LwHistoryClass, forward),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );

    history_class->signalid[LW_HISTORY_CLASS_SIGNALID_ADDED] = g_signal_new (
        "added",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
        G_STRUCT_OFFSET (LwHistoryClass, added),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );

    g_type_class_add_private (object_class, sizeof (LwHistoryPrivate));

    pspec = g_param_spec_int ("max-size",
                              "Max length of the back history.",
                              "Set the maximum length of the back history",
                              -1,
                              10000,
                              -1,
                              G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_MAX_SIZE, pspec);
}


//!
//! @brief Clears the forward history of the desired target.
//!
void 
lw_history_clear_forward_list (LwHistory *history)
{
    //Declarations
    LwHistoryPrivate *priv;
    LwHistoryClass *klass;
    LwSearch *search;
    GList *iter;

    //Initializations
    priv = history->priv;
    klass = LW_HISTORY_CLASS (G_OBJECT_GET_CLASS (history));

    //Free the data of the history
    for (iter = priv->forward; iter != NULL; iter = iter->next)
    {
      search = (LwSearch*) iter->data;
      if (search != NULL)
        lw_search_free (search);
      iter->data = NULL;
    }

    //Free the history itself
    g_list_free (priv->forward);
    priv->forward = NULL;

    g_signal_emit (history,
      klass->signalid[LW_HISTORY_CLASS_SIGNALID_CHANGED],
      0
    );
}


//!
//! @brief Clears the back history of the desired target.
//!
void 
lw_history_clear_back_list (LwHistory *history)
{
    //Declarations
    LwHistoryPrivate *priv;
    LwHistoryClass *klass;
    LwSearch *search;
    GList *iter;

    //Initializations
    priv = history->priv;
    klass = LW_HISTORY_CLASS (G_OBJECT_GET_CLASS (history));

    //Free the data of the history
    for (iter = priv->back; iter != NULL; iter = iter->next)
    {
      search = (LwSearch*) iter->data;
      if (search != NULL)
        lw_search_free (search);
      iter->data = NULL;
    }

    //Free the history itself
    g_list_free (priv->back);
    priv->back = NULL;

    g_signal_emit (history,
      klass->signalid[LW_HISTORY_CLASS_SIGNALID_CHANGED],
      0
    );
}


//!
//! @brief Gets the back history of the target history history
//! @return Returns a GList containing the LwSearch back history
//!
GList* 
lw_history_get_back_list (LwHistory *history)
{
    return history->priv->back;
}


//!
//! @brief Gets the forward history of the target history history
//! @return Returns a GList containing the LwSearch forward history
//!
GList* 
lw_history_get_forward_list (LwHistory *history)
{
    return history->priv->forward;
}


//!
//! @brief Concatinates together a copy of the back and forward histories
//!
//! This function was made with the idea of easily preparing a history history
//! for a history menu which doesn't care about separating each history.
//!
//! @see lw_history_get_back_list ()
//! @see lw_history_get_forward_list ()
//! @return Returns an allocated GList containing the back and forward history
//!
GList* 
lw_history_get_combined_list (LwHistory *history)
{
    //Declarations
    LwHistoryPrivate *priv;
    GList *combined;
    
    //Initializations
    priv = history->priv; 
    combined = NULL;
    combined = g_list_copy (priv->forward);
    combined = g_list_reverse (combined);
    combined = g_list_concat (combined, g_list_copy (priv->back));

    return combined;
}


//!
//! @brief Moves an search to the back history
//!
void 
lw_history_add_search (LwHistory *history, LwSearch *search)
{ 
    //Declarations
    LwHistoryPrivate *priv;
    LwHistoryClass *klass;
    GList *link;

    //Initalizations
    priv = history->priv;
    klass = LW_HISTORY_CLASS (G_OBJECT_GET_CLASS (history));
    
    priv->back = g_list_prepend (priv->back, search);

    //Make sure the history hasn't gotten too long
    if (g_list_length (priv->back) >= priv->max)
    {
      link = g_list_last (priv->back); 
      lw_search_free (LW_SEARCH (link->data));
      priv->back = g_list_delete_link (priv->back, link);
    }

    //Clear the forward history
    lw_history_clear_forward_list (history);

    g_signal_emit (history,
      klass->signalid[LW_HISTORY_CLASS_SIGNALID_ADDED],
      0
    );
}


//!
//! @brief Returns true if it is possible to go forward on a history history
//!
gboolean 
lw_history_has_forward (LwHistory *history)
{
    return (history->priv->forward != NULL);
}


//!
//! @brief Returns true if it is possible to go back on a history history
//!
gboolean 
lw_history_has_back (LwHistory *history)
{
    return (history->priv->back != NULL);
}


//!
//! @brief Go back 1 in history
//!
LwSearch* 
lw_history_go_back (LwHistory *history, LwSearch *pushed)
{ 
    //Sanity check
    if (!lw_history_has_back (history)) return pushed;

    //Declarations
    LwHistoryPrivate *priv;
    LwHistoryClass *klass;
    GList *link;
    LwSearch *popped;

    priv = history->priv;
    klass = LW_HISTORY_CLASS (G_OBJECT_GET_CLASS (history));

    if (pushed != NULL)
    {
      priv->forward = g_list_append (priv->forward, pushed);
    }

    link = g_list_last (priv->back); 
    popped = LW_SEARCH (link->data);
    priv->back = g_list_delete_link (priv->back, link);

    g_signal_emit (history,
      klass->signalid[LW_HISTORY_CLASS_SIGNALID_BACK],
      0
    );

    g_signal_emit (history,
      klass->signalid[LW_HISTORY_CLASS_SIGNALID_CHANGED],
      0
    );

    return popped;
}


//!
//! @brief Go forward 1 in history
//!
LwSearch* 
lw_history_go_forward (LwHistory *history, LwSearch *pushed)
{ 
    //Sanity check
    if (!lw_history_has_forward (history)) return pushed;

    //Declarations
    LwHistoryPrivate *priv;
    LwHistoryClass *klass;
    GList *link;
    LwSearch *popped;

    priv = history->priv;
    klass = LW_HISTORY_CLASS (G_OBJECT_GET_CLASS (history));

    if (pushed != NULL)
    {
      priv->back = g_list_append (priv->back, pushed);
    }

    link = g_list_last (priv->forward); 
    popped = LW_SEARCH (link->data);
    priv->forward = g_list_delete_link (priv->forward, link);

    g_signal_emit (history,
      klass->signalid[LW_HISTORY_CLASS_SIGNALID_FORWARD],
      0
    );

    g_signal_emit (history,
      klass->signalid[LW_HISTORY_CLASS_SIGNALID_CHANGED],
      0
    );

    return popped;
}



//!
//! @brief Checks if the relevant timer has passed a threshold
//! @param search The LwSearch to check for history relevance
//! @param use_idle_timer This variable shoud be set to true if the program does automatic searches so it checks the timer
//!
gboolean 
lw_history_has_relevance (LwHistory *history, LwSearch *search, gboolean check_timestamp)
{
    //Sanity checks
    if (search == NULL) return FALSE;
    g_return_val_if_fail (history != NULL, FALSE);

    //Declarations
    LwHistoryPrivate *priv;
    gboolean has_results;
    gboolean enough_time_since_last_search;
    gint64 timestamp;
    gint64 delta;

    //Initializations
    priv = history->priv;
    has_results = (lw_search_get_total_results (search) > 0);
    timestamp = g_get_monotonic_time ();
    delta = timestamp - search->timestamp;
    enough_time_since_last_search = (delta > priv->time_delta);

    return (has_results && (!check_timestamp || enough_time_since_last_search));
}



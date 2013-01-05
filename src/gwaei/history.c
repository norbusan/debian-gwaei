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

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>

#include <gwaei/history.h>
#include <gwaei/history-private.h>


G_DEFINE_TYPE (GwHistory, gw_history, LW_TYPE_HISTORY)


//!
//! @brief Creates a new GwHistory object
//! @param MAX The maximum items you want in the history before old ones are deleted
//! @return An allocated GwHistory that will be needed to be freed by gw_history_free.
//!
GwHistory* 
gw_history_new (gint max_size)
{
    GwHistory *history;

    //Initializations
    history = GW_HISTORY (g_object_new (GW_TYPE_HISTORY,
                                       "max-size", max_size,
                                       NULL));
    return history;
}


static void 
gw_history_init (GwHistory *history)
{
    history->priv = GW_HISTORY_GET_PRIVATE (history);
    memset(history->priv, 0, sizeof(GwHistoryPrivate));
}


static void 
gw_history_finalize (GObject *object)
{
/*
    //Declarations
    GwHistory *history;

    //Initalizations
    history = LW_HISTORY (object);
*/

    G_OBJECT_CLASS (gw_history_parent_class)->finalize (object);
}


static void
gw_history_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_history_parent_class)->constructed (object);
    }

    //Declarations
    GwHistory *history;
    GwHistoryPrivate *priv;

    //Initializations
    history = GW_HISTORY (object);
    priv = history->priv;

    priv->forward = G_MENU_MODEL (g_menu_new ());
    priv->back = G_MENU_MODEL (g_menu_new ());
    
    { 
      //Declarations
      GMenu *menu;
      GMenuItem *menuitem;

      //Initializations
      menu = g_menu_new ();

      menuitem = g_menu_item_new (NULL, NULL);
      g_menu_item_set_link (menuitem, G_MENU_LINK_SECTION, priv->forward);
      g_menu_append_item (menu, menuitem);
      g_object_unref (menuitem); menuitem = NULL;

      menuitem = g_menu_item_new (NULL, NULL);
      g_menu_item_set_link (menuitem, G_MENU_LINK_SECTION, priv->back);
      g_menu_append_item (menu, menuitem);
      g_object_unref (menuitem); menuitem = NULL;

      priv->combined = G_MENU_MODEL (menu);
    }

    g_signal_connect (G_OBJECT (history), "changed", G_CALLBACK (gw_history_sync_menumodels), NULL);
}


static void
gw_history_class_init (GwHistoryClass *klass)
{
    //Declarations
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->constructed = gw_history_constructed;
    object_class->finalize = gw_history_finalize;

    g_type_class_add_private (object_class, sizeof (GwHistoryPrivate));
}


GMenuModel*
gw_history_get_back_menumodel (GwHistory *history)
{
    return history->priv->back;
}


GMenuModel*
gw_history_get_forward_menumodel (GwHistory *history)
{
    return history->priv->forward;
}


GMenuModel*
gw_history_get_combined_menumodel (GwHistory *history)
{
    return history->priv->combined;
}


void
gw_history_sync_menumodels (GwHistory *history)
{
    //Fill the back list
    {
      GList *link;
      GMenuModel *menumodel;
      GMenu *menu;
      LwSearch *search;
      const gchar *label;
      gchar *detailed_action;
      gint i;

      menumodel = gw_history_get_back_menumodel (history);
      menu = G_MENU (menumodel);
      link = lw_history_get_back_list (LW_HISTORY (history));
      i = g_menu_model_get_n_items (menumodel);

      while (i--) g_menu_remove (menu, 0);

      i = g_list_length (link);

      while (link != NULL)
      {
        search = LW_SEARCH (link->data);
        label = lw_query_get_text (search->query);
        detailed_action = g_strdup_printf ("win.go-back-index::%d", i);

        g_menu_append (menu, label, detailed_action);

        if (detailed_action != NULL) g_free (detailed_action); detailed_action = NULL;

        link = link->next;
        i--;
      }
    }

    //Fill the forward list
    {
      GList *link;
      GMenuModel *menumodel;
      GMenu *menu;
      LwSearch *search;
      const gchar *label;
      gchar *detailed_action;
      gint i;

      menumodel = gw_history_get_forward_menumodel (history);
      menu = G_MENU (menumodel);
      link = lw_history_get_forward_list (LW_HISTORY (history));
      i = g_menu_model_get_n_items (menumodel);

      while (i--) g_menu_remove (menu, 0);

      i = g_list_length (link);

      while (link != NULL)
      {
        search = LW_SEARCH (link->data);
        label = lw_query_get_text (search->query);
        detailed_action = g_strdup_printf ("win.go-forward-index::%d", i);

        g_menu_append (menu, label, detailed_action);

        if (detailed_action != NULL) g_free (detailed_action); detailed_action = NULL;

        link = link->next;
        i--;
      }
    }

}


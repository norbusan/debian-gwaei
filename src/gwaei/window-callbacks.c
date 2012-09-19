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
//! @file window-callbacks.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <gwaei/gwaei.h>
#include <gwaei/window-private.h>


G_MODULE_EXPORT gboolean 
gw_window_configure_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GwWindow *window;
    GwWindowPrivate *priv;
    GdkEventConfigure *event_configure;

    window = GW_WINDOW (widget);
    priv = window->priv;
    event_configure = (GdkEventConfigure*) event;

    priv->x = event_configure->x;
    priv->y = event_configure->y;
    priv->width = event_configure->width;
    priv->height = event_configure->height;

    return FALSE;
}


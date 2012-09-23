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


G_MODULE_EXPORT gboolean
gw_window_focus_in_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    //Declarations
    GwWindow *window;
    GwApplication *application;
    GMenuModel *menumodel;
    gboolean os_shows_win_menu;
    GtkSettings *settings;
    
    //Initializations
    window = GW_WINDOW (widget);
    application = gw_window_get_application (window);
    settings = gtk_settings_get_default ();
    g_object_get (settings, "gtk-shell-shows-menubar", &os_shows_win_menu, NULL);

    menumodel = gw_window_get_transient_for_menumodel (window);
    if (menumodel == NULL)
      menumodel = G_MENU_MODEL (g_menu_new ());
    if (menumodel == NULL) 
      return FALSE;

    if (os_shows_win_menu)
      gw_application_set_win_menubar (GW_APPLICATION (application), menumodel);

    return FALSE;
}


G_MODULE_EXPORT gboolean
gw_window_delete_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GwApplication *application;
    GwWindow *window;

    window = GW_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_WINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    application = gw_window_get_application (window);

    gtk_widget_destroy (GTK_WIDGET (window));

    if (gw_application_should_quit (application))
      gw_application_quit (application);

    return TRUE;
}



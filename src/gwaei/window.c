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
//! @file window.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <gwaei/gwaei.h>
#include <gwaei/window-private.h>

G_DEFINE_ABSTRACT_TYPE (GwWindow, gw_window, GTK_TYPE_APPLICATION_WINDOW)

typedef enum
{
  PROP_0,
  PROP_APPLICATION,
  PROP_UI_XML
} GwWindowProps;

static gboolean gw_window_load_ui_xml (GwWindow*, const char*);

static void 
gw_window_init (GwWindow *window)
{
    window->priv = GW_WINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwWindowPrivate));
}


static void 
gw_window_finalize (GObject *object)
{
    GwWindow *window;
    GwWindowPrivate *priv;

    window = GW_WINDOW (object);
    priv = window->priv;

    priv->application = NULL;
    if (priv->builder != NULL) g_object_unref (priv->builder);
    if (priv->ui_xml != NULL) g_free (priv->ui_xml);
    priv->toplevel = NULL;
    if (priv->accelgroup) g_object_unref (priv->accelgroup); priv->accelgroup = NULL;

    G_OBJECT_CLASS (gw_window_parent_class)->finalize (object);
}


static void 
gw_window_constructed (GObject *object)
{
    GwWindow *window;
    GwWindowPrivate *priv;
    gboolean os_shows_app_menu;
    GtkSettings *settings;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_window_parent_class)->constructed (object);
    }

    window = GW_WINDOW (object);
    priv = window->priv;
    settings = gtk_settings_get_default ();
    g_object_get (settings, "gtk-shell-shows-app-menu", &os_shows_app_menu, NULL);
    gtk_widget_add_events (GTK_WIDGET (window), GDK_FOCUS_CHANGE_MASK);

    priv->accelgroup = gtk_accel_group_new ();
    gtk_window_add_accel_group (GTK_WINDOW (window), priv->accelgroup);
    gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (priv->application));
    priv->builder = gtk_builder_new ();
    gw_window_load_ui_xml (window, priv->ui_xml);
    priv->toplevel = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "toplevel"));

    gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (window), FALSE);

    g_signal_connect (G_OBJECT (window), "configure-event", G_CALLBACK (gw_window_configure_event_cb), NULL);
    g_signal_connect (window, "focus-in-event", G_CALLBACK (gw_window_focus_in_event_cb), NULL);
}


static void 
gw_window_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    GwWindow *window;
    GwWindowPrivate *priv;

    window = GW_WINDOW (object);
    priv = window->priv;

    switch (property_id)
    {
      case PROP_APPLICATION:
        priv->application = GW_APPLICATION (g_value_get_object (value));
        break;
      case PROP_UI_XML:
        if (priv->ui_xml != NULL)
          g_free (priv->ui_xml);
        priv->ui_xml = g_value_dup_string (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void 
gw_window_get_property (GObject      *object,
                        guint         property_id,
                        GValue       *value,
                        GParamSpec   *pspec)
{
    GwWindow *window;
    GwWindowPrivate *priv;

    window = GW_WINDOW (object);
    priv = window->priv;

    switch (property_id)
    {
      case PROP_APPLICATION:
        g_value_set_object (value, priv->application);
        break;
      case PROP_UI_XML:
        g_value_set_string (value, priv->ui_xml);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void
gw_window_class_init (GwWindowClass *klass)
{
    //Declarations
    GParamSpec *pspec;
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = gw_window_set_property;
    object_class->get_property = gw_window_get_property;
    object_class->constructed = gw_window_constructed;
    object_class->finalize = gw_window_finalize;

    g_type_class_add_private (object_class, sizeof (GwWindowPrivate));

    pspec = g_param_spec_object ("application",
                                 "Application construct prop",
                                 "Set GwWindow's Application",
                                 GW_TYPE_APPLICATION,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_APPLICATION, pspec);

    pspec = g_param_spec_string ("ui-xml",
                                 "XML filename construct prop",
                                 "Set GwWindow's ui xml",
                                 "",
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_UI_XML, pspec);
}


//!
//! @brief Loads the gtk builder xml file from the usual paths
//!
//! @param filename The filename of the xml file to look for
//!
static gboolean 
gw_window_load_ui_xml (GwWindow *window, const char *filename)
{
    if (window == NULL || filename == NULL) return FALSE;

    //Declarations
    GwWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkWidget *unused;
    gchar *paths[4];
    gchar **iter;
    gchar *path;
    gboolean loaded;

    //Initializations
    priv = window->priv;
    paths[0] = g_build_filename (filename, NULL);
    paths[1] = g_build_filename ("..", "share", PACKAGE, filename, NULL);
    paths[2] = g_build_filename (DATADIR2, PACKAGE, filename, NULL);
    paths[3] = NULL;
    loaded = FALSE;

    //Search for the files
    for (iter = paths; *iter != NULL && loaded == FALSE; iter++)
    {
      path = *iter;
      if (g_file_test (path, G_FILE_TEST_IS_REGULAR) && gtk_builder_add_from_file (priv->builder, path,  NULL))
      {
        gtk_builder_connect_signals (priv->builder, NULL);

        unused = GTK_WIDGET (gtk_builder_get_object (priv->builder, "unused"));
        toplevel = GTK_WIDGET (gtk_builder_get_object (priv->builder, "toplevel"));
        g_assert (unused != NULL && toplevel != NULL);
        g_object_ref(toplevel);
        gtk_container_remove (GTK_CONTAINER (unused), toplevel);
        gtk_container_add (GTK_CONTAINER (window), toplevel);
        g_object_unref(toplevel);
  
        gtk_widget_destroy (unused); unused = NULL;

        loaded = TRUE;
      }
    }

    //Cleanup
    for (iter = paths; *iter != NULL; iter++)
    {
      g_free (*iter);
    }

    //Bug test
    g_assert (loaded);

    //Return
    return loaded;
}


void
gw_window_unload_xml (GwWindow *window) {
  GwWindowPrivate *priv;

  priv = window->priv;

  g_object_unref (priv->builder);
  priv->builder = NULL;
}


GObject* 
gw_window_get_object (GwWindow *window, const char *ID)
{
    GwWindowPrivate *priv;

    priv = window->priv;

    return G_OBJECT (gtk_builder_get_object (priv->builder, ID));
}


void 
gw_window_set_application (GwWindow *window, GwApplication *application)
{
    GwWindowPrivate *priv;

    priv = window->priv;
    priv->application = application;
    gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (application));
}


GwApplication* 
gw_window_get_application (GwWindow *window)
{
    GwWindowPrivate *priv;

    priv = window->priv;

    return priv->application;
}


GtkWidget*
gw_window_get_toplevel (GwWindow *window)
{
    GwWindowPrivate *priv;

    priv = window->priv;

    return priv->toplevel;
}


GtkAccelGroup*
gw_window_get_accel_group (GwWindow *window)
{
    GwWindowPrivate *priv;

    priv = window->priv;

    return priv->accelgroup;
}


void
gw_window_set_is_important (GwWindow *window, gboolean important)
{
    GwWindowPrivate *priv;

    priv = window->priv;

    priv->important = important;
}


gboolean
gw_window_is_important (GwWindow *window)
{
    GwWindowPrivate *priv;

    priv = window->priv;

    return priv->important;
}


void
gw_window_load_size (GwWindow *window)
{
    GwApplication *application;
    LwPreferences *preferences;
    gchar buffer[500];
    gchar **atoms;
    gchar **atom;
    gchar **ptr;
    gchar *endptr;
    const gchar* NAME;
    gint width, height;

    application = gw_window_get_application (window);
    preferences = gw_application_get_preferences (application);
    lw_preferences_get_string_by_schema (preferences, buffer, LW_SCHEMA_BASE, LW_KEY_WINDOW_SIZE, 500);
    NAME = G_OBJECT_TYPE_NAME (window);

    atoms = g_strsplit (buffer, ";", -1);
    if (atoms != NULL)
    {
      //look for the correct window name
      ptr = atoms;
      while (*ptr != NULL && strncmp(*ptr, NAME, strlen(NAME)) != 0) ptr++;

      //if it exists, get the info for it
      if (*ptr != NULL)
      {
        atom = g_strsplit_set (*ptr, ":,", 3);
        if (g_strv_length (atom) == 3)
        {
          width = (gint) g_ascii_strtoll (atom[1], &endptr, 10);
          height = (gint) g_ascii_strtoll (atom[2], &endptr, 10);
          gint default_width, default_height;
          gtk_window_get_default_size (GTK_WINDOW (window), &default_width, &default_height);
          if (width > 0 && width != default_width && height > 0 && height != default_height)
          {
            gtk_window_set_default_size (GTK_WINDOW (window), width, height);
          }
        }
        if (atom != NULL) g_strfreev (atom); atom = NULL;
      }
      g_strfreev (atoms); atoms = NULL;
    }
}


void
gw_window_save_size (GwWindow *window)
{
    GwWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    gchar buffer[500];
    gchar *new_buffer;
    gchar **atoms;
    gchar *atom;
    gchar **ptr;
    const gchar *NAME;

    priv = window->priv;
    application = gw_window_get_application (window);
    preferences = gw_application_get_preferences (application);
    new_buffer = NULL;
    NAME = G_OBJECT_TYPE_NAME (window);

    atom = g_strdup_printf ("%s:%d,%d", NAME, priv->width, priv->height);
    if (atom != NULL)  //Atom is sometimes freed as part of g_strfreev!
    {
      lw_preferences_get_string_by_schema (preferences, buffer, LW_SCHEMA_BASE, LW_KEY_WINDOW_SIZE, 500);
      atoms = g_strsplit (buffer, ";", -1);
      if (atoms != NULL)
      {
        ptr = atoms;
        while (*ptr != NULL && strncmp(*ptr, NAME, strlen(NAME)) != 0) ptr++;

        if (*ptr != NULL)
        {
          g_free (*ptr);
          *ptr = atom;
          new_buffer = g_strjoinv (";", atoms);
        }
        else
        {
          if (*buffer != '\0')
            new_buffer = g_strjoin (";", buffer, atom, NULL);
          else
            new_buffer = g_strdup (atom);
          g_free (atom); atom = NULL;
        }
        g_strfreev (atoms); atoms = NULL;
      }
    }

    //set our new buffer to the prefs
    if (new_buffer != NULL)
    {
      lw_preferences_set_string_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_WINDOW_SIZE, new_buffer);
      g_free (new_buffer); new_buffer = NULL;
    }
}


void
gw_window_load_menubar (GwWindow *window, const gchar* BASE_NAME)
{
    //Declarations
    GwWindowPrivate *priv;
    GtkBuilder *builder;
    GtkApplication *application;
    GMenuModel *win_menu_model;
    gboolean loaded;
    gboolean os_shows_app_menu;
    gboolean os_shows_win_menu;
    gchar *filename;
    GtkWidget *menubar;
    GtkSettings *settings;
    
    //Initializations
    priv = window->priv;
    application = GTK_APPLICATION (gw_window_get_application (window));
    menubar = NULL;
    loaded = FALSE;
    builder = NULL;
    filename = NULL;
    win_menu_model = NULL;

    settings = gtk_settings_get_default ();
    g_object_get (settings, "gtk-shell-shows-app-menu", &os_shows_app_menu, NULL);
    g_object_get (settings, "gtk-shell-shows-menubar", &os_shows_win_menu, NULL);

    builder = gtk_builder_new (); 
    if (builder == NULL) goto errored;

    if (os_shows_app_menu && os_shows_win_menu) //Mac OS X style
    {
      filename = g_strjoin ("-", BASE_NAME, "menumodel", "macosx.ui", NULL);
      if (filename == NULL) goto errored;
    }
    else if (os_shows_app_menu != os_shows_win_menu) //Gnome 3 style
    {
      filename = g_strjoin ("-", BASE_NAME, "menumodel", "gnome.ui", NULL);
      if (filename == NULL) goto errored;
    }
    else //Windows style
    {
      filename = g_strjoin ("-", BASE_NAME, "menumodel", "standard.ui", NULL);
      if (filename == NULL) goto errored;
    }

    loaded = gw_application_load_xml (builder, filename); 
    if (loaded == FALSE) goto errored;
    win_menu_model = G_MENU_MODEL (gtk_builder_get_object (builder, "menu")); 
    if (win_menu_model == NULL) goto errored;

    //Set the whole menu to the window if appropriate
    if (os_shows_win_menu == FALSE)
    {
      menubar = gtk_menu_bar_new_from_model (win_menu_model);
      if (menubar == NULL) goto errored;
      gtk_box_pack_end (GTK_BOX (priv->toplevel), menubar, FALSE, FALSE, 0);
      gtk_widget_show_all (menubar);
    }

    //Set the menubar to the application
    gw_application_set_win_menubar (GW_APPLICATION (application), win_menu_model); //FIXME

    //Save the menu objects in the window
    if (priv->menu_model != NULL) g_object_unref (priv->menu_model);
    priv->menu_model = win_menu_model; win_menu_model = NULL;
    if (priv->menubar != NULL) gtk_widget_destroy (GTK_WIDGET (priv->menubar)); 
    priv->menubar = GTK_MENU_BAR (menubar); menubar = NULL;

errored:
    if (builder != NULL) g_object_unref (builder); builder = NULL;
    if (filename != NULL) g_free (filename); filename = NULL;
    if (win_menu_model != NULL) g_object_unref (G_OBJECT (win_menu_model));
    if (menubar != NULL) { g_object_ref_sink (menubar); gtk_widget_destroy (menubar); };
}


GMenuModel*
gw_window_get_menumodel (GwWindow *window)
{
    //Sanity checks
    g_return_val_if_fail (window != NULL, NULL);

    return window->priv->menu_model;
}


void 
gw_window_show_menubar (GwWindow *window, gboolean show)
{
    //Sanity checks
    g_return_if_fail (window != NULL);
    if (window->priv->menubar == NULL) return;

    //Declarations
    GwWindowPrivate *priv;

    //Initializations
    priv = window->priv;

    if (show == TRUE)
      gtk_widget_show (GTK_WIDGET (priv->menubar));
    else
      gtk_widget_hide (GTK_WIDGET (priv->menubar));
}


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

#include <gwaei/gwaei.h>
#include <gwaei/window-private.h>

G_DEFINE_TYPE (GwWindow, gw_window, GTK_TYPE_WINDOW)

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

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_window_parent_class)->constructed (object);
    }

    window = GW_WINDOW (object);
    priv = window->priv;

    priv->accelgroup = gtk_accel_group_new ();
    gtk_window_add_accel_group (GTK_WINDOW (window), priv->accelgroup);
    gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (priv->application));
    priv->builder = gtk_builder_new ();
    gw_window_load_ui_xml (window, priv->ui_xml);
    priv->toplevel = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "toplevel"));

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
    char *paths[4];
    char **iter;
    char *path;
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

        toplevel = GTK_WIDGET (gtk_builder_get_object (priv->builder, "toplevel"));
        g_assert (toplevel != NULL);
        gtk_widget_reparent (toplevel, GTK_WIDGET (window));

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

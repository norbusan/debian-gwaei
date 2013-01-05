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
//! @file tagtable.c
//!
//! @brief To be written
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/texttagtable-private.h>
#include <gwaei/gettext.h>

static void gw_texttagtable_init_base_tags (GwTextTagTable*);
static void gw_texttagtable_attach_signals (GwTextTagTable*);
static void gw_texttagtable_remove_signals (GwTextTagTable*);
static void gw_texttagtable_sync_tag_cb (GSettings*, gchar*, gpointer);

G_DEFINE_TYPE (GwTextTagTable, gw_texttagtable, GTK_TYPE_TEXT_TAG_TABLE)

typedef enum
{
  PROP_0,
  PROP_APPLICATION,
} GwTextTagTableProps;


GtkTextTagTable*
gw_texttagtable_new (GwApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwTextTagTable *model;

    //Initializations
    model = GW_TEXTTAGTABLE (g_object_new (GW_TYPE_TEXTTAGTABLE, "application", application, NULL));

    return GTK_TEXT_TAG_TABLE (model);
}


static void 
gw_texttagtable_init (GwTextTagTable *tagtable)
{
    tagtable->priv = GW_TEXTTAGTABLE_GET_PRIVATE (tagtable);
    memset(tagtable->priv, 0, sizeof(GwTextTagTablePrivate));
}


static void 
gw_texttagtable_finalize (GObject *object)
{
    GwTextTagTable *tagtable;

    tagtable = GW_TEXTTAGTABLE (object);

    gw_texttagtable_remove_signals (tagtable);

    G_OBJECT_CLASS (gw_texttagtable_parent_class)->finalize (object);
}


static void 
gw_texttagtable_constructed (GObject *object)
{
    //Declarations
    GwTextTagTable *tagtable;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_texttagtable_parent_class)->constructed (object);
    }

    //Initializations
    tagtable = GW_TEXTTAGTABLE (object);

    gw_texttagtable_init_base_tags (tagtable);
    gw_texttagtable_attach_signals (tagtable);
}


static void 
gw_texttagtable_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GwTextTagTable *tagtable;
    GwTextTagTablePrivate *priv;

    tagtable = GW_TEXTTAGTABLE (object);
    priv = tagtable->priv;

    switch (property_id)
    {
      case PROP_APPLICATION:
        priv->application = GW_APPLICATION (g_value_get_object (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void 
gw_texttagtable_get_property (GObject      *object,
                              guint         property_id,
                              GValue       *value,
                              GParamSpec   *pspec)
{
    GwTextTagTable *tagtable;
    GwTextTagTablePrivate *priv;

    tagtable = GW_TEXTTAGTABLE (object);
    priv = tagtable->priv;

    switch (property_id)
    {
      case PROP_APPLICATION:
        g_value_set_object (value, priv->application);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void
gw_texttagtable_class_init (GwTextTagTableClass *klass)
{
    //Declarations
    GParamSpec *pspec;
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = gw_texttagtable_set_property;
    object_class->get_property = gw_texttagtable_get_property;
    object_class->finalize = gw_texttagtable_finalize;
    object_class->constructed = gw_texttagtable_constructed;

    g_type_class_add_private (object_class, sizeof (GwTextTagTablePrivate));

    pspec = g_param_spec_object ("application",
                                 "Application construct prop",
                                 "Set GwWindow's Application",
                                 GW_TYPE_APPLICATION,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_APPLICATION, pspec);
}


static void
gw_texttagtable_init_base_tags (GwTextTagTable *tagtable)
{
    //Declarations
    GtkTextTag *tag;

    tag = gtk_text_tag_new ("entry-grand-header");
    g_object_set (tag, "scale", 5.0, "family", "KanjiStrokeOrders", NULL);
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("entry-header");
    g_object_set (tag, "scale", 1.3, "weight", 600, NULL);
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("entry-definition");
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("entry-lexicon");
    g_object_set (tag, "scale", 1.0, "foreground", "#888888", "style", PANGO_STYLE_ITALIC, NULL);
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("entry-popular");
    g_object_set (tag, "scale", 1.0, NULL);
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("entry-example-definition");
    g_object_set (tag, "scale", 1.3, NULL);
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("entry-bullet");
    g_object_set (tag, "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("comment");
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("match");
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("header");
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);

    tag = gtk_text_tag_new ("spacing");
    g_object_set (tag, "scale", 0.5, NULL);
    gtk_text_tag_table_add (GTK_TEXT_TAG_TABLE (tagtable), tag);
    g_object_unref (tag);
}


static void
gw_texttagtable_attach_signals (GwTextTagTable *tagtable)
{
    GwTextTagTablePrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;

    priv = tagtable->priv;
    application = priv->application;
    preferences = gw_application_get_preferences (application);

    priv->signalid[GW_TEXTTAGTABLE_SIGNALID_MATCH_FG] = lw_preferences_add_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        LW_KEY_MATCH_FG, 
        gw_texttagtable_sync_tag_cb, 
        tagtable
    );

    priv->signalid[GW_TEXTTAGTABLE_SIGNALID_MATCH_BG] = lw_preferences_add_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        LW_KEY_MATCH_BG, 
        gw_texttagtable_sync_tag_cb, 
        tagtable
    );

    priv->signalid[GW_TEXTTAGTABLE_SIGNALID_HEADER_FG] = lw_preferences_add_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        LW_KEY_HEADER_FG, 
        gw_texttagtable_sync_tag_cb, 
        tagtable
    );

    priv->signalid[GW_TEXTTAGTABLE_SIGNALID_HEADER_BG] = lw_preferences_add_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        LW_KEY_HEADER_BG, 
        gw_texttagtable_sync_tag_cb, 
        tagtable
    );

    priv->signalid[GW_TEXTTAGTABLE_SIGNALID_COMMENT_FG] = lw_preferences_add_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        LW_KEY_COMMENT_FG, 
        gw_texttagtable_sync_tag_cb, 
        tagtable
    );
}


static void
gw_texttagtable_remove_signals (GwTextTagTable *tagtable)
{
    GwTextTagTablePrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;

    priv = tagtable->priv;
    application = priv->application;
    preferences = gw_application_get_preferences (application);

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalid[GW_TEXTTAGTABLE_SIGNALID_MATCH_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalid[GW_TEXTTAGTABLE_SIGNALID_MATCH_BG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalid[GW_TEXTTAGTABLE_SIGNALID_HEADER_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalid[GW_TEXTTAGTABLE_SIGNALID_HEADER_BG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalid[GW_TEXTTAGTABLE_SIGNALID_COMMENT_FG]
    );
}


//!
//! @brief Resets the color tags according to the preferences
//!
static void
gw_texttagtable_sync_tag_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    gchar hex[20];
    GdkRGBA color;
    gchar **pair;
    GtkTextTag *tag;
    GtkTextTagTable *tagtable;

    tagtable = GTK_TEXT_TAG_TABLE (data);

    //Parse the color
    lw_preferences_get_string (hex, settings, key, 20);
    if (gdk_rgba_parse (&color, hex) == FALSE)
    {
      fprintf(stderr, "Failed to set tag to the tag table: %s\n", hex);
      lw_preferences_reset_value (settings, key);
      return;
    }

    //Update the tag 
    pair = g_strsplit (key, "-", 2);
    if (pair != NULL && pair[0] != NULL && pair[1] != NULL)
    {
      tag = gtk_text_tag_table_lookup (tagtable, pair[0]);
      g_object_set (G_OBJECT (tag), pair[1], hex, NULL);
      g_strfreev (pair);
    }
}



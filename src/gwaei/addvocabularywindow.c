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
//! @file addvocabularywindow.c
//!
//! @brief To be written
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/addvocabularywindow-private.h>

static void gw_addvocabularywindow_init_accelerators (GwAddVocabularyWindow*);
static void gw_addvocabularywindow_init_combobox (GwAddVocabularyWindow*);

G_DEFINE_TYPE (GwAddVocabularyWindow, gw_addvocabularywindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_addvocabularywindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwAddVocabularyWindow *window;

    //Initializations
    window = GW_ADDVOCABULARYWINDOW (g_object_new (GW_TYPE_ADDVOCABULARYWINDOW,
                                                   "type",        GTK_WINDOW_TOPLEVEL,
                                                   "application", GW_APPLICATION (application),
                                                   "ui-xml",      "addvocabularywindow.ui",
                                                   NULL));

    return GTK_WINDOW (window);
}


static void 
gw_addvocabularywindow_init (GwAddVocabularyWindow *window)
{
    window->priv = GW_ADDVOCABULARYWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwAddVocabularyWindowPrivate));
}


static void 
gw_addvocabularywindow_finalize (GObject *object)
{
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;

    window = GW_ADDVOCABULARYWINDOW (object);
    priv = window->priv;

    if (priv->kanji_text != NULL) g_free (priv->kanji_text);
    if (priv->furigana_text != NULL) g_free (priv->furigana_text);
    if (priv->definitions_text != NULL) g_free (priv->definitions_text);
    if (priv->list_text != NULL) g_free (priv->list_text);

    G_OBJECT_CLASS (gw_addvocabularywindow_parent_class)->finalize (object);
}


static void 
gw_addvocabularywindow_constructed (GObject *object)
{
    //Declarations
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_addvocabularywindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_ADDVOCABULARYWINDOW (object);
    priv = window->priv;

    //Set up the gtkbuilder links
    priv->kanji_entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "kanji_entry"));
    priv->furigana_entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "furigana_entry"));
    priv->definitions_textview = GTK_TEXT_VIEW (gw_window_get_object (GW_WINDOW (window), "definitions_textview"));
    priv->cancel_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "cancel_button"));
    priv->add_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "add_button"));
    priv->vocabulary_list_combobox = GTK_COMBO_BOX (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_combobox"));

    //Set up the gtk window
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_default_size (GTK_WINDOW (window), 500, 250);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_title (GTK_WINDOW (window), gettext("Add Vocabulary Word..."));
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), FALSE);
    gtk_window_set_modal (GTK_WINDOW (window), TRUE);
    gtk_window_set_default (GTK_WINDOW (window), GTK_WIDGET (priv->add_button));

    gw_addvocabularywindow_init_accelerators (window);
    gw_addvocabularywindow_init_combobox (window);

    gw_window_unload_xml (GW_WINDOW (window));
}


static void
gw_addvocabularywindow_class_init (GwAddVocabularyWindowClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = gw_addvocabularywindow_constructed;
    object_class->finalize = gw_addvocabularywindow_finalize;

    klass->last_selected_list_name = NULL;

    g_type_class_add_private (object_class, sizeof (GwAddVocabularyWindowPrivate));

    klass->signalid[GW_ADDVOCABULARYWINDOW_CLASS_SIGNALID_WORD_ADDED] = g_signal_new (
        "word-added",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (GwAddVocabularyWindowClass, word_added),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );
}


static void
gw_addvocabularywindow_init_accelerators (GwAddVocabularyWindow *window)
{
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;

    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));

    //Set menu accelerators
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "cancel_button"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Escape), 0, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "add_button"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_ISO_Enter), 0, GTK_ACCEL_VISIBLE);
}


static void
gw_addvocabularywindow_init_combobox (GwAddVocabularyWindow *window)
{
    GwAddVocabularyWindowPrivate *priv;
    GwAddVocabularyWindowClass *klass;
    GwApplication *application;
    GtkListStore *store;
    GtkTreeModel *model;

    priv = window->priv;
    klass = GW_ADDVOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    application = gw_window_get_application (GW_WINDOW (window));
    store = gw_application_get_vocabularyliststore (application);
    model = GTK_TREE_MODEL (store);

    //Initialize the combobox
    gtk_combo_box_set_model (priv->vocabulary_list_combobox, model); 

    //Remove the default entry since it doesn't seem to be editable for some reason
    priv->list_entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->vocabulary_list_combobox)));
    gtk_widget_destroy (GTK_WIDGET (priv->list_entry));

    //Add our entry
    priv->list_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_entry_set_activates_default (priv->list_entry, TRUE);
    g_signal_connect (G_OBJECT (priv->list_entry), "changed", G_CALLBACK (gw_addvocabularywindow_list_changed_cb), window);
    gtk_widget_show (GTK_WIDGET (priv->list_entry));
    gtk_combo_box_set_entry_text_column (priv->vocabulary_list_combobox, GW_VOCABULARYLISTSTORE_COLUMN_NAME);
    gtk_container_add (GTK_CONTAINER (priv->vocabulary_list_combobox), GTK_WIDGET (priv->list_entry));

    //Set the correct initial selection
    if (klass->last_selected_list_name != NULL)
    {
      gtk_entry_set_text (priv->list_entry, klass->last_selected_list_name);
    }
    else
    {
      gtk_combo_box_set_active (priv->vocabulary_list_combobox, 0);
    }
    gtk_editable_select_region (GTK_EDITABLE (priv->list_entry), 0, 0);
}


const gchar*
gw_addvocabularywindow_get_kanji (GwAddVocabularyWindow *window)
{
    GwAddVocabularyWindowPrivate *priv;
    priv = window->priv;

    if (priv->kanji_text == NULL) priv->kanji_text = g_strdup ("");

    return priv->kanji_text;
}

void
gw_addvocabularywindow_set_kanji (GwAddVocabularyWindow *window, const gchar *KANJI)
{
    if (KANJI == NULL) return;

    gtk_entry_set_text (window->priv->kanji_entry, KANJI);
}

const gchar*
gw_addvocabularywindow_get_furigana (GwAddVocabularyWindow *window)
{
    GwAddVocabularyWindowPrivate *priv;
    priv = window->priv;

    if (priv->furigana_text == NULL) priv->furigana_text = g_strdup ("");

    return priv->furigana_text;
}

void
gw_addvocabularywindow_set_furigana (GwAddVocabularyWindow *window, const gchar *FURIGANA)
{
    if (FURIGANA == NULL) return;

    gtk_entry_set_text (window->priv->furigana_entry, FURIGANA);
}

const gchar*
gw_addvocabularywindow_get_definitions (GwAddVocabularyWindow *window)
{
    GwAddVocabularyWindowPrivate *priv;
    priv = window->priv;

    if (priv->definitions_text == NULL) priv->definitions_text = g_strdup ("");

    return priv->definitions_text;
}


void
gw_addvocabularywindow_set_definitions (GwAddVocabularyWindow *window, const gchar *DEFINITIONS)
{
    if (DEFINITIONS == NULL) return;

    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (window->priv->definitions_textview);
    gtk_text_buffer_set_text (buffer, DEFINITIONS, -1);
}


const gchar*
gw_addvocabularywindow_get_list (GwAddVocabularyWindow *window)
{
    if (window->priv->list_text == NULL) window->priv->list_text = g_strdup ("");
    return window->priv->list_text;
}


void
gw_addvocabularywindow_set_list (GwAddVocabularyWindow *window, const gchar *LIST)
{
    if (LIST == NULL) return;

    GtkEntry *entry;
    entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (window->priv->vocabulary_list_combobox)));
    gtk_entry_set_text (entry, LIST);
    gtk_editable_select_region (GTK_EDITABLE (window->priv->list_entry), 0, 0);
}


GtkListStore*
gw_addvocabularywindow_get_wordstore (GwAddVocabularyWindow *window)
{
    //Declarations
    GwApplication *application;
    GtkListStore *wordstore;
    GtkListStore *liststore;
    const gchar *name;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);
    name = gw_addvocabularywindow_get_list (window);
    wordstore = gw_vocabularyliststore_get_wordstore_by_name (GW_VOCABULARYLISTSTORE (liststore), name);

    return wordstore;
}


gboolean
gw_addvocabularywindow_validate (GwAddVocabularyWindow *window)
{
    GwAddVocabularyWindowPrivate *priv;
    const gchar *kanji, *furigana, *definitions, *list;
    gboolean has_kanji, has_furigana, has_definitions, has_list;
    gboolean valid;

    priv = window->priv;
    kanji = gw_addvocabularywindow_get_kanji (window);
    furigana = gw_addvocabularywindow_get_furigana (window);
    definitions = gw_addvocabularywindow_get_definitions (window);
    list = gw_addvocabularywindow_get_list (window);

    has_kanji = (strlen (kanji) > 0);
    has_furigana = (strlen (furigana) > 0);
    has_definitions = (strlen (definitions) > 0);
    has_list = (strlen (list) > 0);
    valid = ((has_kanji || has_furigana || has_definitions) && has_list);

    gtk_widget_set_sensitive (GTK_WIDGET (priv->add_button), valid);

    return valid;
}


void
gw_addvocabularywindow_focus_add_button (GwAddVocabularyWindow *window)
{
    if (gw_addvocabularywindow_validate (window))
      gtk_widget_grab_focus (GTK_WIDGET (window->priv->add_button));
}


gboolean
gw_addvocabularywindow_get_iter (GwAddVocabularyWindow *window, GtkTreeIter *iter)
{
    g_assert (iter != NULL);

    *iter = window->priv->iter;

    return window->priv->valid;
}


void
gw_addvocabularywindow_save (GwAddVocabularyWindow *window)
{
   if (window->priv->wordstore != NULL)
   {
     gw_vocabularywordstore_save (window->priv->wordstore, NULL);
   }
}


void
gw_addvocabularywindow_set_focus (GwAddVocabularyWindow *window, GwAddVocabularyWindowFocus focus)
{
    GwAddVocabularyWindowPrivate *priv;

    priv = window->priv;

    switch (focus)
    {
      case GW_ADDVOCABULARYWINDOW_FOCUS_LIST:
        gtk_widget_grab_focus (GTK_WIDGET (priv->list_entry));
        break;
      case GW_ADDVOCABULARYWINDOW_FOCUS_KANJI:
        gtk_widget_grab_focus (GTK_WIDGET (priv->kanji_entry));
        break;
      case GW_ADDVOCABULARYWINDOW_FOCUS_FURIGANA:
        gtk_widget_grab_focus (GTK_WIDGET (priv->furigana_entry));
        break;
      case GW_ADDVOCABULARYWINDOW_FOCUS_DEFINITIONS:
        gtk_widget_grab_focus (GTK_WIDGET (priv->definitions_textview));
        break;
      case GW_ADDVOCABULARYWINDOW_FOCUS_ADD_BUTTON:
        gtk_widget_grab_focus (GTK_WIDGET (priv->add_button));
        break;
      default:
        g_assert_not_reached ();
        break;
    }
}

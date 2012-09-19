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
//! @file vocabularywindow.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/vocabularyliststore.h>
#include <gwaei/vocabularywindow-private.h>


//Static declarations
static void gw_vocabularywindow_attach_signals (GwVocabularyWindow*);
static void gw_vocabularywindow_remove_signals (GwVocabularyWindow*);
static void gw_vocabularywindow_init_styles (GwVocabularyWindow*);
static void gw_vocabularywindow_init_list_treeview (GwVocabularyWindow*);
static void gw_vocabularywindow_init_word_treeview (GwVocabularyWindow*);
static void gw_vocabularywindow_init_accelerators (GwVocabularyWindow*);

enum {
    TARGET_LIST_ROW_STRING,
    TARGET_WORD_ROW_STRING
};

//Word model
static GtkTargetEntry word_row_dest_targets[] = { //Will accept drops
    { "words",    GTK_TARGET_SAME_WIDGET, TARGET_WORD_ROW_STRING }
};
static guint n_word_row_dest_targets = G_N_ELEMENTS (word_row_dest_targets);

static GtkTargetEntry word_row_source_targets[] = { //Data given to drags
    { "words",    GTK_TARGET_SAME_WIDGET, TARGET_WORD_ROW_STRING }
};
static guint n_word_row_source_targets = G_N_ELEMENTS (word_row_source_targets);

//List model
static GtkTargetEntry list_row_dest_targets[] = { //Will accept drops
    { "lists",    GTK_TARGET_SAME_WIDGET, TARGET_LIST_ROW_STRING },
    { "words",    GTK_TARGET_SAME_APP,    TARGET_WORD_ROW_STRING }
};
static guint n_list_row_dest_targets = G_N_ELEMENTS (list_row_dest_targets);

static GtkTargetEntry list_row_source_targets[] = { //Data given to drags
    { "lists",    GTK_TARGET_SAME_WIDGET, TARGET_LIST_ROW_STRING }
};
static guint n_list_row_source_targets = G_N_ELEMENTS (list_row_source_targets);




G_DEFINE_TYPE (GwVocabularyWindow, gw_vocabularywindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_vocabularywindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (g_object_new (GW_TYPE_VOCABULARYWINDOW,
                                                "type",        GTK_WINDOW_TOPLEVEL,
                                                "application", GW_APPLICATION (application),
                                                "ui-xml",      "vocabularywindow.ui",
                                                NULL));

    return GTK_WINDOW (window);
}


static void 
gw_vocabularywindow_init (GwVocabularyWindow *window)
{
    window->priv = GW_VOCABULARYWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwVocabularyWindowPrivate));
}


static void 
gw_vocabularywindow_finalize (GObject *object)
{
    GwVocabularyWindow *window;
//    GwVocabularyWindowPrivate *priv;

    window = GW_VOCABULARYWINDOW (object);
//    priv = window->priv;

    gw_window_save_size (GW_WINDOW (window));

    G_OBJECT_CLASS (gw_vocabularywindow_parent_class)->finalize (object);
}


static void 
gw_vocabularywindow_constructed (GObject *object)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_vocabularywindow_parent_class)->constructed (object);
    }
    //Initializations
    window = GW_VOCABULARYWINDOW (object);
    priv = window->priv;

    //Set up the gtkbuilder links
    priv->list_treeview =   GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_treeview"));
    priv->list_toolbar =    GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_toolbar"));
    priv->word_treeview =   GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "vocabulary_word_treeview"));
    priv->word_toolbar =    GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "vocabulary_word_toolbar"));
    priv->study_toolbar =   GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "study_toolbar"));
    priv->edit_toolbutton = GTK_TOGGLE_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "edit_toolbutton"));
    priv->paned =           GTK_PANED (gw_window_get_object (GW_WINDOW (window), "vocabulary_paned"));

    priv->copy_menuitem = GTK_MENU_ITEM (gw_window_get_object (GW_WINDOW (window), "copy_menuitem"));
    priv->paste_menuitem = GTK_MENU_ITEM (gw_window_get_object (GW_WINDOW (window), "paste_menuitem"));
    priv->cut_menuitem = GTK_MENU_ITEM (gw_window_get_object (GW_WINDOW (window), "cut_menuitem"));
    priv->delete_menuitem = GTK_MENU_ITEM (gw_window_get_object (GW_WINDOW (window), "delete_menuitem"));

    priv->save_action =     GTK_ACTION (gw_window_get_object (GW_WINDOW (window), "save_action"));
    priv->revert_action =     GTK_ACTION (gw_window_get_object (GW_WINDOW (window), "revert_action"));

    priv->shuffle_toggleaction = 
      GTK_TOGGLE_ACTION (gw_window_get_object (GW_WINDOW (window), "shuffle_flashcards_toggleaction"));
    priv->trim_toggleaction = 
      GTK_TOGGLE_ACTION (gw_window_get_object (GW_WINDOW (window), "trim_flashcards_toggleaction"));
    priv->track_results_toggleaction = 
      GTK_TOGGLE_ACTION (gw_window_get_object (GW_WINDOW (window), "track_results_toggleaction"));
    priv->show_toolbar_toggleaction =
      GTK_TOGGLE_ACTION (gw_window_get_object (GW_WINDOW (window), "show_toolbar_toggleaction"));
    priv->show_position_column_toggleaction =
      GTK_TOGGLE_ACTION (gw_window_get_object (GW_WINDOW (window), "show_position_column_toggleaction"));
    priv->show_score_column_toggleaction =
      GTK_TOGGLE_ACTION (gw_window_get_object (GW_WINDOW (window), "show_score_column_toggleaction"));
    priv->show_timestamp_column_toggleaction =
      GTK_TOGGLE_ACTION (gw_window_get_object (GW_WINDOW (window), "show_timestamp_column_toggleaction"));

    priv->kanji_definition_flashcards_action = 
      GTK_ACTION (gw_window_get_object (GW_WINDOW (window), "kanji_definition_flashcards_action"));
    priv->definition_kanji_flashcards_action = 
      GTK_ACTION (gw_window_get_object (GW_WINDOW (window), "definition_kanji_flashcards_action"));
    priv->kanji_furigana_flashcards_action = 
      GTK_ACTION (gw_window_get_object (GW_WINDOW (window), "kanji_furigana_flashcards_action"));
    priv->furigana_kanji_flashcards_action = 
      GTK_ACTION (gw_window_get_object (GW_WINDOW (window), "furigana_kanji_flashcards_action"));
    priv->definition_furigana_flashcards_action = 
      GTK_ACTION (gw_window_get_object (GW_WINDOW (window), "definition_furigana_flashcards_action"));
    priv->furigana_definition_flashcards_action = 
      GTK_ACTION (gw_window_get_object (GW_WINDOW (window), "furigana_definition_flashcards_action"));

    //Set up the gtk window
    gtk_window_set_title (GTK_WINDOW (window), gettext("gWaei Vocabulary Manager"));
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_set_default_size (GTK_WINDOW (window), 790, 450);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_NORMAL);
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), FALSE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), FALSE);
    gw_window_set_is_important (GW_WINDOW (window), TRUE);
    gw_window_load_size (GW_WINDOW (window));

    gw_vocabularywindow_init_styles (window);
    gw_vocabularywindow_init_word_treeview (window);
    gw_vocabularywindow_init_list_treeview (window);
    gw_vocabularywindow_init_accelerators (window);

    gw_vocabularywindow_attach_signals (window);

    {
      //Set the initial selection
      GtkTreeSelection *selection;
      GtkTreeModel *model;
      GtkListStore *store;
      GtkTreeIter iter;
      GtkTreePath *path;
      gboolean valid;
      selection = gtk_tree_view_get_selection (priv->list_treeview);
      model = gtk_tree_view_get_model (priv->list_treeview);
      valid = gtk_tree_model_get_iter_first (model, &iter);
      if (valid)
      {
        path = gtk_tree_model_get_path (model, &iter);
        if (path != NULL)
        { 
          gtk_tree_view_set_cursor (priv->list_treeview, path, NULL, FALSE);
          gtk_tree_path_free (path); path = NULL;
        }
        gtk_tree_selection_select_iter (selection, &iter);
        store = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
        gtk_tree_view_set_model (priv->word_treeview, GTK_TREE_MODEL (store));
      }
    }
    gtk_widget_grab_focus (GTK_WIDGET (priv->list_treeview));

    gw_window_unload_xml (GW_WINDOW (window));
    gw_vocabularywindow_update_flashcard_menu_sensitivities (window);
}


static void
gw_vocabularywindow_init_accelerators (GwVocabularyWindow *window)
{
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;
    gint i;
    const gchar *flashcard_id[6] =
    {
      "kanji_definition_flashcard_menuitem",
      "definition_kanji_flashcard_menuitem",
      "furigana_definition_flashcard_menuitem",
      "definition_furigana_flashcard_menuitem",
      "kanji_furigana_flashcard_menuitem",
      "furigana_kanji_flashcard_menuitem"
    };

    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));

    //File popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_list_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_word_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "save_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_S), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "import_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_O), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "export_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_S), GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "close_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Edit popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "revert_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Z), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "cut_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_X), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "copy_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_C), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "paste_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_V), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "delete_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Delete), 0, GTK_ACCEL_VISIBLE);

    for (i = 0; i < 6; i++)
    {
      widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), flashcard_id[i]));
      gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
        accelgroup, (GDK_KEY_0 + i), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    }
}


static void
gw_vocabularywindow_class_init (GwVocabularyWindowClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = gw_vocabularywindow_constructed;
    object_class->finalize = gw_vocabularywindow_finalize;

    g_type_class_add_private (object_class, sizeof (GwVocabularyWindowPrivate));
}


static void 
gw_vocabularywindow_attach_signals (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    GtkListStore *store;
    
    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    store = gw_application_get_vocabularyliststore (application);

    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gw_vocabularywindow_remove_signals), NULL);
    g_signal_connect (G_OBJECT (window), "delete-event",
                      G_CALLBACK (gw_vocabularywindow_delete_event_cb), window);
    g_signal_connect (G_OBJECT (window), "event-after",
                      G_CALLBACK (gw_vocabularywindow_event_after_cb), window);


    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_CHANGED] = g_signal_connect (
        G_OBJECT (store), 
        "changed", 
        G_CALLBACK (gw_vocabularywindow_liststore_changed_cb), 
        window);


    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TOOLBAR_TOGGLED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_TOOLBAR_SHOW,
        gw_vocabularywindow_sync_toolbar_show_cb,
        window
    );

    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_POSITION_COLUMN_TOGGLED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_POSITION_COLUMN_SHOW,
        gw_vocabularywindow_sync_position_column_show_cb,
        window
    );

    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_SCORE_COLUMN_TOGGLED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_SCORE_COLUMN_SHOW,
        gw_vocabularywindow_sync_score_column_show_cb,
        window
    );

    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TIMESTAMP_COLUMN_TOGGLED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_TIMESTAMP_COLUMN_SHOW,
        gw_vocabularywindow_sync_timestamp_column_show_cb,
        window
    );

    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_SHUFFLE_CHANGED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_SHUFFLE_FLASHCARDS,
        gw_vocabularywindow_sync_shuffle_flashcards_cb,
        window
    );

    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TRIM_CHANGED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_TRIM_FLASHCARDS,
        gw_vocabularywindow_sync_trim_flashcards_cb,
        window
    );

    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_LIST_ORDER_CHANGED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_LIST_ORDER,
        gw_vocabularywindow_sync_list_order_cb,
        window
    );

    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TRACK_RESULTS_CHANGED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_TRACK_RESULTS,
        gw_vocabularywindow_sync_track_results_cb,
        window
    );
}


static void 
gw_vocabularywindow_remove_signals (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    GtkListStore *store;
    GSource *source;
    gint i;

    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    store = gw_application_get_vocabularyliststore (application);

    for (i = 0; i < TOTAL_GW_VOCABULARYWINDOW_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          priv->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, priv->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      priv->timeoutid[i] = 0;
    }

    g_signal_handler_disconnect (G_OBJECT (store), priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_CHANGED]);

    lw_preferences_remove_change_listener_by_schema (
      preferences, 
      LW_SCHEMA_VOCABULARY,
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_SHUFFLE_CHANGED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_SHUFFLE_CHANGED] = 0;

    lw_preferences_remove_change_listener_by_schema (
      preferences, 
      LW_SCHEMA_VOCABULARY,
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TRIM_CHANGED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TRIM_CHANGED] = 0;

    lw_preferences_remove_change_listener_by_schema (
      preferences, 
      LW_SCHEMA_VOCABULARY,
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_LIST_ORDER_CHANGED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_LIST_ORDER_CHANGED] = 0;

    lw_preferences_remove_change_listener_by_schema (
      preferences, 
      LW_SCHEMA_VOCABULARY,
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TRACK_RESULTS_CHANGED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TRACK_RESULTS_CHANGED] = 0;

    lw_preferences_remove_change_listener_by_schema (
      preferences, 
      LW_SCHEMA_VOCABULARY,
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TOOLBAR_TOGGLED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TOOLBAR_TOGGLED] = 0;

    lw_preferences_remove_change_listener_by_schema (
      preferences, 
      LW_SCHEMA_VOCABULARY,
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_POSITION_COLUMN_TOGGLED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_POSITION_COLUMN_TOGGLED] = 0;


    lw_preferences_remove_change_listener_by_schema (
      preferences, 
      LW_SCHEMA_VOCABULARY,
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TIMESTAMP_COLUMN_TOGGLED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_TIMESTAMP_COLUMN_TOGGLED] = 0;

    lw_preferences_remove_change_listener_by_schema (
      preferences, 
      LW_SCHEMA_VOCABULARY,
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_SCORE_COLUMN_TOGGLED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_SCORE_COLUMN_TOGGLED] = 0;
}


static void
gw_vocabularywindow_init_styles (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkStyleContext *context;
    GtkWidget *widget;
    GtkJunctionSides sides;

    priv = window->priv;

    //Vocabulary list pane
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_scrolledwindow"));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
    gtk_widget_reset_style (widget);

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->list_toolbar));
    gtk_style_context_add_class (context, "inline-toolbar");
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
    gtk_widget_reset_style (GTK_WIDGET (priv->list_toolbar));

    //Vocabulary listitem pane
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "vocabulary_word_scrolledwindow"));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
    gtk_widget_reset_style (widget);

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->word_toolbar));
    gtk_style_context_add_class (context, "inline-toolbar");
    sides = GTK_JUNCTION_CORNER_TOPLEFT | GTK_JUNCTION_CORNER_TOPRIGHT | GTK_JUNCTION_CORNER_BOTTOMRIGHT;
    gtk_style_context_set_junction_sides (context, sides);
    gtk_widget_reset_style (GTK_WIDGET (priv->word_toolbar));

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->study_toolbar));
    gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
    sides = GTK_JUNCTION_CORNER_BOTTOMLEFT | GTK_JUNCTION_CORNER_TOPLEFT | GTK_JUNCTION_CORNER_TOPRIGHT;
    gtk_style_context_set_junction_sides (context, sides);
    gtk_widget_reset_style (GTK_WIDGET (priv->study_toolbar));

    GtkWidget *menu, *menutoolbutton;
    menu = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "toolbar_study_menu"));
    menutoolbutton = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "study_menutoolbutton"));
    gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (menutoolbutton), menu);
}


static void
gw_vocabularywindow_init_list_treeview (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeSelection *selection;
    GtkTreeModel *model;

    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    application = gw_window_get_application (GW_WINDOW (window));
    model = GTK_TREE_MODEL (gw_application_get_vocabularyliststore (application));

    //Set up the columns
    column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (priv->list_treeview, column);

    renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (GW_VOCABULARYLISTSTORE_COLUMN_NAME));
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (gw_vocabularywindow_list_cell_edited_cb), priv->list_treeview);
    gtk_tree_view_column_set_title (column, gettext("Lists"));
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, 
        "text", GW_VOCABULARYLISTSTORE_COLUMN_NAME, 
        "weight", GW_VOCABULARYLISTSTORE_COLUMN_CHANGED, 
        NULL);

    gtk_tree_view_set_model (priv->list_treeview, model);

    gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

    gtk_drag_source_set (
        GTK_WIDGET (priv->list_treeview), 
        GDK_BUTTON1_MASK,
        list_row_source_targets,
        n_list_row_source_targets,
        GDK_ACTION_MOVE
    );

    gtk_drag_dest_set (
        GTK_WIDGET (priv->list_treeview),
        GTK_DEST_DEFAULT_ALL,
        list_row_dest_targets,
        n_list_row_dest_targets,
        GDK_ACTION_MOVE
    );

    priv->paned_initial_size = gtk_paned_get_position (priv->paned);
}


static void
gw_vocabularywindow_init_word_treeview (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeSelection *selection;
    gboolean editable;

    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->word_treeview);

    //g_object_set (G_OBJECT (priv->word_treeview), "even-row-color", "#eeeeee", NULL);
    //gtk_tree_view_set_grid_lines (priv->word_treeview, GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
    gtk_tree_view_set_rules_hint (priv->word_treeview, TRUE);
    gtk_widget_set_has_tooltip (GTK_WIDGET (priv->word_treeview), TRUE);

    //Position Column
    priv->position_column = column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sort_column_id (column, GW_VOCABULARYWORDSTORE_COLUMN_POSITION_INTEGER);
    priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_POSITION_STRING] = renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "weight", PANGO_WEIGHT_SEMIBOLD, "scale", .75, NULL);
    gtk_tree_view_column_set_title (column, "#");
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, 
        "text",   GW_VOCABULARYWORDSTORE_COLUMN_POSITION_STRING, 
        NULL);
    gtk_tree_view_append_column (priv->word_treeview, column);

    //Kanji Column
    editable = gtk_toggle_tool_button_get_active (priv->edit_toolbutton);
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sort_column_id (column, GW_VOCABULARYWORDSTORE_COLUMN_KANJI);
    priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_KANJI] = renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "editable", editable, "scale", 1.25, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (GW_VOCABULARYWORDSTORE_COLUMN_KANJI));
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (gw_vocabularywindow_cell_edited_cb), priv->word_treeview);
    gtk_tree_view_column_set_title (column, gettext("Word"));
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, 
        "text",   GW_VOCABULARYWORDSTORE_COLUMN_KANJI, 
        "weight", GW_VOCABULARYWORDSTORE_COLUMN_WEIGHT,
        NULL);
    gtk_tree_view_append_column (priv->word_treeview, column);

    //Furigana Column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sort_column_id (column, GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA);
    priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA] = renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA));
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (gw_vocabularywindow_cell_edited_cb), priv->word_treeview);
    gtk_tree_view_column_set_title (column, gettext("Reading"));
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, 
        "text",   GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA, 
        "weight", GW_VOCABULARYWORDSTORE_COLUMN_WEIGHT,
        NULL);
    gtk_tree_view_append_column (priv->word_treeview, column);

    //Definitions Column
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sort_column_id (column, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);
    g_object_set (G_OBJECT (column), "expand", TRUE, NULL);
    priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS] = renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS));
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (gw_vocabularywindow_cell_edited_cb), priv->word_treeview);
    gtk_tree_view_column_set_title (column, gettext("Definitions"));
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, 
        "text",   GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS, 
        "weight", GW_VOCABULARYWORDSTORE_COLUMN_WEIGHT,
        NULL);
    gtk_tree_view_append_column (priv->word_treeview, column);

    //Date Column
    priv->timestamp_column = column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sort_column_id (column, GW_VOCABULARYWORDSTORE_COLUMN_TIMESTAMP);
    priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_DAYS] = renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "alignment", PANGO_ALIGN_RIGHT, "scale", 0.75, "weight", PANGO_WEIGHT_SEMIBOLD, NULL);
    gtk_tree_view_column_set_title (column, gettext("Last Studied"));
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, 
        "text",   GW_VOCABULARYWORDSTORE_COLUMN_DAYS, 
        NULL);
    gtk_tree_view_append_column (priv->word_treeview, column);

    //Score Column
    priv->score_column = column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sort_column_id (column, GW_VOCABULARYWORDSTORE_COLUMN_SCORE);
    priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_SCORE] = renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "alignment", PANGO_ALIGN_RIGHT, "scale", 0.75, "weight", PANGO_WEIGHT_SEMIBOLD, NULL);
    gtk_tree_view_column_set_title (column, gettext("Score"));
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, 
        "text",   GW_VOCABULARYWORDSTORE_COLUMN_SCORE, 
        NULL);
    gtk_tree_view_append_column (priv->word_treeview, column);

    GtkEntry *entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "vocabulary_search_entry"));
    gtk_tree_view_set_search_entry (priv->word_treeview, entry);

    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

    gtk_drag_source_set (
        GTK_WIDGET (priv->word_treeview), 
        GDK_BUTTON1_MASK,
        word_row_source_targets,
        n_word_row_source_targets,
        GDK_ACTION_MOVE
    );

    gtk_drag_dest_set (
        GTK_WIDGET (priv->word_treeview),
        GTK_DEST_DEFAULT_ALL,
        word_row_dest_targets,
        n_word_row_dest_targets,
        GDK_ACTION_MOVE
    );
}


gboolean
gw_vocabularywindow_current_wordstore_has_changes (GwVocabularyWindow *window)
{
   GwVocabularyWindowPrivate *priv;
   GtkTreeModel *model;
   GtkTreeSelection *selection;
   gboolean valid;
   gboolean has_changes;
   GtkTreeIter iter;
   GtkListStore *wordstore;

   priv = window->priv;
   model = gtk_tree_view_get_model (priv->list_treeview);
   selection = gtk_tree_view_get_selection (priv->list_treeview);
   valid = gtk_tree_selection_get_selected (selection, &model, &iter);
   has_changes = FALSE;

   if (valid)
   {
     wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
     has_changes = gw_vocabularywordstore_has_changes (GW_VOCABULARYWORDSTORE (wordstore));
   }

   return has_changes;
}


void
gw_vocabularywindow_set_has_changes (GwVocabularyWindow *window, gboolean has_changes)
{
   GwVocabularyWindowPrivate *priv;
   gboolean wordstore_has_changes;

   priv = window->priv;
   priv->has_changes = has_changes;
   wordstore_has_changes = gw_vocabularywindow_current_wordstore_has_changes (window);

   gtk_action_set_sensitive (priv->save_action, has_changes);
   gtk_action_set_sensitive (priv->revert_action, wordstore_has_changes);
}


gboolean
gw_vocabularywindow_has_changes (GwVocabularyWindow *window)
{
    return (window->priv->has_changes);
}


void
gw_vocabularywindow_start_flashcards (GwVocabularyWindow *window,
                                      const gchar        *flash_cards_type,
                                      const gchar        *question_text,
                                      gint                question_column,
                                      gint                answer_column    )
{
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    GtkWindow *flashcardwindow;
    GtkTreeModel *model;
    GtkListStore *liststore;
    GwVocabularyWordStore *vocabularywordstore;
    GwFlashCardStore *flashcardstore;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean valid;
    gint max;


    if (window == NULL) return;
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    liststore = GTK_LIST_STORE (gtk_tree_view_get_model (priv->list_treeview));
    model = GTK_TREE_MODEL (liststore);
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    valid = gtk_tree_selection_get_selected (selection, &model, &iter);
    max = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_FLASHCARD_DECK_SIZE);

    if (valid)
    {
      vocabularywordstore = GW_VOCABULARYWORDSTORE (gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter));
      if ((gtk_tree_model_iter_n_children (GTK_TREE_MODEL (vocabularywordstore), NULL)) == 0) return;

      flashcardstore = GW_FLASHCARDSTORE (gw_flashcardstore_new ());
      gw_flashcardstore_set_vocabularywordstore (flashcardstore, vocabularywordstore, question_column, answer_column);
      if (priv->trim) gw_flashcardstore_trim (GW_FLASHCARDSTORE (flashcardstore), max);
      if (priv->shuffle) gw_flashcardstore_shuffle (flashcardstore);

      flashcardwindow = gw_flashcardwindow_new (GTK_APPLICATION (application));
      gw_flashcardwindow_set_model (
        GW_FLASHCARDWINDOW (flashcardwindow), 
        GW_FLASHCARDSTORE (flashcardstore),
        flash_cards_type,
        gw_vocabularywordstore_get_name (GW_VOCABULARYWORDSTORE (vocabularywordstore)),
        question_text
      );

      gw_flashcardwindow_set_track_results (GW_FLASHCARDWINDOW (flashcardwindow), priv->track);

      gtk_widget_show (GTK_WIDGET (flashcardwindow));
    }
}


void
gw_vocabularywindow_set_selected_list (GwVocabularyWindow *window, GtkTreePath *path)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkTreeView *view;

    //Initializations
    priv = window->priv;
    view = priv->list_treeview;

    gtk_tree_view_set_cursor (view, path, NULL, FALSE);
}


GtkListStore*
gw_vocabularywindow_get_selected_wordstore (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *liststore, *wordstore;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);
    model = GTK_TREE_MODEL (liststore);
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    wordstore = NULL;

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter);
    }

    return wordstore;
} 


gboolean
gw_vocabularywindow_show_save_dialog (GwVocabularyWindow *window)
{
    //Declarations
    GtkWidget *dialog;
    GwApplication *application;
    gint response;
    GtkWidget *box;
    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *content_area;
    gchar *markup;
    const gchar *header, *description;
    LwPreferences *preferences;
    GtkListStore *store;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    dialog = gtk_dialog_new ();
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
    gtk_window_set_title (GTK_WINDOW (dialog), gettext("Save changes before closing?"));
    gtk_dialog_add_button (GTK_DIALOG (dialog), gettext("Close _without Saving"), GTK_RESPONSE_NO);
    gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
    gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_SAVE, GTK_RESPONSE_YES);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);
    image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
    label = gtk_label_new (NULL);
    gtk_misc_set_padding (GTK_MISC (label), 0, 8);
    gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
    gtk_widget_show_all (box);

    header = gettext("Save Changes before Closing?");
    description = gettext("Some of your vocabulary lists have changed since your last save.");
    markup = g_markup_printf_escaped ("<big><b>%s</b></big>\n%s", header, description);
    gtk_label_set_markup (GTK_LABEL (label), markup);
    g_free (markup);
    gtk_container_add (GTK_CONTAINER (content_area), box);
    gtk_container_set_border_width (GTK_CONTAINER (box), 8);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
    response = gtk_dialog_run (GTK_DIALOG (dialog));
    preferences = gw_application_get_preferences (application);
    store = gw_application_get_vocabularyliststore (application);

    switch (response)
    {
      case GTK_RESPONSE_YES:
        gw_vocabularyliststore_save_all (GW_VOCABULARYLISTSTORE (store)); 
        gw_vocabularyliststore_save_list_order (GW_VOCABULARYLISTSTORE (store), preferences);
        gtk_widget_destroy (GTK_WIDGET (window));
        break;
      case GTK_RESPONSE_CANCEL:
        break;
      case GTK_RESPONSE_NO:
        gw_vocabularyliststore_revert_all (GW_VOCABULARYLISTSTORE (store)); 
        gw_vocabularyliststore_load_list_order (GW_VOCABULARYLISTSTORE (store), preferences);
        gtk_widget_destroy (GTK_WIDGET (window));
        break;
      default:
        break;
    }
    gtk_widget_destroy (GTK_WIDGET (dialog));

    return (response  == GTK_RESPONSE_YES || response == GTK_RESPONSE_NO);
}


void
gw_vocabularywindow_show_vocabulary_list (GwVocabularyWindow *window, gboolean request)
{
    GwVocabularyWindowPrivate *priv;

    priv = window->priv;
    
    if (request)
      gtk_paned_set_position (priv->paned, priv->paned_initial_size);
    else
      gtk_paned_set_position (priv->paned, 0);
}


void
gw_vocabularywindow_update_flashcard_menu_sensitivities (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    gint n_children;
    GtkTreeModel *model;

    //Initializations
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->word_treeview);
    if (model != NULL)
      n_children = gtk_tree_model_iter_n_children (model, NULL);
    else 
      n_children = 0;

    gtk_action_set_sensitive (priv->kanji_definition_flashcards_action, n_children);
    gtk_action_set_sensitive (priv->definition_kanji_flashcards_action, n_children);
    gtk_action_set_sensitive (priv->kanji_furigana_flashcards_action, n_children);
    gtk_action_set_sensitive (priv->furigana_kanji_flashcards_action, n_children);
    gtk_action_set_sensitive (priv->definition_furigana_flashcards_action, n_children);
    gtk_action_set_sensitive (priv->furigana_definition_flashcards_action, n_children);
}



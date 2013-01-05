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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/gettext.h>
#include <gwaei/vocabularyliststore.h>
#include <gwaei/vocabularywindow-private.h>


//Static declarations
static void gw_vocabularywindow_attach_signals (GwVocabularyWindow*);
static void gw_vocabularywindow_remove_signals (GwVocabularyWindow*);
static void gw_vocabularywindow_init_styles (GwVocabularyWindow*);
static void gw_vocabularywindow_init_list_treeview (GwVocabularyWindow*);
static void gw_vocabularywindow_init_word_treeview (GwVocabularyWindow*);
static void gw_vocabularywindow_initialize_toolbar (GwVocabularyWindow*);

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

    gw_window_load_menubar (GW_WINDOW (window), "vocabularywindow");

    gw_vocabularywindow_map_actions (G_ACTION_MAP (window), window);

    //Set up the gtkbuilder links
    priv->primary_toolbar = GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "primary_toolbar"));
    priv->list_treeview =   GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_treeview"));
    priv->list_toolbar =    GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_toolbar"));
    priv->word_treeview =   GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "vocabulary_word_treeview"));
    priv->word_toolbar =    GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "vocabulary_word_toolbar"));
    priv->edit_toolbutton = GTK_TOGGLE_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "edit_toolbutton"));
    priv->paned =           GTK_PANED (gw_window_get_object (GW_WINDOW (window), "vocabulary_paned"));

    gw_vocabularywindow_initialize_toolbar (window);
    gw_vocabularywindow_initialize_menu_links (window);

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
                      G_CALLBACK (gw_window_delete_event_cb), window);
    g_signal_connect (G_OBJECT (window), "event-after",
                      G_CALLBACK (gw_vocabularywindow_event_after_cb), window);


    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_CHANGED] = g_signal_connect (
        G_OBJECT (store), 
        "changed", 
        G_CALLBACK (gw_vocabularywindow_liststore_changed_cb), 
        window);

    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_MENUBAR_TOGGLED] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_VOCABULARY,
        LW_KEY_MENUBAR_SHOW,
        gw_vocabularywindow_sync_menubar_show_cb,
        window
    );

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
      priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_MENUBAR_TOGGLED]
    );
    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_MENUBAR_TOGGLED] = 0;

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
    {
      GActionMap *map = G_ACTION_MAP (window);
      GAction *action = g_action_map_lookup_action (map, "toggle-editable");
      GVariant *state = g_action_get_state (action);
      editable = g_variant_get_boolean (state);
      g_variant_unref (state); state = NULL;
    }
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


GwVocabularyWordStore*
gw_vocabularywindow_get_current_wordstore (GwVocabularyWindow *window)
{
   //Sanity checks
   g_return_val_if_fail (window != NULL, NULL);

   //Declarations
   GwVocabularyWindowPrivate *priv;
   GtkTreeModel *treemodel;

   //Initializations
   priv = window->priv;
   treemodel = gtk_tree_view_get_model (priv->word_treeview);

   return GW_VOCABULARYWORDSTORE (treemodel);
}


GwVocabularyListStore*
gw_vocabularywindow_get_liststore (GwVocabularyWindow *window)
{
   //Sanity checks
   g_return_val_if_fail (window != NULL, NULL);

   //Declarations
   GwVocabularyWindowPrivate *priv;
   GtkTreeModel *treemodel;

   //Initializations
   priv = window->priv;
   treemodel = gtk_tree_view_get_model (priv->list_treeview);

   return GW_VOCABULARYLISTSTORE (treemodel);
}


void
gw_vocabularywindow_sync_has_changes (GwVocabularyWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwVocabularyWordStore *wordstore;
    GwVocabularyListStore *liststore;
    gboolean wordstore_has_changes;
    gboolean wordstore_has_rows;
    gboolean liststore_has_rows;
    GSimpleAction *action;
    GActionMap *map;

    //Initializations
    wordstore = gw_vocabularywindow_get_current_wordstore (window);
    liststore = gw_vocabularywindow_get_liststore (window);
    wordstore_has_changes = wordstore != NULL && gw_vocabularywordstore_has_changes (wordstore);
    wordstore_has_rows = (wordstore != NULL && gtk_tree_model_iter_n_children (GTK_TREE_MODEL (wordstore), NULL) > 0);
    liststore_has_rows = (gtk_tree_model_iter_n_children (GTK_TREE_MODEL (liststore), NULL) > 0);
    map = G_ACTION_MAP (window);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "save-list"));
    g_simple_action_set_enabled (action, wordstore_has_changes);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "revert-list"));
    g_simple_action_set_enabled (action, wordstore_has_changes);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "new-word"));
    g_simple_action_set_enabled (action, wordstore != NULL);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "remove-word"));
    g_simple_action_set_enabled (action, wordstore_has_rows);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "remove-list"));
    g_simple_action_set_enabled (action, liststore_has_rows);
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


void
gw_vocabularywindow_set_selected_list_by_index (GwVocabularyWindow *window, gint index)
{
    //Sanity checks
    g_return_if_fail (window != NULL);
    
    //Declarations
    gchar *path_string;
    GtkTreePath *path;

    //Initializations
    if (index < -1) index = -1;
    path_string = g_strdup_printf ("%d", index);
    path = gtk_tree_path_new_from_string (path_string);

    gw_vocabularywindow_set_selected_list (window, path);

    g_free (path_string); path_string = NULL;
    gtk_tree_path_free (path); path = NULL;
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
    description = gettext("Some of your word lists have changed since your last save.");
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
    GActionMap *map;  
    GSimpleAction *action;
    gboolean enabled;

    //Initializations
    priv = window->priv;
    map = G_ACTION_MAP (window);
    model = gtk_tree_view_get_model (priv->word_treeview);
    n_children = 0;
    if (model != NULL) n_children = gtk_tree_model_iter_n_children (model, NULL);
    enabled = (n_children > 0);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "show-kanji-definition-flashcards"));
    g_simple_action_set_enabled (action, enabled);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "show-definition-kanji-flashcards"));
    g_simple_action_set_enabled (action, enabled);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "show-kanji-furigana-flashcards"));
    g_simple_action_set_enabled (action, enabled);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "show-furigana-kanji-flashcards"));
    g_simple_action_set_enabled (action, enabled);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "show-definition-furigana-flashcards"));
    g_simple_action_set_enabled (action, enabled);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "show-furigana-definition-flashcards"));
    g_simple_action_set_enabled (action, enabled);
}


void
gw_vocabularywindow_map_actions (GActionMap *map, GwVocabularyWindow *window)
{
    //Sanity checks
    g_return_if_fail (map != NULL);
    g_return_if_fail (window != NULL);

    static GActionEntry entries[] = {
      { "toggle-menubar-show", gw_vocabularywindow_menubar_show_toggled_cb, NULL, "false", NULL},
      { "toggle-toolbar-show", gw_vocabularywindow_toolbar_show_toggled_cb, NULL, "false", NULL},

      { "new-list", gw_vocabularywindow_new_list_cb, NULL, NULL, NULL},
      { "new-word", gw_vocabularywindow_new_word_cb, NULL, NULL, NULL},

      { "remove-list", gw_vocabularywindow_remove_list_cb, NULL, NULL, NULL},
      { "remove-word", gw_vocabularywindow_remove_word_cb, NULL, NULL, NULL},

      { "save-list", gw_vocabularywindow_save_cb, NULL, NULL, NULL },
      { "revert-list", gw_vocabularywindow_revert_cb, NULL, NULL, NULL },
      { "import", gw_vocabularywindow_import_cb, NULL, NULL, NULL },
      { "export", gw_vocabularywindow_export_cb, NULL, NULL, NULL },

      { "close", gw_vocabularywindow_close_cb, NULL, NULL, NULL },

      { "cut", gw_vocabularywindow_cut_cb, NULL, NULL, NULL},
      { "copy", gw_vocabularywindow_copy_cb, NULL, NULL, NULL},
      { "paste", gw_vocabularywindow_paste_cb, NULL, NULL, NULL},
      { "delete", gw_vocabularywindow_delete_cb, NULL, NULL, NULL},

      { "toggle-editable", gw_vocabularywindow_editable_toggled_cb, NULL, "false", NULL},

      { "toggle-position-column-show", gw_vocabularywindow_position_column_toggled_cb, NULL, "false", NULL},
      { "toggle-timestamp-column-show", gw_vocabularywindow_timestamp_column_toggled_cb, NULL, "false", NULL},
      { "toggle-score-column-show", gw_vocabularywindow_score_column_toggled_cb, NULL, "false", NULL},

      { "show-kanji-definition-flashcards", gw_vocabularywindow_kanji_definition_flashcards_cb, NULL, NULL, NULL},
      { "show-definition-kanji-flashcards", gw_vocabularywindow_definition_kanji_flashcards_cb, NULL, NULL, NULL},
      { "show-furigana-definition-flashcards", gw_vocabularywindow_furigana_definition_flashcards_cb, NULL, NULL, NULL},
      { "show-definition-furigana-flashcards", gw_vocabularywindow_definition_furigana_flashcards_cb, NULL, NULL, NULL},
      { "show-kanji-furigana-flashcards", gw_vocabularywindow_kanji_furigana_flashcards_cb, NULL, NULL, NULL},
      { "show-furigana-kanji-flashcards", gw_vocabularywindow_furigana_kanji_flashcards_cb, NULL, NULL, NULL},

      { "toggle-shuffle", gw_vocabularywindow_shuffle_toggled_cb, NULL, "false", NULL},
      { "toggle-trim", gw_vocabularywindow_trim_toggled_cb, NULL, "false", NULL},
      { "toggle-track-results", gw_vocabularywindow_track_results_toggled_cb, NULL, "false", NULL}
    };
    g_action_map_add_action_entries (map, entries, G_N_ELEMENTS (entries), window);
}


GMenuModel*
gw_vocabularywindow_get_flashcard_menumodel (GwVocabularyWindow *window)
{
    GwVocabularyWindowPrivate *priv;
    GMenuModel *menumodel;
    GtkBuilder *builder;

    priv = window->priv;
    menumodel = NULL;
    builder = gtk_builder_new ();
    if (builder == NULL) goto errored;

    if (priv->flashcard_menumodel == NULL)
    {
      gw_application_load_xml (builder, "vocabularywindow-menumodel-flashcards.ui");
      menumodel = G_MENU_MODEL (gtk_builder_get_object (builder, "menu"));
      priv->flashcard_menumodel = menumodel;
    }

errored:
    if (builder != NULL) g_object_unref (builder); builder = NULL;

    return priv->flashcard_menumodel;
}


void
gw_vocabularywindow_initialize_menu_links (GwVocabularyWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GMenuModel *menumodel;
    GMenuModel *link;

    //Initializations
    menumodel = gw_window_get_menumodel (GW_WINDOW (window));
    g_return_if_fail (menumodel != NULL);
    link = gw_vocabularywindow_get_flashcard_menumodel (window);

    gw_menumodel_set_links (menumodel, "flashcards-list-link", gettext("_Study"), G_MENU_LINK_SUBMENU, link);
}


static void
gw_vocabularywindow_initialize_toolbar (GwVocabularyWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkIconTheme *theme;
    GtkToolbar *toolbar;
    GtkToolItem *item;

    priv = window->priv;
    toolbar = priv->primary_toolbar;
    theme = gtk_icon_theme_get_default ();

    item = gtk_tool_button_new_from_stock (GTK_STOCK_SAVE);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.save-all");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));

/*
    item = gtk_tool_button_new_from_stock (GTK_STOCK_CUT);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.cut");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new_from_stock (GTK_STOCK_COPY);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.copy");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new_from_stock (GTK_STOCK_PASTE);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.paste");
    gtk_widget_show (GTK_WIDGET (item));
*/

///////////////////

    toolbar = priv->list_toolbar;

    item = gtk_tool_button_new_from_stock (GTK_STOCK_ADD);
    if (gtk_icon_theme_has_icon (theme, "list-add-symbolic"))
    {
      gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "list-add-symbolic");
      gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), NULL);
    }
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.new-list");
    gtk_widget_show (GTK_WIDGET (item));
    
    item = gtk_tool_button_new_from_stock (GTK_STOCK_REMOVE);
    if (gtk_icon_theme_has_icon (theme, "list-remove-symbolic"))
    {
      gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "list-remove-symbolic");
      gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), NULL);
    }
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.remove-list");
    gtk_widget_show (GTK_WIDGET (item));

////////////////

    toolbar = priv->word_toolbar;

    item = gtk_tool_button_new_from_stock (GTK_STOCK_ADD);
    if (gtk_icon_theme_has_icon (theme, "list-add-symbolic"))
    {
      gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "list-add-symbolic");
      gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), NULL);
    }
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.new-word");
    gtk_widget_show (GTK_WIDGET (item));
    
    item = gtk_tool_button_new_from_stock (GTK_STOCK_REMOVE);
    if (gtk_icon_theme_has_icon (theme, "list-remove-symbolic"))
    {
      gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "list-remove-symbolic");
      gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), NULL);
    }
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.remove-word");
    gtk_widget_show (GTK_WIDGET (item));

/*
    item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));
*/

    item = gtk_tool_button_new_from_stock (GTK_STOCK_REVERT_TO_SAVED);
    if (gtk_icon_theme_has_icon (theme, "edit-undo-symbolic"))
    {
      gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "edit-undo-symbolic");
      gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), NULL);
    }
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.revert-list");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new_from_stock (GTK_STOCK_SAVE);
    if (gtk_icon_theme_has_icon (theme, "document-save-symbolic"))
    {
      gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "document-save-symbolic");
      gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), NULL);
    }
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.save-list");
    gtk_widget_show (GTK_WIDGET (item));

/*
    item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));
*/

    item = gtk_toggle_tool_button_new_from_stock (GTK_STOCK_EDIT);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.toggle-editable");
    gtk_widget_show (GTK_WIDGET (item));
}


void
gw_vocabularywindow_new_list (GwVocabularyWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *liststore, *wordstore;
    GtkTreeIter iter;
    GtkTreePath *path;

    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);

    gw_vocabularyliststore_new_list (GW_VOCABULARYLISTSTORE (liststore), &iter);
    path = gtk_tree_model_get_path (GTK_TREE_MODEL (liststore), &iter);

    gtk_tree_view_set_cursor (priv->list_treeview, path, NULL, TRUE);
    wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter);
    gtk_tree_view_set_model (priv->word_treeview, GTK_TREE_MODEL (wordstore));
    gtk_tree_view_set_search_column (priv->word_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);
    gw_vocabularywindow_update_flashcard_menu_sensitivities (window);
    gw_vocabularywindow_show_vocabulary_list (window, TRUE);

    //Cleanup
    gtk_tree_path_free (path); path = NULL;
}


void
gw_vocabularywindow_new_word (GwVocabularyWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkWindow *avw;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean valid;
    gchar *list;

    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    model = gtk_tree_view_get_model (priv->list_treeview);
    if (model == NULL) return;
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    valid = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (valid)
    {
      avw = gw_addvocabularywindow_new (GTK_APPLICATION (application));
      list = gw_vocabularyliststore_get_name_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
      if (list != NULL)
      {
        gw_addvocabularywindow_set_list (GW_ADDVOCABULARYWINDOW (avw), list);
        gw_addvocabularywindow_set_focus (GW_ADDVOCABULARYWINDOW (avw), GW_ADDVOCABULARYWINDOW_FOCUS_KANJI);
        g_free (list);
      }
      gtk_window_set_transient_for (avw, GTK_WINDOW (window));
      g_signal_connect (G_OBJECT (avw), "word-added", G_CALLBACK (gw_vocabularywindow_select_new_word_from_dialog_cb), window);
      gtk_widget_show (GTK_WIDGET (avw));
    }
}


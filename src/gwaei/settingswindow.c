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
//! @file settingswindow.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/settingswindow-private.h>


enum {
    TARGET_DICTIONARY_ROW,
};

//List model
static GtkTargetEntry dictionary_row_dest_targets[] = { //Will accept drops
    { "dictionaries",    GTK_TARGET_SAME_WIDGET, TARGET_DICTIONARY_ROW }
};
static guint n_list_row_dest_targets = G_N_ELEMENTS (dictionary_row_dest_targets);

static GtkTargetEntry dictionary_row_source_targets[] = { //Data given to drags
    { "dictionaries",    GTK_TARGET_SAME_WIDGET, TARGET_DICTIONARY_ROW }
};
static guint n_list_row_source_targets = G_N_ELEMENTS (dictionary_row_source_targets);


static void gw_settingswindow_init_styles (GwSettingsWindow*);
static void gw_settingswindow_init_dictionary_treeview (GwSettingsWindow*);
static void gw_settingswindow_attach_signals (GwSettingsWindow*);
static void gw_settingswindow_remove_signals (GwSettingsWindow*);

G_DEFINE_TYPE (GwSettingsWindow, gw_settingswindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_settingswindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwSettingsWindow *window;

    //Initializations
    window = GW_SETTINGSWINDOW (g_object_new (GW_TYPE_SETTINGSWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "settingswindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


static void 
gw_settingswindow_init (GwSettingsWindow *window)
{
    window->priv = GW_SETTINGSWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwSettingsWindowPrivate));
}


static void 
gw_settingswindow_finalize (GObject *object)
{
    GwSettingsWindow *window;
    GwApplication *application;

    window = GW_SETTINGSWINDOW (object);
    application = gw_window_get_application (GW_WINDOW (window));

    if (g_main_current_source () != NULL) gw_application_unblock_searches (application);

    G_OBJECT_CLASS (gw_settingswindow_parent_class)->finalize (object);
}


static void 
gw_settingswindow_constructed (GObject *object)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *dictionarystore;
    LwDictInfoList *dictinfolist;
    GtkAccelGroup *accelgroup;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_settingswindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_SETTINGSWINDOW (object);
    priv = window->priv;
    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarystore = gw_application_get_dictionarystore (application);
    dictinfolist = gw_dictionarystore_get_dictinfolist (GW_DICTIONARYSTORE (dictionarystore));

    priv->manage_dictionaries_treeview = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "dictionary_treeview"));
    priv->notebook = GTK_NOTEBOOK (gw_window_get_object (GW_WINDOW (window), "settings_notebook"));
    priv->close_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "close_button"));
    priv->spellcheck_checkbutton = GTK_TOGGLE_BUTTON (gw_window_get_object (GW_WINDOW (window), "spellcheck_checkbutton"));
    priv->please_install_dictionary_hbox = GTK_BOX (gw_window_get_object (GW_WINDOW (window), "please_install_dictionary_hbox"));
    priv->custom_font_fontbutton = GTK_FONT_BUTTON (gw_window_get_object (GW_WINDOW (window), "custom_font_fontbutton"));

    priv->match_foreground = GTK_COLOR_BUTTON (gw_window_get_object (GW_WINDOW (window), "match_foreground_colorbutton"));
    priv->match_background = GTK_COLOR_BUTTON (gw_window_get_object (GW_WINDOW (window), "match_background_colorbutton"));
    priv->comment_foreground = GTK_COLOR_BUTTON (gw_window_get_object (GW_WINDOW (window), "comment_foreground_colorbutton"));
    priv->header_foreground = GTK_COLOR_BUTTON (gw_window_get_object (GW_WINDOW (window), "header_foreground_colorbutton"));
    priv->header_background = GTK_COLOR_BUTTON (gw_window_get_object (GW_WINDOW (window), "header_background_colorbutton"));
    priv->system_document_font_hbox = GTK_BOX (gw_window_get_object (GW_WINDOW (window), "system_document_font_hbox"));
    priv->system_font_checkbutton = GTK_CHECK_BUTTON (gw_window_get_object (GW_WINDOW (window), "system_font_checkbutton"));
    priv->search_as_you_type_checkbutton = GTK_CHECK_BUTTON (gw_window_get_object (GW_WINDOW (window), "search_as_you_type_checkbutton"));
    priv->romaji_to_kana_combobox = GTK_COMBO_BOX (gw_window_get_object (GW_WINDOW (window), "romaji_to_kana_combobox"));
    priv->hiragana_to_katakana_checkbutton = GTK_CHECK_BUTTON (gw_window_get_object (GW_WINDOW (window), "hiragana_to_katakana_checkbutton"));
    priv->katakana_to_hiragana_checkbutton = GTK_CHECK_BUTTON (gw_window_get_object (GW_WINDOW (window), "katakana_to_hiragana_checkbutton"));
    priv->remove_dictionary_toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "remove_dictionary_toolbutton"));

    gtk_window_set_title (GTK_WINDOW (window), gettext("gWaei Settings"));
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);

    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gw_settingswindow_remove_signals), NULL);

    if (g_main_current_source () != NULL) gw_application_block_searches (application);


    gw_settingswindow_init_styles (window);
    gw_settingswindow_init_dictionary_treeview (window);

    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (dictinfolist)) == 0)
      gtk_notebook_set_current_page (priv->notebook, 1);
    gw_settingswindow_check_for_dictionaries (window);

    #ifdef WITH_HUNSPELL
    gtk_widget_show (GTK_WIDGET (priv->spellcheck_checkbutton));
    #else
    gtk_widget_hide (GTK_WIDGET (priv->spellcheck_checkbutton));
    #endif

    gw_settingswindow_attach_signals (window);

    gtk_widget_add_accelerator (GTK_WIDGET (priv->close_button), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (priv->close_button), "activate", 
      accelgroup, (GDK_KEY_Escape), 0, GTK_ACCEL_VISIBLE);

    gw_window_unload_xml (GW_WINDOW (window));
}


static void
gw_settingswindow_class_init (GwSettingsWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_settingswindow_constructed;
  object_class->finalize = gw_settingswindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwSettingsWindowPrivate));
}


static void 
gw_settingswindow_attach_signals (GwSettingsWindow *window)
{
    //Declarations
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    int i;

    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    for (i = 0; i < TOTAL_GW_SETTINGSWINDOW_SIGNALIDS; i++)
      priv->signalids[i] = 0;

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_ROMAJI_KANA] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_ROMAN_KANA,
        gw_settingswindow_sync_romaji_kana_conv_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HIRA_KATA] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_HIRA_KATA,
        gw_settingswindow_sync_hira_kata_conv_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_KATA_HIRA] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_KATA_HIRA,
        gw_settingswindow_sync_kata_hira_conv_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_SPELLCHECK] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK,
        gw_settingswindow_sync_spellcheck_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_USE_GLOBAL_DOCUMENT_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_USE_GLOBAL_FONT,
        gw_settingswindow_sync_use_global_document_font_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_GLOBAL_DOCUMENT_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_GNOME_INTERFACE,
        LW_KEY_DOCUMENT_FONT_NAME,
        gw_settingswindow_sync_global_document_font_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_CUSTOM_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_CUSTOM_FONT,
        gw_settingswindow_sync_custom_font_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_SEARCH_AS_YOU_TYPE] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SEARCH_AS_YOU_TYPE,
        gw_settingswindow_sync_search_as_you_type_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_FG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_MATCH_FG,
        gw_settingswindow_sync_swatch_color_cb,
        priv->match_foreground
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_BG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_MATCH_BG,
        gw_settingswindow_sync_swatch_color_cb,
        priv->match_background
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_COMMENT_FG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_COMMENT_FG,
        gw_settingswindow_sync_swatch_color_cb,
        priv->comment_foreground
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_FG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_HEADER_FG,
        gw_settingswindow_sync_swatch_color_cb,
        priv->header_foreground
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_BG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_HEADER_BG,
        gw_settingswindow_sync_swatch_color_cb,
        priv->header_background
    );
}


static void 
gw_settingswindow_remove_signals (GwSettingsWindow *window)
{
    //Declarations
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    int i;

    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_ROMAJI_KANA]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HIRA_KATA]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_KATA_HIRA]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_SPELLCHECK]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_FONT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_USE_GLOBAL_DOCUMENT_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_GNOME_INTERFACE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_GLOBAL_DOCUMENT_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_FONT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_CUSTOM_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_SEARCH_AS_YOU_TYPE]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_BG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_COMMENT_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_BG]
    );

    for (i = 0; i < TOTAL_GW_SETTINGSWINDOW_SIGNALIDS; i++)
      priv->signalids[i] = 0;
}


//!
//! @brief Sets the text in the source gtkentry for the appropriate dictionary
//!
//! @param widget Pointer to a GtkEntry to set the text of
//! @param value The constant string to use as the source for the text
//!
void 
gw_settings_set_dictionary_source (GtkWidget *widget, const char* value)
{
    if (widget != NULL && value != NULL)
    {
      gtk_entry_set_text (GTK_ENTRY (widget), value);
    }
}


static void
gw_settingswindow_init_styles (GwSettingsWindow *window)
{
    //Declarations
    GtkStyleContext *context;
    GtkWidget *widget;

    //Vocabulary list pane
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "dictionary_scrolledwindow"));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
    gtk_widget_reset_style (widget);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "dictionary_toolbar"));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_class (context, "inline-toolbar");
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
    gtk_widget_reset_style (widget);

    //Vocabulary listitem pane
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "plugin_scrolledwindow"));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
    gtk_widget_reset_style (widget);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "plugin_toolbar"));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_class (context, "inline-toolbar");
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
    gtk_widget_reset_style (widget);
}


static void 
gw_settingswindow_init_dictionary_treeview (GwSettingsWindow *window)
{
    //Declarations
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *dictionarystore;
    GtkTreeView *view;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;

    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarystore = gw_application_get_dictionarystore (application);
    view = priv->manage_dictionaries_treeview;
    selection = gtk_tree_view_get_selection (view);

    gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (dictionarystore));

    //Create the columns and renderer for each column
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, " ");
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, "icon-name", GW_DICTIONARYSTORE_COLUMN_IMAGE, NULL);
    gtk_tree_view_append_column (view, column);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new_with_attributes ("#", renderer, "text", GW_DICTIONARYSTORE_COLUMN_POSITION, NULL);
    gtk_tree_view_append_column (view, column);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new_with_attributes (gettext("Name"), renderer, "text", GW_DICTIONARYSTORE_COLUMN_LONG_NAME, NULL);
    gtk_tree_view_column_set_min_width (column, 100);
    gtk_tree_view_append_column (view, column);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new_with_attributes (gettext("Engine"), renderer, "text", GW_DICTIONARYSTORE_COLUMN_ENGINE, NULL);
    gtk_tree_view_append_column (view, column);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new_with_attributes (gettext("Shortcut"), renderer, "text", GW_DICTIONARYSTORE_COLUMN_SHORTCUT, NULL);
    gtk_tree_view_append_column (view, column);

    gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

    gtk_drag_source_set (
        GTK_WIDGET (priv->manage_dictionaries_treeview), 
        GDK_BUTTON1_MASK,
        dictionary_row_source_targets,
        n_list_row_source_targets,
        GDK_ACTION_MOVE
    );

    gtk_drag_dest_set (
        GTK_WIDGET (priv->manage_dictionaries_treeview),
        GTK_DEST_DEFAULT_ALL,
        dictionary_row_dest_targets,
        n_list_row_dest_targets,
        GDK_ACTION_MOVE
    );
}


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
void 
gw_settingswindow_check_for_dictionaries (GwSettingsWindow *window)
{
    //Declarations
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *dictionarystore;
    LwDictInfoList *dictinfolist;
    GtkWidget *message;

    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarystore = gw_application_get_dictionarystore (application);
    dictinfolist = gw_dictionarystore_get_dictinfolist (GW_DICTIONARYSTORE (dictionarystore));
    message = GTK_WIDGET (priv->please_install_dictionary_hbox);

    //Set the show state of the dictionaries required message
    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (dictinfolist)) > 0)
      gtk_widget_hide (message);
    else
      gtk_widget_show (message);
}


G_MODULE_EXPORT void
gw_settingswindow_drag_begin_cb (
  GtkWidget      *widget,
  GdkDragContext *context,
  gpointer        data)
{
  cairo_surface_t *surface;
  GtkTreeView *view;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreePath *path;
  GList *selectedlist;

  view = GTK_TREE_VIEW (widget);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
  selectedlist = gtk_tree_selection_get_selected_rows (selection, &model);
  path = (GtkTreePath*) selectedlist->data;
  surface = gtk_tree_view_create_row_drag_icon (view, path);

  gtk_drag_set_icon_surface (context, surface);

  cairo_surface_destroy (surface);
  g_list_foreach (selectedlist, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (selectedlist);
}


static gboolean
gw_settingswindow_dictionary_drag_reorder (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer user_data)
{
    //Declarations
    GwSettingsWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    GtkTreeViewDropPosition drop_position;
    GtkTreePath *path;
    GtkTreeView *view;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter, position;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_SETTINGSWINDOW));
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    g_return_val_if_fail (window != NULL, FALSE);
    view = GTK_TREE_VIEW (widget);
    selection = gtk_tree_view_get_selection (view);
    model = gtk_tree_view_get_model (view);

    gtk_tree_view_get_dest_row_at_pos (view, x, y, &path, &drop_position);
    if (path == NULL) return FALSE;
    gtk_tree_model_get_iter (model, &position, path);
    gtk_tree_path_free (path); path = NULL;

    if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) 
      drop_position = GTK_TREE_VIEW_DROP_BEFORE;
    else if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_AFTER) 
      drop_position = GTK_TREE_VIEW_DROP_AFTER;

    gtk_tree_selection_get_selected (selection, &model, &iter);

    if (drop_position == GTK_TREE_VIEW_DROP_BEFORE) 
      gtk_list_store_move_before (GTK_LIST_STORE (model), &iter, &position);
    else if (drop_position == GTK_TREE_VIEW_DROP_AFTER) 
      gtk_list_store_move_after (GTK_LIST_STORE (model), &iter, &position);

    gw_dictionarystore_save_order (GW_DICTIONARYSTORE (model), preferences);

    return TRUE;
}


G_MODULE_EXPORT gboolean
gw_settingswindow_dictionary_drag_drop_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer user_data)
{
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GtkTreeView *source;
    gboolean success;

    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_SETTINGSWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    source = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));
    success = FALSE;

    if (source == priv->manage_dictionaries_treeview)
      gw_settingswindow_dictionary_drag_reorder (widget, context, x, y, time, user_data);

    gtk_drag_finish (context, success, FALSE, time);

    return success;
}


G_MODULE_EXPORT void
gw_settingswindow_dictionary_drag_motion_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GtkTreeView *view;
    GtkTreeViewDropPosition drop_position;
    GtkTreePath *path;
    GtkTreeView *source;

    //Initializations
    window = GW_SETTINGSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_SETTINGSWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    view = GTK_TREE_VIEW (widget);
    source = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));

    if (source == priv->manage_dictionaries_treeview)
    {
      gtk_tree_view_get_dest_row_at_pos (view, x, y, &path, &drop_position);

      if (path != NULL)
      {
        if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) 
          drop_position = GTK_TREE_VIEW_DROP_BEFORE;
        else if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_AFTER) 
          drop_position = GTK_TREE_VIEW_DROP_AFTER;

        gtk_tree_view_set_drag_dest_row (view, path, drop_position);
      }
    }

    if (path != NULL) gtk_tree_path_free (path); path = NULL;
}

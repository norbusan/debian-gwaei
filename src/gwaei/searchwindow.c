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
//! @file searchwindow.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/searchwindow-private.h>


//Static declarations
static void gw_searchwindow_attach_signals (GwSearchWindow*);
static void gw_searchwindow_remove_signals (GwSearchWindow*);

static GtkCssProvider* gw_searchwindowclass_get_tablabel_style_provider (GwSearchWindowClass*);
static GtkInfoBar* _construct_infobar ();

G_DEFINE_TYPE (GwSearchWindow, gw_searchwindow, GW_TYPE_WINDOW)


//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_searchwindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (g_object_new (GW_TYPE_SEARCHWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "searchwindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


static void 
gw_searchwindow_init (GwSearchWindow *window)
{
    window->priv = GW_SEARCHWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwSearchWindowPrivate));

    GwSearchWindowPrivate *priv;
    priv = window->priv;

    priv->feedback_status = LW_SEARCHSTATUS_IDLE;
}


static void 
gw_searchwindow_finalize (GObject *object)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;

    window = GW_SEARCHWINDOW (object);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));

    if (gw_application_get_last_focused_searchwindow (application) == window)
      gw_application_set_last_focused_searchwindow (application, NULL);

#ifdef WITH_HUNSPELL
    if (priv->spellcheck) g_object_unref (priv->spellcheck); priv->spellcheck = NULL;
#endif
    if (priv->history) g_object_unref (priv->history); priv->history = NULL;
    if (priv->keep_searching_query) g_free (priv->keep_searching_query); priv->keep_searching_query = NULL;

    gw_window_save_size (GW_WINDOW (window));

    G_OBJECT_CLASS (gw_searchwindow_parent_class)->finalize (object);
}


static void
_activate_toggle (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       data)
{
  GVariant *state;

  state = g_action_get_state (G_ACTION (action));

  g_action_change_state (G_ACTION (action), g_variant_new_boolean (!g_variant_get_boolean (state)));

  g_variant_unref (state);
}


static void
gw_searchwindow_initialize_notebook (GwSearchWindow *window)
{
    //Sanity check
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    GtkNotebook *notebook;
    GtkWidget *widget;
    GtkWidget *image;
    GtkCssProvider *provider;
    GtkStyleContext *context;
    GwSearchWindowClass *klass;

    //Initializations
    priv = window->priv;
    klass = GW_SEARCHWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    notebook = priv->notebook;
    widget = gtk_button_new ();
    gtk_widget_show (widget);
    image = gtk_image_new_from_icon_name ("tab-new", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image);
    provider = gw_searchwindowclass_get_tablabel_style_provider (klass);
    gtk_button_set_relief (GTK_BUTTON (widget), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click (GTK_BUTTON (widget), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (widget), 1);
    gtk_misc_set_padding (GTK_MISC (image), 4, 0);
    gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.5);
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (widget), "win.new-tab");

    gtk_container_add (GTK_CONTAINER (widget), image);
    gtk_notebook_set_action_widget (notebook, widget, GTK_PACK_START);
}


void
gw_searchwindow_map_actions (GActionMap *map, GwSearchWindow *window)
{
    //Sanity checks
    g_return_if_fail (map != NULL);
    g_return_if_fail (window != NULL);

    static GActionEntry entries[] = {
      { "cut", gw_searchwindow_cut_cb, NULL, NULL, NULL },
      { "copy", gw_searchwindow_copy_cb, NULL, NULL, NULL },
      { "paste", gw_searchwindow_paste_cb, NULL, NULL, NULL },
      { "select-all", gw_searchwindow_select_all_cb, NULL, NULL, NULL },

      { "new-tab", gw_searchwindow_new_tab_cb, NULL, NULL, NULL },
      { "next-tab", gw_searchwindow_next_tab_cb, NULL, NULL, NULL },
      { "previous-tab", gw_searchwindow_previous_tab_cb, NULL, NULL, NULL },
      { "toggle-menubar-show", gw_searchwindow_menubar_show_toggled_cb, NULL, "false", NULL},
      { "toggle-toolbar-show", gw_searchwindow_toolbar_show_toggled_cb, NULL, "false", NULL},
      { "toggle-tabbar-show", gw_searchwindow_tabbar_show_toggled_cb, NULL, "false", NULL},
      { "toggle-statusbar-show", gw_searchwindow_statusbar_show_toggled_cb, NULL, "false", NULL },
      { "toggle-kanjipad-show", _activate_toggle, NULL, "false", gw_searchwindow_toggle_kanjipadwindow_cb },
      { "toggle-radicals-show", _activate_toggle, NULL, "false", gw_searchwindow_toggle_radicalswindow_cb },
      { "zoom-100", gw_searchwindow_zoom_100_cb, NULL, NULL, NULL },
      { "zoom-in", gw_searchwindow_zoom_in_cb, NULL, NULL, NULL },

      { "print", gw_print_cb, NULL, NULL, NULL },
      { "print-preview", gw_print_preview_cb, NULL, NULL, NULL },

      { "save", gw_searchwindow_save_cb, NULL, NULL, NULL },
      { "save-as", gw_searchwindow_save_as_cb, NULL, NULL, NULL },

      { "zoom-out", gw_searchwindow_zoom_out_cb, NULL, NULL, NULL },
      { "set-dictionary", gw_searchwindow_set_dictionary_cb, "s", "''", NULL },

      { "next-dictionary", gw_searchwindow_previous_dictionary_cb, NULL, NULL, NULL },
      { "previous-dictionary", gw_searchwindow_next_dictionary_cb, NULL, NULL, NULL },

      { "insert-unknown-character", gw_searchwindow_insert_unknown_character_cb, NULL, NULL, NULL },
      { "insert-word-edge-character", gw_searchwindow_insert_word_edge_cb, NULL, NULL, NULL },
      { "insert-not-word-edge-character", gw_searchwindow_insert_not_word_edge_cb, NULL, NULL, NULL },
      { "insert-and-character", gw_searchwindow_insert_and_cb, NULL, NULL, NULL },
      { "insert-or-character", gw_searchwindow_insert_or_cb, NULL, NULL, NULL },

      { "go-back", gw_searchwindow_go_back_cb, NULL, NULL, NULL },
      { "go-forward", gw_searchwindow_go_forward_cb, NULL, NULL, NULL },

      { "go-back-index", gw_searchwindow_go_back_index_cb, "s", NULL, NULL },
      { "go-forward-index", gw_searchwindow_go_forward_index_cb, "s", NULL, NULL },

      { "add-word", gw_searchwindow_add_vocabulary_word_cb, NULL, NULL, NULL },
      { "clear", gw_searchwindow_clear_search_cb, NULL, NULL, NULL },
      { "close", gw_searchwindow_close_cb, NULL, NULL, NULL }
    };
    g_action_map_add_action_entries (map, entries, G_N_ELEMENTS (entries), window);
}

static void 
gw_searchwindow_constructed (GObject *object)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_searchwindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_SEARCHWINDOW (object);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));

    gw_searchwindow_map_actions (G_ACTION_MAP (window), window);
    gw_window_load_menubar (GW_WINDOW (window), "searchwindow");

    //Set up the gtkbuilder links
    priv->primary_toolbar = GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "primary_toolbar"));
    priv->search_toolbar = GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "search_toolbar"));
    priv->notebook = GTK_NOTEBOOK (gw_window_get_object (GW_WINDOW (window), "notebook"));
    priv->statusbar = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "statusbar"));
    priv->statusbar_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "statusbar_label"));
    priv->statusbar_progressbar = GTK_PROGRESS_BAR (gw_window_get_object (GW_WINDOW (window), "statusbar_progressbar"));

    priv->history = gw_history_new (20);

    gw_searchwindow_initialize_toolbar (window);
    gw_searchwindow_initialize_search_toolbar (window);
    gw_searchwindow_initialize_dictionary_combobox (window);
    gw_searchwindow_initialize_notebook (window);
    gw_searchwindow_initialize_menu_links (window);

    //Set up the gtk window
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_set_default_size (GTK_WINDOW (window), 620, 500);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gw_window_set_is_important (GW_WINDOW (window), TRUE);
    gw_window_load_size (GW_WINDOW (window));

    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
    gw_searchwindow_set_dictionary (window, 0);
    gw_searchwindow_guarantee_first_tab (window);

    gw_searchwindow_attach_signals (window);
    gw_searchwindow_sync_history (window);
    gw_application_set_last_focused_searchwindow (application, window);;
    gw_window_unload_xml (GW_WINDOW (window));
}


static void
gw_searchwindow_class_init (GwSearchWindowClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = gw_searchwindow_constructed;
    object_class->finalize = gw_searchwindow_finalize;

    g_type_class_add_private (object_class, sizeof (GwSearchWindowPrivate));

    klass->tablabel_sizegroup = NULL;
    klass->tablabel_cssprovider = NULL;

    klass->signalid[GW_SEARCHWINDOW_CLASS_SIGNALID_WORD_ADDED] = g_signal_new (
        "word-added",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
        G_STRUCT_OFFSET (GwSearchWindowClass, word_added),
        NULL, NULL,
        g_cclosure_marshal_VOID__POINTER,
        G_TYPE_NONE, 
        1, G_TYPE_POINTER
    );
}


//!
//! @brief Updates the progress information based on the LwSearch info
//! @param search A LwSearch pointer to gleam information from.
//! @returns Currently always returns TRUE
//!
gboolean 
gw_searchwindow_update_progress_feedback_timeout (GwSearchWindow *window)
{
    //Sanity checks
    if (gtk_widget_get_visible (GTK_WIDGET (window)) == FALSE) return TRUE;

    //Declarations
    GwSearchWindowPrivate *priv;
    LwSearch *search;
    gint index;

    //Initializations
    priv = window->priv;
    index = gw_searchwindow_get_current_tab_index (window);
    search = gw_searchwindow_get_searchitem_by_index (window, index);

    if (search != NULL) 
    {
      lw_search_lock (search);
        if (
            search->status != LW_SEARCHSTATUS_IDLE &&
            (search != priv->feedback_item ||
             search->current != priv->feedback ||
             search->status != priv->feedback_status       )
            )
        {
          gw_searchwindow_set_search_progressbar_by_searchitem (window, search);
          gw_searchwindow_set_total_results_label_by_searchitem (window, search);
          gw_searchwindow_set_title_by_searchitem (window, search);

          priv->feedback_item = search;
          priv->feedback = search->current;
          priv->feedback_status = search->status;
        }
      lw_search_unlock (search);
    }

    return TRUE;
}


gboolean 
gw_searchwindow_append_result_timeout (GwSearchWindow *window)
{
    //Sanity check
    if (gtk_widget_get_visible (GTK_WIDGET (window)) == FALSE) return TRUE;

    //Declarations
    GwSearchWindowPrivate *priv;
    LwSearch *search;
    gint index;
    gint chunk;
    gint max_chunk;
    LwSearchStatus status;
    gboolean has_results;
    gint total_results;

    //Initializations
    priv = window->priv;
    index = gw_searchwindow_get_current_tab_index (window);
    search = gw_searchwindow_get_searchitem_by_index (window, index);

    if (search != NULL) 
    {
      status = lw_search_get_status (search);
      total_results = lw_search_get_total_results (search);
      has_results = lw_search_has_results (search);
      chunk = 0;
      max_chunk = 10;
      
      if (status == LW_SEARCHSTATUS_FINISHING && total_results == 0)
      {
          gw_searchwindow_display_no_results_found_page (window, search);
      }
      else if (has_results)
      {
        while (has_results && chunk++ < max_chunk)
        {
          gw_searchwindow_append_result (window, search);
          has_results = lw_search_has_results (search);
        }
      }
    }

    search = priv->mouse_item;

    if (search != NULL)
    {
      has_results = lw_search_has_results (search);
      
      if (search != NULL && has_results);
      {
        gw_searchwindow_append_kanjidict_tooltip_result (window, search);
      }
    }

    return TRUE;
}


void 
gw_searchwindow_entry_set_text (GwSearchWindow *window, const gchar *TEXT)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;

    //Initializations
    priv = window->priv;

    if (TEXT != NULL) gtk_entry_set_text (priv->entry, TEXT);
}


void 
gw_searchwindow_entry_insert_text (GwSearchWindow *window, const gchar *TEXT)
{
    //Sanity checks
    g_return_if_fail (window != NULL);
    if (TEXT == NULL) return;

    //Declarations
    GwSearchWindowPrivate *priv;
    gint start, end;

    //Initializations
    priv = window->priv;

    gtk_editable_get_selection_bounds (GTK_EDITABLE (priv->entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (priv->entry), start, end);

    gtk_editable_insert_text (GTK_EDITABLE (priv->entry), TEXT, -1, &start);
    gtk_editable_set_position (GTK_EDITABLE (priv->entry), start + strlen(TEXT));
}



//!
//! @brief Sets the query text of the program using the informtion from the searchitem
//!
//! @param search a LwSearch argument.
//!
void 
gw_searchwindow_set_entry_text_by_searchitem (GwSearchWindow* window, LwSearch *search)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    LwQuery *query;
    const gchar *text;

    //Initializations
    priv = window->priv;

    if (search == NULL || search->query == NULL)
    {
      text = "";
    }
    else
    {
      query = search->query;
      text = lw_query_get_text (query);
      if (text == NULL) text = "";
    }

    gtk_entry_set_text (priv->entry, text);
    gtk_editable_set_position (GTK_EDITABLE (priv->entry), -1);
}


//!
//! @brief Sets the main window title text of the program using the informtion from the searchitem
//!
//! @param search a LwSearch argument.
//!
gchar* 
gw_searchwindow_get_title_by_searchitem (GwSearchWindow* window, LwSearch *search)
{
    //Sanity checks
    g_return_val_if_fail (window != NULL, NULL);

    //Declarations
    GwApplication *application;
    gchar *title;
    const gchar *program_name;
    gint num_relevant, num_total;
    LwQuery *query;
    const gchar *text;

    //Declarations
    application = gw_window_get_application (GW_WINDOW (window));
    query = NULL;

    if (search == NULL || search->query == NULL)
    {
      //Initializations
      title = g_strdup (gw_application_get_program_name (application));
    }
    else
    {
      //Initializations
      query = search->query;
      program_name = gw_application_get_program_name (application);
      text = lw_query_get_text (query);
      num_relevant = lw_search_get_total_relevant_results (search);
      num_total = lw_search_get_total_results (search);
      title = g_strdup_printf ("%s [%d/%d] - %s", text, num_relevant, num_total, program_name);
    }

    return title;
}

//!
//! @brief Sets the main window title text of the program using the informtion from the searchitem
//!
//! @param search a LwSearch argument.
//!
void 
gw_searchwindow_set_title_by_searchitem (GwSearchWindow* window, LwSearch *search)
{
    //Declarations
    char *title;

    //Initializations
    title = gw_searchwindow_get_title_by_searchitem (window, search);

    gtk_window_set_title (GTK_WINDOW (window), title);

    //Cleanup
    g_free (title);
}


//!
//! @brief Set's the progress label of the program using the inforamtion from the searchitem
//!
//! @param search A LwSearch pointer to gleam information from
//!
void 
gw_searchwindow_set_total_results_label_by_searchitem (GwSearchWindow *window, LwSearch* search)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    gint total;
    gint relevant;

    //Initializations
    priv = window->priv;

    if (search == NULL)
    {
      gtk_label_set_text (priv->statusbar_label, "");
    }
    else
    {
      //Declarations
      relevant = lw_search_get_total_relevant_results (search);
      total = lw_search_get_total_results (search);

      gchar *idle_message_none = "";
      const gchar *searching_message_none = gettext("Searching...");

      const gchar *idle_message_total = ngettext("Found %d result", "Found %d results", total);
      const gchar *searching_message_total = ngettext("Searching... %d result", "Searching... %d results", total);

      // TRANSLATORS: relevant what ? It's the number of "relevant" result(s) displayed while or after searching.
      const gchar *message_relevant = ngettext("(%d Exact)", "(%d Exact)", relevant);

      gchar *base_message = NULL;
      gchar *final_message = NULL;

      //Initializations
      switch (search->status)
      {
        case LW_SEARCHSTATUS_IDLE:
        case LW_SEARCHSTATUS_FINISHING:
        case LW_SEARCHSTATUS_CANCELING:
            if (search->current == 0L)
              gtk_label_set_text (priv->statusbar_label, idle_message_none);
            else if (relevant == total)
              final_message = g_strdup_printf (idle_message_total, total);
            else
            {
              base_message = g_strdup_printf ("%s %s", idle_message_total, message_relevant);
              if (base_message != NULL)
                final_message = g_strdup_printf (base_message, total, relevant);
            }
            break;
        case LW_SEARCHSTATUS_SEARCHING:
            if (search->total_results == 0)
              gtk_label_set_text(priv->statusbar_label, searching_message_none);
            else if (relevant == total)
              final_message = g_strdup_printf (searching_message_total, total);
            else
            {
              base_message = g_strdup_printf ("%s %s", searching_message_total, message_relevant);
              if (base_message != NULL)
                final_message = g_strdup_printf (base_message, total, relevant);
            }
            break;
      }

      //Finalize
      if (base_message != NULL)
        g_free (base_message);
      if (final_message != NULL)
      {
        gtk_label_set_text(priv->statusbar_label, final_message);
        g_free (final_message);
      }
    }
}


//!
//! @brief Sets the current dictionary by using the load position
//!
//! @param request Sets the current dictionary by the number here
//!
void 
gw_searchwindow_set_dictionary (GwSearchWindow *window, gint position)
{
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    LwDictionary *dictionary;
    GwDictionaryList *dictionarylist;
    GSimpleAction *action;
    gchar *position_string;
    GActionMap *map;

    map = G_ACTION_MAP (window);
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    dictionarylist = gw_application_get_installed_dictionarylist (application);
    dictionary = lw_dictionarylist_get_dictionary_by_position (LW_DICTIONARYLIST (dictionarylist), position);
    if (dictionary == NULL) return;
    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "set-dictionary"));
    position_string = g_strdup_printf ("%d", position + 1);

    //Make sure the correct combobox search is selected
    G_GNUC_EXTENSION g_signal_handlers_block_by_func (priv->combobox, gw_searchwindow_dictionary_combobox_changed_cb, NULL);
    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_string (position_string));
    gtk_combo_box_set_active (priv->combobox, position);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (priv->combobox, gw_searchwindow_dictionary_combobox_changed_cb, NULL);

    priv->dictionary = dictionary;

    if (position_string != NULL) g_free (position_string); position_string = NULL;
}


//!
//! @brief Uses a LwSearch to set the currently active dictionary
//!
//! This function is greatly useful for doing searches from the history.
//!
//! @param search A lnSearchItem to gleam information from
//!
void 
gw_searchwindow_set_dictionary_by_searchitem (GwSearchWindow *window, LwSearch *search)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwApplication *application;
    GwDictionaryList *dictionarylist;
    LwDictionary *dictionary;
    gint position;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = gw_application_get_installed_dictionarylist (application);
    position = 0;

    if (search != NULL)
    {
      dictionary = LW_DICTIONARY (search->dictionary);
      position = lw_dictionarylist_get_position (LW_DICTIONARYLIST (dictionarylist), dictionary);
    }

    gw_searchwindow_set_dictionary (window, position);
}


LwDictionary* 
gw_searchwindow_get_dictionary (GwSearchWindow* window)
{
    GwSearchWindowPrivate *priv;

    priv = window->priv;

    return priv->dictionary;
}


//!
//! @brief Updates the status of the window progressbar
//!
//! @param search A LwSearch to gleam information from
//!
void 
gw_searchwindow_set_search_progressbar_by_searchitem (GwSearchWindow *window, LwSearch *search)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    gdouble fraction;

    //Initializations
    priv = window->priv;
    fraction = lw_search_get_progress (search);

    if (gtk_widget_get_visible (GTK_WIDGET (priv->statusbar)))
    {
      gtk_progress_bar_set_fraction (priv->statusbar_progressbar, fraction);
      gtk_entry_set_progress_fraction (priv->entry, 0.0);
    }
    else
    {
      gtk_entry_set_progress_fraction (priv->entry, fraction);
    }
}


void 
gw_searchwindow_sync_history (GwSearchWindow *window)
{
    GwSearchWindowPrivate *priv;
    GActionMap *map;
    GSimpleAction *action;
    gboolean enabled;
    LwHistory *history;

    priv = window->priv;
    map = G_ACTION_MAP (window);
    history = LW_HISTORY (priv->history);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "go-back"));
    enabled = lw_history_has_back (history);
    g_simple_action_set_enabled (action, enabled);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "go-forward"));
    enabled = lw_history_has_forward (history);
    g_simple_action_set_enabled (action, enabled);
}


//!
//! @brief Sets the style of the toolbar [icons/text/both/both-horizontal]
//!
//! @param request The name of the style
//!
void 
gw_searchwindow_set_toolbar_style (GwSearchWindow *window, const char *request) 
{
    GwSearchWindowPrivate *priv;
    GtkToolbarStyle style;

    priv = window->priv;

    if (strcmp(request, "text") == 0)
      style = GTK_TOOLBAR_TEXT;
    else if (strcmp(request, "icons") == 0)
      style = GTK_TOOLBAR_ICONS;
    else if (strcmp(request, "both-horiz") == 0)
      style = GTK_TOOLBAR_BOTH_HORIZ;
    else
      style = GTK_TOOLBAR_BOTH;

    gtk_toolbar_set_style (priv->primary_toolbar, style);
}


//!
//! @brief Appends some text to the text buffer
//!
//! @param search A LwSearch to gleam information from
//! @param text The text to append to the buffer
//! @param tag1 A tag to apply to the text or NULL
//! @param tag2 A tag to apply to the text or NULL
//! @param start_line Returns the start line of the text inserted
//! @param end_line Returns the end line of the text inserted
//!
void 
gw_searchwindow_append_to_buffer (GwSearchWindow *window, LwSearch *search, const char *text, char *tag1,
                                       char *tag2, int *start_line, int *end_line)
{
    if (search == NULL) return;

    //Assertain the target text buffer
    GwSearchData *sd;
    GtkTextView *view;
    GtkTextBuffer *buffer;

    sd = (GwSearchData*) lw_search_get_data (search);
    view = GTK_TEXT_VIEW (sd->view);
    if (view == NULL) return;
    buffer = gtk_text_view_get_buffer (view);
    if (buffer == NULL) return;

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter (buffer, &iter);
    if (start_line != NULL)
      *start_line = gtk_text_iter_get_line (&iter);

    if (tag1 == NULL && tag2 == NULL)
      gtk_text_buffer_insert (buffer, &iter, text, -1);
    else if (tag2 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag1, NULL);
    else if (tag1 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag2, NULL);
    else
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag1, tag2, NULL);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    if (end_line != NULL)
      *end_line = gtk_text_iter_get_line(&iter);
}


static void
gw_searchwindow_remove_anonymous_tags (GtkTextTag *tag, gpointer data)
{
    GtkTextTagTable *tagtable;
    gchar *name;

    tagtable = GTK_TEXT_TAG_TABLE (data);

    //This is not the tag we were looking for
    g_object_get (G_OBJECT (tag), "name", &name, NULL);
    if (name != NULL)
    {
      g_free (name);
    }
    //Remove the anonymous tag
    else
    {
      gtk_text_tag_table_remove (tagtable, tag);
    }
}


//!
//! @brief Performs initializations absolutely necessary before a window can take place
//!
//! Correctly the pointer in the LwSearch to the correct textbuffer and moves marks
//!
//! @param search A LwSearch to gleam information from
//!
void 
gw_searchwindow_initialize_buffer_by_searchitem (GwSearchWindow *window, LwSearch *search)
{
    //Sanity check
    g_assert (lw_search_has_data (search));

    //Make sure searches done from the history are pointing at a valid target
    GwSearchData *data;
    GtkTextView *view;
    GtkTextBuffer *buffer;

    data = GW_SEARCHDATA (lw_search_get_data (search));
    view = GTK_TEXT_VIEW (data->view);
    buffer = gtk_text_view_get_buffer (view);

    if (view == NULL || buffer == NULL) return;

    gtk_text_buffer_set_text (buffer, "", -1);

    //Clear the target text buffer
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter (buffer, &iter);

    gtk_text_buffer_create_mark (buffer, "content_insertion_mark", &iter, FALSE);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_create_mark (buffer, "footer_insertion_mark", &iter, FALSE);

    gw_searchwindow_set_total_results_label_by_searchitem (window, search);

    GtkTextTagTable *tagtable;
    tagtable = gtk_text_buffer_get_tag_table (buffer);
    if (tagtable != NULL)
    {
      gtk_text_tag_table_foreach (tagtable, gw_searchwindow_remove_anonymous_tags, tagtable);
    }
}


//!
//! @brief Selects all text in a target
//!
//! @param TARGET The widget where to select all text
//!
void 
gw_searchwindow_select_all (GwSearchWindow* window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    //Declarations
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    if (GTK_IS_ENTRY (widget))
    {
      gtk_editable_select_region (GTK_EDITABLE (widget), 0, -1);
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      gtk_text_buffer_get_start_iter (buffer, &start);
      gtk_text_buffer_get_end_iter (buffer, &end);
      gtk_text_buffer_select_range (buffer, &start, &end);
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_select_all()\n");
    }
}


//!
//! @brief Deselects all text in a target
//!
//! @param TARGET The widget where to deselect all text
//!
void 
gw_searchwindow_select_none (GwSearchWindow *window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    //Declarations
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    if (GTK_IS_ENTRY (widget))
    {
      gtk_editable_select_region (GTK_EDITABLE (widget), -1, -1);
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      gtk_text_buffer_get_end_iter (buffer, &start);
      gtk_text_buffer_get_end_iter (buffer, &end);
      gtk_text_buffer_select_range (buffer, &start, &end);
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_select_none()\n");
    }
}


//!
//! @brief Copy Text into the target output
//!
void 
gw_searchwindow_copy_text (GwSearchWindow* window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    //Declarations
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;

    if (GTK_IS_ENTRY (widget))
    {
      gtk_editable_copy_clipboard (GTK_EDITABLE (widget));
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
      gtk_text_buffer_copy_clipboard (buffer, clipboard);
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_copy_text()\n");
    }
}


//!
//! @brief Cut Text into the target output
//!
void 
gw_searchwindow_cut_text (GwSearchWindow *window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    if (GTK_IS_ENTRY (widget))
    {
        gtk_editable_cut_clipboard (GTK_EDITABLE (widget));
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      //We don't allow user editing the textview
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_cut_text()\n");
    }
}


//!
//! @brief Pastes Text into the target output
//!
void 
gw_searchwindow_paste_text (GwSearchWindow* window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    if (GTK_IS_ENTRY (widget))
    {
      gtk_editable_paste_clipboard (GTK_EDITABLE (widget));
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      //We don't allow user editing the textview
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_paste_text()\n");
    }
}



//!
//! @brief Returns the slice of characters between to line numbers in the target output buffer
//! @param sl The start line number
//! @param el The end line number
//!
char* 
gw_searchwindow_buffer_get_text_slice_from_current_view (GwSearchWindow* window, int sl, int el)
{
    //Assertain the target text buffer
    GtkTextView *view;
    GtkTextBuffer *buffer;

    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) return NULL;
    buffer = gtk_text_view_get_buffer (view);

    //Set up the text
    GtkTextIter si, ei;
    gtk_text_buffer_get_iter_at_line (buffer, &si, sl);
    gtk_text_buffer_get_iter_at_line (buffer, &ei, el);

    return gtk_text_buffer_get_slice (buffer, &si, &ei, TRUE);
}


//!
//! @brief Cycles the dictionaries forward or backward, looping when it reaches the end
//!
//! @param cycle_forward A boolean to choose the cycle direction
//!
void 
gw_searchwindow_cycle_dictionaries (GwSearchWindow* window, gboolean cycle_forward)
{
    int increment;

    if (cycle_forward)
      increment = 1;
    else
      increment = -1;

    //Declarations
    GwSearchWindowPrivate *priv;
    GtkTreeIter iter;
    gint active;
    gboolean set;

    priv = window->priv;
    active = gtk_combo_box_get_active (priv->combobox);
    set = FALSE;

    if ((active = active + increment) == -1)
    {
      do {
        active++;
        gtk_combo_box_set_active (priv->combobox, active);
        set = gtk_combo_box_get_active_iter (priv->combobox, &iter);
      } while (set);
      active--;
      gtk_combo_box_set_active (priv->combobox, active);
    }
    else
    {
      gtk_combo_box_set_active (priv->combobox, active);
      set = gtk_combo_box_get_active_iter (priv->combobox, &iter);
      if (!set)
        gtk_combo_box_set_active (priv->combobox, 0);
    }
}


//!
//! @brief  Returns the unfreeable text from a gtk widget by target
//!
//! The fancy thing about this function is it will only return the
//! highlighted text if some is highlighted.
//!
char* 
gw_searchwindow_get_text (GwSearchWindow *window, GtkWidget *widget)
{
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter s, e;

    view = gw_searchwindow_get_current_textview (window);
    buffer = gtk_text_view_get_buffer (view);

    if (gtk_text_buffer_get_has_selection (buffer))
    {
      gtk_text_buffer_get_selection_bounds (buffer, &s, &e);
    }
    //Get the region of text to be saved if no text is highlighted
    else
    {
      gtk_text_buffer_get_start_iter (buffer, &s);
      gtk_text_buffer_get_end_iter (buffer, &e);
    }
    return gtk_text_buffer_get_text (buffer, &s, &e, FALSE);
}



//!
//! @brief A simple window initiater function made to be looped by a timer
//!
//! @param data An unused gpointer.  It should always be NULL
//!
gboolean 
gw_searchwindow_keep_searching_timeout (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    const char *query;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    query = gtk_entry_get_text (GTK_ENTRY (priv->entry));
    //Sanity check
    if (!gw_application_can_start_search (application)) return TRUE;
    if (!priv->keep_searching_enabled) return TRUE;
    if (priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING] == 0) return FALSE;

    if (priv->keep_searching_query == NULL) priv->keep_searching_query = g_strdup ("");

    if (priv->keep_searching_delay >= GW_SEARCHWINDOW_KEEP_SEARCHING_MAX_DELAY || strlen(query) == 0)
    {
      priv->keep_searching_delay = 0;
      gtk_widget_activate (GTK_WIDGET (priv->entry));
    }
    else
    {
      if (strcmp(priv->keep_searching_query, query) == 0)
      {
        priv->keep_searching_delay++;
      }
      else
      {
        if (priv->keep_searching_query != NULL)
          g_free (priv->keep_searching_query);
        priv->keep_searching_query = g_strdup (query);
        priv->keep_searching_delay = 0;
      }
    }
    
    return TRUE;
}


//!
//! @brief Abstraction function to find out if some text is selected
//!
//! It gets the requested text buffer and then returns if it has text selected
//! or not.
//!
gboolean 
gw_searchwindow_has_selection (GwSearchWindow *window, GtkWidget *widget)
{
    //Declarations
    gboolean has_selection;
    GtkTextBuffer *buffer;

    //Initializations

    if (GTK_IS_ENTRY (widget))
    {
      has_selection = gtk_editable_get_selection_bounds (GTK_EDITABLE (widget), NULL, NULL);
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      has_selection = (buffer != NULL && gtk_text_buffer_get_has_selection (buffer));
    }
    else
    {
      has_selection = FALSE;
    }

    return has_selection;
}


//!
//! @brief Cancels all searches in all currently open tabs
//!
void 
gw_searchwindow_cancel_all_searches (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    GList *children, *link;
    LwSearch *search;
    GtkWidget *container;

    priv = window->priv;
    children = link = gtk_container_get_children (GTK_CONTAINER (priv->notebook));

    if (children != NULL)
    {
      while (link != NULL)
      {
        container = GTK_WIDGET (link->data);
        if (container != NULL)
        {
          search = LW_SEARCH (g_object_get_data (G_OBJECT (container), "searchitem"));
          if (search != NULL) 
          {
            lw_search_cancel (search);
          }
        }
        link = link->next;
      }
      g_list_free (children); children = NULL;
    }

    lw_search_cancel (priv->mouse_item);
}


//!
//! @brief Cancels the search of the tab number
//! @param page_num The page number of the tab to cancel the search of
//!
void 
gw_searchwindow_cancel_search_by_tab_number (GwSearchWindow *window, const int page_num)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    LwSearch *search;
    GtkWidget *widget;

    //Initializations
    priv = window->priv;
    widget = GTK_WIDGET (gtk_notebook_get_nth_page (priv->notebook, page_num));
    if (widget != NULL)
    {
      search = LW_SEARCH (g_object_get_data (G_OBJECT (widget), "searchitem"));
      if (search != NULL)
      {
        lw_search_cancel (search);
      }
    }
}


//!
//! @brief Cancels the search of the currently visibile tab
//!
void 
gw_searchwindow_cancel_search_for_current_tab (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    int page_num;

    //Initializations
    priv = window->priv;
    page_num = gtk_notebook_get_current_page (priv->notebook);

    gw_searchwindow_cancel_search_by_tab_number (window, page_num);
}


static GtkWidget*
_get_searchwindow_child_widget (GwSearchWindow *window, int page_num, int child_idx)
{
    //Sanity check
    g_assert (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    GtkWidget *child;
    GtkScrolledWindow *scrolledwindow;
    GtkViewport *viewport;
    GtkBox *scrollbox;
    GList *children;

    //Initializations
    priv = window->priv;
    child = NULL;

    scrolledwindow = GTK_SCROLLED_WINDOW (gtk_notebook_get_nth_page (priv->notebook, page_num));
    if (scrolledwindow == NULL)
      return NULL;

    viewport = GTK_VIEWPORT (gtk_bin_get_child (GTK_BIN (scrolledwindow)));
    if (viewport == NULL)
      return NULL;

    scrollbox = GTK_BOX (gtk_bin_get_child (GTK_BIN (viewport)));
    if (scrollbox == NULL)
      return NULL;

    children = gtk_container_get_children (GTK_CONTAINER (scrollbox));
    child = GTK_WIDGET (g_list_nth_data (children, child_idx));
    g_list_free (children);

    return child;
}


GtkTextView*
gw_searchwindow_get_textview (GwSearchWindow *window, int page_num)
{
    return GTK_TEXT_VIEW (_get_searchwindow_child_widget (window, page_num, 1));
}


GtkTextView*
gw_searchwindow_get_current_textview (GwSearchWindow *window)
{
    //Sanity checks
    g_return_val_if_fail (window != NULL, NULL);
    //gw_searchwindow_guarantee_first_tab (window);

    //Declarations
    GwSearchWindowPrivate *priv;
    gint page_num;

    //Initializaitons
    priv = window->priv;
    page_num = gtk_notebook_get_current_page (priv->notebook);

    return gw_searchwindow_get_textview (window, page_num);
}


GtkInfoBar*
gw_searchwindow_get_infobar (GwSearchWindow *window, int page_num)
{
    return GTK_INFO_BAR (_get_searchwindow_child_widget (window, page_num, 0));
}


GtkInfoBar*
gw_searchwindow_get_current_infobar (GwSearchWindow *window)
{
    g_assert (window != NULL);

    GwSearchWindowPrivate *priv;
    int page_num;

    priv = window->priv;
    page_num = gtk_notebook_get_current_page (priv->notebook);
    return gw_searchwindow_get_infobar (window, page_num);
}


//!
//! @brief Makes sure that at least one tab is available to output search results.
//!
void 
gw_searchwindow_guarantee_first_tab (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    int pages;

    //Initializations
    priv = window->priv;
    pages = gtk_notebook_get_n_pages (priv->notebook);

    if (pages == 0)
    {
      gw_searchwindow_new_tab (window);
    }
}


//!
//! @brief Sets the title text of the current tab.
//! @param TEXT The text to set to the tab
//!
void 
gw_searchwindow_update_tab_text_by_index (GwSearchWindow *window, gint index)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    GtkWidget *container;
    GtkWidget *box;
    GList *children;
    GtkWidget *label;
    const char *text;
    LwSearch *search;
    LwQuery *query;

    //Initializations
    priv = window->priv;
    container = GTK_WIDGET (gtk_notebook_get_nth_page (priv->notebook, index));
    search = LW_SEARCH (g_object_get_data (G_OBJECT (container), "searchitem"));
    query = NULL;
    text = "New Tab";
    box = GTK_WIDGET (gtk_notebook_get_tab_label (priv->notebook, GTK_WIDGET (container)));
    children = gtk_container_get_children (GTK_CONTAINER (box));

    if (search != NULL) query = search->query;
    if (query != NULL && lw_query_get_text (query) != NULL) text = lw_query_get_text (query);

    if (children != NULL)
    {
      label = GTK_WIDGET (children->data);
      gtk_label_set_text (GTK_LABEL (label), text);
      g_list_free (children); children = NULL;
    }
}


static GtkCssProvider* 
gw_searchwindowclass_get_tablabel_style_provider (GwSearchWindowClass *klass)
{
    const gchar *STYLE_DATA;
    if (klass->tablabel_cssprovider == NULL)
    {
      klass->tablabel_cssprovider = gtk_css_provider_new ();
      STYLE_DATA = "* {\n"
                   "-GtkButton-default-border : 0;\n"
                   "-GtkButton-default-outside-border : 0;\n"
                   "-GtkButton-inner-border: 0;\n"
                   "-GtkWidget-focus-line-width : 0;\n"
                   "-GtkWidget-focus-padding : 0;\n"
                   "padding: 1px;\n"
                   "}";
      gtk_css_provider_load_from_data (klass->tablabel_cssprovider, STYLE_DATA, -1, NULL);
      g_object_add_weak_pointer (G_OBJECT (klass->tablabel_cssprovider), (gpointer*) &(klass->tablabel_cssprovider));
    }
    else
    {
      g_object_ref (klass->tablabel_cssprovider);
    }

    return klass->tablabel_cssprovider;
}


static GtkWidget*
gw_searchwindow_tabcontent_new (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GtkWidget *scrollbox;
    GtkWidget *viewport;
    GtkWidget *infobar;
    GtkWidget *scrolledwindow;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextTagTable *tagtable;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    tagtable = gw_texttagtable_new (application);
    scrolledwindow = GTK_WIDGET (gtk_scrolled_window_new (NULL, NULL));
    buffer = GTK_TEXT_BUFFER (gtk_text_buffer_new (tagtable));
    view = GTK_TEXT_VIEW (gtk_text_view_new_with_buffer (buffer));
    infobar = GTK_WIDGET (_construct_infobar());
    scrollbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
    viewport = GTK_WIDGET (gtk_viewport_new (NULL, NULL));

    g_object_unref (G_OBJECT (tagtable));
    g_object_unref (G_OBJECT (buffer));

    //Set up the text buffer
    gtk_text_buffer_get_start_iter (buffer, &iter);
    gtk_text_buffer_create_mark (buffer, "less_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "more_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "footer_insertion_mark", &iter, FALSE);

    //Set up the text view
    gtk_text_view_set_right_margin (view, 10);
    gtk_text_view_set_left_margin (view, 10);
    gtk_text_view_set_cursor_visible (view, FALSE); 
    gtk_text_view_set_editable (view, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_text_view_set_wrap_mode (view, GTK_WRAP_WORD);

    g_signal_connect (G_OBJECT (view), "drag_motion", G_CALLBACK (gw_searchwindow_drag_motion_1_cb), window);
    g_signal_connect (G_OBJECT (view), "button_press_event", G_CALLBACK (gw_searchwindow_get_position_for_button_press_cb), window);
    g_signal_connect (G_OBJECT (view), "motion_notify_event", G_CALLBACK (gw_searchwindow_motion_notify_event_cb), window);
    g_signal_connect (G_OBJECT (view), "drag_drop", G_CALLBACK (gw_searchwindow_drag_drop_1_cb), window);
    g_signal_connect (G_OBJECT (view), "button_release_event", G_CALLBACK (gw_searchwindow_get_iter_for_button_release_cb), window);
    g_signal_connect (G_OBJECT (view), "drag_leave", G_CALLBACK (gw_searchwindow_drag_leave_1_cb), window);
    g_signal_connect (G_OBJECT (view), "drag_data_received", G_CALLBACK (gw_searchwindow_search_drag_data_recieved_cb), window);
    g_signal_connect (G_OBJECT (view), "key_press_event", G_CALLBACK (gw_searchwindow_focus_change_on_key_press_cb), window);
    g_signal_connect (G_OBJECT (view), "scroll_event", G_CALLBACK (gw_searchwindow_scroll_or_zoom_cb), window);

    //Set up the scrolled window
    gtk_box_pack_start (GTK_BOX (scrollbox), GTK_WIDGET (infobar), FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (scrollbox), GTK_WIDGET (view), TRUE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (viewport), GTK_WIDGET (scrollbox));

    gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (viewport));
    gtk_widget_show_all (GTK_WIDGET (scrolledwindow));

    return GTK_WIDGET (scrolledwindow);
}


GtkSizeGroup* gw_searchwindowclass_get_tablabel_sizegroup (GwSearchWindowClass *klass)
{
    if (klass->tablabel_sizegroup == NULL)
    {
      klass->tablabel_sizegroup = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);
      g_object_add_weak_pointer (G_OBJECT (klass->tablabel_sizegroup), (gpointer*) &(klass->tablabel_sizegroup));
    }
    else
    {
      g_object_ref (G_OBJECT (klass->tablabel_sizegroup));
    }

    return klass->tablabel_sizegroup;
}


static GtkWidget*
gw_searchwindow_tablabel_new (GwSearchWindow *window, const gchar *TEXT, GtkWidget *contents)
{
    //Create the tab label
    GwSearchWindowClass *klass;
    GtkWidget *container;
    GtkWidget *label;
    GtkWidget *close_button;
    GtkWidget *button_image;
    GtkStyleContext *context;
    GtkCssProvider *provider;
    GtkSizeGroup *sizegroup;

    //Initializations
    klass = GW_SEARCHWINDOW_CLASS (G_OBJECT_GET_CLASS (G_OBJECT (window)));
    container = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10));
    label = GTK_WIDGET (gtk_label_new (TEXT));
    close_button = GTK_WIDGET (gtk_button_new ());
    button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
    provider = gw_searchwindowclass_get_tablabel_style_provider (klass);
    context = gtk_widget_get_style_context (close_button);
    sizegroup = gw_searchwindowclass_get_tablabel_sizegroup (klass);

    //Set up the label
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

    //Set up the button
    gtk_widget_set_margin_top (close_button, 1);
    gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (close_button), 0);
    gtk_misc_set_padding (GTK_MISC (button_image), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (button_image), 0.5, 0.5);
    gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_container_add (GTK_CONTAINER (close_button), button_image);
    g_signal_connect (G_OBJECT (close_button), "clicked", G_CALLBACK (gw_searchwindow_remove_tab_cb), contents);

    //Combine the elements
    gtk_box_pack_start (GTK_BOX (container), label, TRUE, TRUE, 0);
    gtk_size_group_add_widget (sizegroup, label);

    gtk_box_pack_start (GTK_BOX (container), close_button, TRUE, TRUE, 0);
    gtk_size_group_add_widget (sizegroup, close_button);

    gtk_widget_show_all (GTK_WIDGET (container));

    g_object_unref (provider);
    g_object_unref (sizegroup);

    return GTK_WIDGET (container);
}


//!
//! @brief Creats a new tab.  The focus and other details are handled by gw_tabs_new_cb ()
//!
int 
gw_searchwindow_new_tab (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    GtkWidget *tabcontent;
    GtkWidget *tablabel;
    gint index;

    //Initializations
    priv = window->priv;
    tabcontent = gw_searchwindow_tabcontent_new (window);
    tablabel = gw_searchwindow_tablabel_new (window, NULL, tabcontent);
    index = gtk_notebook_append_page (priv->notebook, tabcontent, tablabel);

    gw_searchwindow_set_searchitem_by_index (window, index, NULL);
    gw_searchwindow_set_font (window);

    //Put everything together
    gtk_notebook_set_tab_reorderable (priv->notebook, tabcontent, TRUE);

    gtk_notebook_set_current_page (priv->notebook, index);
    gw_searchwindow_set_entry_text_by_searchitem (window, NULL);
    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));

    return index;
}


void 
gw_searchwindow_remove_tab_by_index (GwSearchWindow *window, gint index)
{
    //Sanity check
    g_assert (window != NULL && index > -1);

    //Declarations
    GwSearchWindowPrivate *priv;
    LwSearch *search;
    LwHistory *history;

    //Initializations
    priv = window->priv;
    search = gw_searchwindow_steal_searchitem_by_index (window, index);
    history = LW_HISTORY (priv->history);

    gw_searchwindow_cancel_search_by_tab_number (window, index);

    if (search != NULL)
    {
      gw_searchwindow_initialize_buffer_by_searchitem (window, search);
      if (lw_history_has_relevance (history, search, priv->keep_searching_enabled))
      {
        lw_history_add_search (history, search);
      }
      else
      {
        lw_search_free (search); search = NULL;
      }
    }

    gtk_notebook_remove_page (priv->notebook, index);
    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
}


gint
gw_searchwindow_get_current_tab_index (GwSearchWindow *window)
{
    //Sanity checks
    g_return_val_if_fail (window != NULL, 0);

    //Declarations
    GwSearchWindowPrivate *priv;
    gint index;

    //Initializations
    priv = window->priv;
    index = gtk_notebook_get_current_page (priv->notebook);

    return index;
}


//!
//! @brief The searchwindow searchitem should be set when a new search takes place
//!        using a newly made searchitem.
//!
void 
gw_searchwindow_set_searchitem_by_index (GwSearchWindow *window, gint index, LwSearch *search)
{
    //Sanity checks
    g_return_if_fail (GW_SEARCHWINDOW (window) != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    GtkWidget *container;
    gboolean enabled;
    GActionMap *map;
    GSimpleAction *action;

    //Initializations
    priv = window->priv;
    map = G_ACTION_MAP (window);

    if (index > -1)
    {
      container = gtk_notebook_get_nth_page (priv->notebook, index);
      if (container != NULL)
      {
        g_object_set_data_full (G_OBJECT (container), "searchitem", search, (GDestroyNotify) lw_search_free);

        //Update the window to match the searchitem data
        gw_searchwindow_update_tab_text_by_index (window, index);
        gw_searchwindow_set_dictionary_by_searchitem (window, search);
        gw_searchwindow_set_entry_text_by_searchitem (window, search);
        gw_searchwindow_set_title_by_searchitem (window, search);
        gw_searchwindow_set_total_results_label_by_searchitem (window, search);
        gw_searchwindow_set_search_progressbar_by_searchitem (window, search);

        //Update Save sensitivity state
        enabled = (search != NULL);
        action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "save"));
        g_simple_action_set_enabled (action, enabled);

        //Update Save as sensitivity state
        enabled = (search != NULL);
        action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "save-as"));
        g_simple_action_set_enabled (action, enabled);

        //Update Print sensitivity state
        enabled = (search != NULL);
        action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "print"));
        g_simple_action_set_enabled (action, enabled);

        //Update Print preview sensitivity state
        enabled = (search != NULL);
        action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "print-preview"));
        g_simple_action_set_enabled (action, enabled);
      }
    }
}


LwSearch*
gw_searchwindow_get_searchitem_by_index (GwSearchWindow *window, gint index)
{
    GwSearchWindowPrivate *priv;
    LwSearch *search;
    GtkWidget *container;

    priv = window->priv;
    search = NULL;

    container = gtk_notebook_get_nth_page (priv->notebook, index);
    if (container != NULL)
    {
      search = LW_SEARCH (g_object_get_data (G_OBJECT (container), "searchitem"));
    }

    return search;
}


LwSearch*
gw_searchwindow_steal_searchitem_by_index (GwSearchWindow *window, gint index)
{
    GwSearchWindowPrivate *priv;
    LwSearch *search;
    GtkWidget *container;

    priv = window->priv;
    search = NULL;

    container = gtk_notebook_get_nth_page (priv->notebook, index);
    if (container != NULL)
    {
      search = LW_SEARCH (g_object_steal_data (G_OBJECT (container), "searchitem"));
    }

    return search;
}


void 
gw_searchwindow_start_search (GwSearchWindow *window, LwSearch* new_search)
{
    //Sanity checks
    g_return_if_fail (window != NULL);
    g_return_if_fail (new_search != NULL);

    //Declarations
    GwApplication *application;
    GwSearchData *sdata;
    GtkTextView *view;
    gint index;

    //Initializations
    index = gw_searchwindow_get_current_tab_index (window);
    application = gw_window_get_application (GW_WINDOW (window));
    if (!gw_application_can_start_search (application)) return;
    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) goto errored;
    sdata = GW_SEARCHDATA (gw_searchdata_new (view, window));
    if (sdata == NULL) goto errored;
    lw_search_set_data (new_search, sdata, LW_SEARCH_DATA_FREE_FUNC (gw_searchdata_free));

    {
      LwSearch *search;
      search = gw_searchwindow_get_searchitem_by_index (window, index);
      if (search != NULL)
      {
        LwSearchStatus status;
        status = lw_search_get_status (search);
        g_return_if_fail (status != LW_SEARCHSTATUS_IDLE || status != LW_SEARCHSTATUS_FINISHING);
      }
    }

    gw_searchwindow_set_searchitem_by_index (window, index, new_search);
    gw_searchwindow_initialize_buffer_by_searchitem (sdata->window, new_search);

    lw_search_start (new_search, TRUE);

#if WITH_MECAB
    if (new_search->query->morphology)
    {
      gchar *message;
      message = g_strdup_printf (gettext("Also showing results for: 「%s」"), new_search->query->morphology);
      gw_searchwindow_show_current_infobar (window, message);
      g_free(message); message = NULL;
    }
    else
    {
      gw_searchwindow_hide_current_infobar (window);
    }
#endif

    return;
errored:
    if (new_search != NULL) lw_search_free (new_search); new_search = NULL;
}


//!
//! @brief Sets the requested font with magnification applied
//!
void 
gw_searchwindow_set_font (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    LwPreferences *preferences;

    GtkTextView *view;
    gboolean use_global_font_setting;
    gint size;
    gint magnification;
    gchar font[50];
    PangoFontDescription *desc;
    gint i;


    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    preferences = gw_application_get_preferences (application);

    use_global_font_setting = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_USE_GLOBAL_FONT);
    magnification = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION);

    if (use_global_font_setting)
    {
      lw_preferences_get_string_by_schema (preferences, font, LW_SCHEMA_GNOME_INTERFACE, LW_KEY_DOCUMENT_FONT_NAME, 50);
    }
    else
    {
      lw_preferences_get_string_by_schema (preferences, font, LW_SCHEMA_FONT, LW_KEY_FONT_CUSTOM_FONT, 50);
    }

    desc = pango_font_description_from_string (font);
    if (desc != NULL)
    {
      if (pango_font_description_get_size_is_absolute (desc))
        size = pango_font_description_get_size (desc) + magnification;
      else
        size = PANGO_PIXELS (pango_font_description_get_size (desc)) + magnification;

      //Make sure the font size is sane
      if (size < GW_APPLICATION_MIN_FONT_SIZE)
        size = GW_APPLICATION_MIN_FONT_SIZE;
      else if (size > GW_APPLICATION_MAX_FONT_SIZE)
        size = GW_APPLICATION_MAX_FONT_SIZE;

      priv->font_size = size;

      pango_font_description_set_size (desc, size * PANGO_SCALE);
      if (use_global_font_setting) pango_font_description_set_family (desc, "Serif");

      //Set it
      i = 0;
      while ((view = gw_searchwindow_get_textview (window, i)) != NULL)
      {
        gtk_widget_override_font (GTK_WIDGET (view), desc);
        i++;
      }

      //Cleanup
      pango_font_description_free (desc); desc = NULL;

      {
        GActionMap *map;
        GSimpleAction *action;
        gboolean enabled;
        
        map = G_ACTION_MAP (window);

        //Update Zoom in sensitivity state
        enabled = (magnification < GW_APPLICATION_MAX_FONT_MAGNIFICATION);
        action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "zoom-in"));
        g_simple_action_set_enabled (action, enabled);

        //Update Zoom out sensitivity state
        enabled = (magnification > GW_APPLICATION_MIN_FONT_MAGNIFICATION);
        action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "zoom-out"));
        g_simple_action_set_enabled (action, enabled);

        //Update Zoom 100 sensitivity state
        enabled = (magnification != GW_APPLICATION_DEFAULT_FONT_MAGNIFICATION);
        action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "zoom-100"));
        g_simple_action_set_enabled (action, enabled);
      }
    }
}




static void 
gw_searchwindow_attach_signals (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GwDictionaryList *dictionarylist;
    LwPreferences *preferences;

    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;

    priv->signalid = g_new0 (guint, TOTAL_GW_SEARCHWINDOW_SIGNALIDS);
    priv->timeoutid = g_new0 (guint, TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS);

    dictionarylist = gw_application_get_installed_dictionarylist (application);
    preferences = gw_application_get_preferences (application);


    g_signal_connect_after (G_OBJECT (window), "delete-event", 
                            G_CALLBACK (gw_window_delete_event_cb), window);
    g_signal_connect (G_OBJECT (window), "key-release-event", 
                      G_CALLBACK (gw_searchwindow_key_release_modify_status_update_cb), window);
    g_signal_connect (G_OBJECT (window), "key-press-event", 
                      G_CALLBACK (gw_searchwindow_key_press_modify_status_update_cb), window);
    g_signal_connect (G_OBJECT (window), "focus-in-event", 
                      G_CALLBACK (gw_searchwindow_focus_in_event_cb), window);
    g_signal_connect (G_OBJECT (window), "event-after", 
                      G_CALLBACK (gw_searchwindow_event_after_cb), window);
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gw_searchwindow_remove_signals), NULL);

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_MENUBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_MENUBAR_SHOW,
        gw_searchwindow_sync_menubar_show_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_TOOLBAR_SHOW,
        gw_searchwindow_sync_toolbar_show_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_TABBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_TABBAR_SHOW,
        gw_searchwindow_sync_tabbar_show_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_STATUSBAR_SHOW,
        gw_searchwindow_sync_statusbar_show_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_USE_GLOBAL_FONT,
        gw_searchwindow_sync_font_cb,
        window
    );
    priv->signalid[GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_CUSTOM_FONT,
        gw_searchwindow_sync_font_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_MAGNIFICATION,
        gw_searchwindow_sync_font_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SEARCH_AS_YOU_TYPE,
        gw_searchwindow_sync_search_as_you_type_cb,
        window
    );

#ifdef WITH_HUNSPELL
    priv->signalid[GW_SEARCHWINDOW_SIGNALID_SPELLCHECK] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK,
        gw_searchwindow_sync_spellcheck_cb,
        window
    );
#endif

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_CHANGED] = g_signal_connect_swapped (
        G_OBJECT (dictionarylist),
        "changed",
        G_CALLBACK (gw_searchwindow_dictionaries_changed_cb),
        window 
    );

    priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING] = gdk_threads_add_timeout (
          500,
          (GSourceFunc) gw_searchwindow_keep_searching_timeout, 
          window
    );

    priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_PROGRESS] = g_timeout_add_full (
          G_PRIORITY_LOW, 
          100, 
          (GSourceFunc) gw_searchwindow_update_progress_feedback_timeout, 
          window, 
          NULL
    );

    priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_APPEND_RESULT] = g_timeout_add_full (
          G_PRIORITY_LOW, 
          100, 
          (GSourceFunc) gw_searchwindow_append_result_timeout, 
          window, 
          NULL
    );


    g_signal_connect_swapped (G_OBJECT (priv->history), "changed", G_CALLBACK (gw_searchwindow_sync_history), window);
}


static void 
gw_searchwindow_remove_signals (GwSearchWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    LwPreferences *preferences;
    GSource *source;
    gint i;

    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    preferences = gw_application_get_preferences (application);
  
    if (priv->radicalswindow != NULL && priv->signalid[GW_SEARCHWINDOW_SIGNALID_RADICALSWINDOW_CLOSED] != 0)
    {
      g_signal_handler_disconnect (priv->radicalswindow, priv->signalid[GW_SEARCHWINDOW_SIGNALID_RADICALSWINDOW_CLOSED]);
      priv->signalid[GW_SEARCHWINDOW_SIGNALID_RADICALSWINDOW_CLOSED] = 0;
      gtk_widget_destroy (GTK_WIDGET (priv->radicalswindow));
    }

    if (priv->kanjipadwindow != NULL && priv->signalid[GW_SEARCHWINDOW_SIGNALID_KANJIPADWINDOW_CLOSED] != 0)
    {
      g_signal_handler_disconnect (priv->kanjipadwindow, priv->signalid[GW_SEARCHWINDOW_SIGNALID_KANJIPADWINDOW_CLOSED]);
      priv->signalid[GW_SEARCHWINDOW_SIGNALID_KANJIPADWINDOW_CLOSED] = 0;
      gtk_widget_destroy (GTK_WIDGET (priv->kanjipadwindow));
    }

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS; i++)
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

    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_MENUBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_TABBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING]
    );

#ifdef WITH_HUNSPELL
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_SPELLCHECK]
    );
#endif

    if (priv->signalid != NULL) g_free (priv->signalid); priv->signalid = NULL;
    if (priv->timeoutid != NULL) g_free (priv->timeoutid); priv->timeoutid = NULL;
}


void 
gw_searchwindow_initialize_dictionary_combobox (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GtkComboBox *combobox;
    GtkCellRenderer *renderer;
    GwDictionaryList *dictionarylist;
    GtkListStore *liststore;
    GtkTreeModel *treemodel;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    combobox = priv->combobox;
    renderer = gtk_cell_renderer_text_new ();
    dictionarylist = gw_application_get_installed_dictionarylist (application);
    liststore = gw_dictionarylist_get_liststore (dictionarylist);
    treemodel = GTK_TREE_MODEL (liststore);

    gtk_combo_box_set_model (combobox, NULL);
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (combobox));

    gtk_combo_box_set_model (combobox, treemodel);
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", GW_DICTIONARYLIST_COLUMN_LONG_NAME, NULL);
    gtk_combo_box_set_active (combobox, 0);
}

void
gw_searchwindow_initialize_search_toolbar (GwSearchWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    GtkIconTheme *theme;
    GtkToolbar *toolbar;
    GtkToolItem *item;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *combobox;
    GtkSettings *settings;
    gboolean os_shows_win_menu;

    priv = window->priv;
    theme = gtk_icon_theme_get_default ();
    toolbar = priv->search_toolbar;

    settings = gtk_settings_get_default ();
    g_object_get (settings, "gtk-shell-shows-menubar", &os_shows_win_menu, NULL);
    gtk_toolbar_set_icon_size (toolbar, GTK_ICON_SIZE_MENU);

/*
    if (gtk_icon_theme_has_icon (theme, "edit-find-symbolic") == FALSE)
    {
*/
      item = gtk_tool_item_new (); 
      label = gtk_label_new_with_mnemonic (gettext ("Sear_ch:"));
      gtk_container_add (GTK_CONTAINER (item), label);
      gtk_widget_set_margin_left (GTK_WIDGET (item), 2);
      gtk_widget_set_margin_right (GTK_WIDGET (item), 2);
      gtk_widget_show_all (GTK_WIDGET (item));
      gtk_toolbar_insert (toolbar, item, -1);
/*
    }
*/

    item = gtk_tool_item_new (); 
    entry = gtk_entry_new ();
/*
    if (gtk_icon_theme_has_icon (theme, "edit-find-symbolic"))
    {
      gtk_entry_set_icon_from_icon_name (GTK_ENTRY (entry), GTK_ENTRY_ICON_PRIMARY, "edit-find-symbolic");
    }
    else
    {
*/
      gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
/*
    }
*/
    g_signal_connect (entry, "activate", G_CALLBACK (gw_searchwindow_search_cb), window);
    g_signal_connect (entry, "changed", G_CALLBACK (gw_searchwindow_update_button_states_based_on_entry_text_cb), window);
    g_signal_connect (entry, "icon-release", G_CALLBACK (gw_searchwindow_clear_entry_button_pressed_cb), window);
    g_signal_connect (entry, "key-press-event", G_CALLBACK (gw_searchwindow_focus_change_on_key_press_cb), window);

    gtk_container_add (GTK_CONTAINER (item), entry);
    gtk_widget_set_margin_left (GTK_WIDGET (item), 2);
    gtk_widget_set_margin_right (GTK_WIDGET (item), 2);
    gtk_tool_item_set_expand (item, TRUE);
    gtk_widget_show_all (GTK_WIDGET (item));
    gtk_toolbar_insert (toolbar, item, -1);
    priv->entry = GTK_ENTRY (entry);

    item = gtk_tool_item_new (); 
    combobox = gtk_combo_box_new ();
    g_signal_connect (combobox, "changed", G_CALLBACK (gw_searchwindow_dictionary_combobox_changed_cb), window);
    gtk_container_add (GTK_CONTAINER (item), combobox);
    gtk_widget_set_margin_left (GTK_WIDGET (item), 2);
    gtk_widget_set_margin_right (GTK_WIDGET (item), 2);
    gtk_widget_show_all (GTK_WIDGET (item));
    gtk_toolbar_insert (toolbar, item, -1);
    priv->combobox = GTK_COMBO_BOX (combobox);
    
    item = gtk_tool_button_new_from_stock (GTK_STOCK_FIND);
    if (gtk_icon_theme_has_icon (theme, "edit-find-symbolic")) 
    {
      gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "edit-find-symbolic");
      gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), NULL);
    }
    g_signal_connect (item, "clicked", G_CALLBACK (gw_searchwindow_search_cb), window);

    gtk_widget_show_all (GTK_WIDGET (item));
    gtk_toolbar_insert (toolbar, item, -1);
    priv->submit_toolbutton = GTK_TOOL_BUTTON (item);


    if (!os_shows_win_menu)
    {
      item = gtk_tool_item_new ();

      GMenuModel *menumodel;
      GtkMenuBar *menubar;
      GtkMenuItem *menuitem;
      GtkImage *image;
      GtkMenu *submenu;

      menumodel = gw_searchwindow_get_popup_menu (window);
      menubar = GTK_MENU_BAR (gtk_menu_bar_new ());
      image = GTK_IMAGE (gtk_image_new_from_stock (GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU));
      if (gtk_icon_theme_has_icon (theme, "system-run-symbolic")) gtk_image_set_from_icon_name (image, "system-run-symbolic", GTK_ICON_SIZE_MENU);
      menuitem = GTK_MENU_ITEM (gtk_menu_item_new ());
      submenu = GTK_MENU (gtk_menu_new_from_model (menumodel));

      gtk_toolbar_insert (toolbar, item, -1);
      gtk_container_add (GTK_CONTAINER (item), GTK_WIDGET (menubar));
      gtk_container_add (GTK_CONTAINER (menuitem), GTK_WIDGET (image));

      gtk_menu_shell_append (GTK_MENU_SHELL (menubar), GTK_WIDGET (menuitem));
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), GTK_WIDGET (submenu));
      gtk_widget_show_all (GTK_WIDGET (submenu));

      gtk_widget_show_all (GTK_WIDGET (item));
      priv->menu_toolbutton = GTK_TOOL_ITEM (item);
    }
}


void
gw_searchwindow_initialize_toolbar (GwSearchWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    GtkToolbar *toolbar;
    GtkToolItem *item;

    priv = window->priv;
    toolbar = priv->primary_toolbar;

    item = gtk_tool_button_new_from_stock (GTK_STOCK_SAVE);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.save");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new_from_stock (GTK_STOCK_SAVE_AS);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.save-as");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new_from_stock (GTK_STOCK_PRINT);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.print");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new (NULL, gettext("Edge"));
    gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "word-boundary");
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.insert-word-edge-character");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new (NULL, gettext("Not-Edge"));
    gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "non-word-boundary");
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.insert-not-word-edge-character");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new (NULL, gettext("Unknown"));
    gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (item), "unknown-character");
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.insert-unknown-character");
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));

#ifdef WITH_HUNSPELL
    item = gtk_toggle_tool_button_new ();
    gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), GTK_STOCK_SPELL_CHECK);
    gtk_toolbar_insert (toolbar, item, -1);
    priv->spellcheck_toolbutton = GTK_TOOL_BUTTON (item);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "app.toggle-spellcheck");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));
#endif

    item = gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_IN);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.zoom-in");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.zoom-out");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_100);
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.zoom-100");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_toggle_tool_button_new ();
    gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), GTK_STOCK_EDIT);
    gtk_tool_button_set_label (GTK_TOOL_BUTTON (item), "Kanjipad");
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.toggle-kanjipad-show");
    gtk_widget_show (GTK_WIDGET (item));

    item = gtk_toggle_tool_button_new ();
    gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (item), GTK_STOCK_FIND);
    gtk_tool_button_set_label (GTK_TOOL_BUTTON (item), "Radicals");
    gtk_toolbar_insert (toolbar, item, -1);
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (item), "win.toggle-radicals-show");
    gtk_widget_show (GTK_WIDGET (item));
}


void 
gw_searchwindow_initialize_menu_links (GwSearchWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    GwDictionaryList *dictionarylist;
    GwVocabularyListStore *store;
    GMenuModel *menumodel;
    GMenuModel *link;

    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    menumodel = gw_window_get_menumodel (GW_WINDOW (window));
    g_return_if_fail (menumodel != NULL);

    link = gw_history_get_combined_menumodel (priv->history);
    gw_menumodel_set_links (menumodel, "history-list-link", NULL, G_MENU_LINK_SECTION, link);
    
    dictionarylist = gw_application_get_installed_dictionarylist (application);
    link = gw_dictionarylist_get_menumodel (dictionarylist);
    gw_menumodel_set_links (menumodel, "dictionary-list-link", NULL, G_MENU_LINK_SECTION, link);

    store = GW_VOCABULARYLISTSTORE (gw_application_get_vocabularyliststore (application));
    link = gw_vocabularyliststore_get_menumodel (store);
    gw_menumodel_set_links (menumodel, "vocabulary-list-link", NULL, G_MENU_LINK_SECTION, link);
}
    

static 
GtkInfoBar* _construct_infobar ()
/*TODO*/
{
    GtkInfoBar *infobar;
    GtkWidget *label;
    GtkWidget *content_area;
    GtkWidget *image;

    infobar = GTK_INFO_BAR (gtk_info_bar_new ());
    content_area = gtk_info_bar_get_content_area (infobar);
    label = gtk_label_new ("");
    image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_BUTTON);

    gtk_widget_set_no_show_all (GTK_WIDGET (infobar), TRUE);

    gtk_widget_set_margin_left (image, 8);

    gtk_label_set_selectable (GTK_LABEL (label), TRUE);
    gtk_widget_set_can_focus (GTK_WIDGET (label), FALSE);

    gtk_box_set_spacing (GTK_BOX (content_area), 0);
    gtk_container_set_border_width (GTK_CONTAINER (content_area), 20);
    gtk_box_pack_start (GTK_BOX (content_area), GTK_WIDGET (image), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (content_area), GTK_WIDGET (label), FALSE, TRUE, 0);

    g_signal_connect (G_OBJECT (infobar), "response", G_CALLBACK (gtk_widget_hide), NULL);

    gtk_widget_show (label);
    gtk_widget_show (image);

    return infobar;
}

void 
gw_searchwindow_show_current_infobar (GwSearchWindow *window, char *message)
{
    GtkContainer *container;
    GtkLabel *label;
    GList *children;
    GtkInfoBar *infobar;
    gchar *markup;

    infobar = gw_searchwindow_get_current_infobar (window);

    container = GTK_CONTAINER (gtk_info_bar_get_content_area (infobar));
    children = gtk_container_get_children (container);

    if (children != NULL)
    {
      label = GTK_LABEL (g_list_nth_data (children, 1));

      if (label != NULL)
      {
        markup = g_markup_printf_escaped ("<small><b>%s</b></small>", message);
        gtk_label_set_markup (GTK_LABEL (label), markup);
        g_free (markup);

        gtk_info_bar_set_message_type (infobar, GTK_MESSAGE_OTHER);

        if (!gtk_widget_get_visible (GTK_WIDGET (infobar)))
          gtk_widget_show (GTK_WIDGET (infobar));
      }

      g_list_free (children); children = NULL;
    }
}

void 
gw_searchwindow_hide_current_infobar (GwSearchWindow *window)
{
    GtkInfoBar *infobar;

    infobar = gw_searchwindow_get_current_infobar (window);
    gtk_widget_hide (GTK_WIDGET (infobar));
}


void
gw_searchwindow_sync_tabbar_show (GwSearchWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GActionMap *map;
    GwSearchWindowPrivate *priv;
    GtkNotebook *notebook;
    gint pages;
    gboolean always_show_tabbar;
    gboolean show;
    gboolean enabled;
    GSimpleAction *action;

    //Initializations
    map = G_ACTION_MAP (window);
    priv = window->priv;
    notebook = priv->notebook;
    always_show_tabbar = priv->always_show_tabbar;
    pages = gtk_notebook_get_n_pages (notebook);
    show = ((pages > 1) || always_show_tabbar);
    enabled = (pages > 1);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "previous-tab"));
    g_simple_action_set_enabled (action, enabled);
    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "next-tab"));
    g_simple_action_set_enabled (action, enabled);
    
    gtk_notebook_set_show_tabs (notebook, show);
}



void
gw_searchwindow_go_back (GwSearchWindow *window, gint i)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    LwHistory *history;
    LwSearch *search;
    gint index;
    
    //Initializations
    priv = window->priv;
    history = LW_HISTORY (priv->history);
    index = gw_searchwindow_get_current_tab_index (window);
    search = gw_searchwindow_steal_searchitem_by_index (window, index);

    while (i-- && search != NULL) search = lw_history_go_back (history, search);

    if (search != NULL) gw_searchwindow_start_search (window, search);
}


void
gw_searchwindow_go_forward (GwSearchWindow *window, gint i)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    LwHistory *history;
    LwSearch *search;
    gint index;
    
    //Initializations
    priv = window->priv;
    history = LW_HISTORY (priv->history);
    index = gw_searchwindow_get_current_tab_index (window);
    search = gw_searchwindow_steal_searchitem_by_index (window, index);

    while (i-- && search != NULL) search = lw_history_go_forward (history, search);

    if (search != NULL) gw_searchwindow_start_search (window, search);
}


GMenuModel*
gw_searchwindow_get_popup_menu (GwSearchWindow *window)
{
    //Sanity checks
    g_return_val_if_fail (window != NULL, NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    GtkBuilder *builder;
    GMenuModel *menumodel;
    GMenuModel *link;

    //Initializations
    priv = window->priv;
    builder = gtk_builder_new ();
    gw_application_load_xml (builder, "searchwindow-menumodel-button.ui");
    menumodel = G_MENU_MODEL (gtk_builder_get_object (builder, "menu"));
    link = gw_history_get_combined_menumodel (priv->history);
    
    gw_menumodel_set_links (menumodel, "history-list-link", NULL, G_MENU_LINK_SECTION, link);

    if (builder != NULL) g_object_unref (builder); builder = NULL;
    
    return menumodel;
}

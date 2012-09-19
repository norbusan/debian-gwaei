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
//! @file src/gtk-main-interface-tabs.c
//!
//! @brief Abstraction layer for gtk callbacks
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static GList *_tab_searchitems = NULL;


void gw_tabs_initialize ()
{
    gw_tabs_guarantee_first ();
}

void gw_tabs_free ()
{
}

//!
//! @brief Returns the private glist of searchitems for the tabs
//!
GList *gw_tabs_get_searchitem_list ()
{
    return _tab_searchitems;
}


LwSearchItem* gw_tabs_get_searchitem ()
{
    GList* iter;
    LwSearchItem *item;
    GtkBuilder *builder;
    GtkWidget *notebook;
    int page_num;

    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    iter = g_list_nth (_tab_searchitems, page_num);
    item = (LwSearchItem*) iter->data;

    return item;
}



void gw_tabs_set_searchitem (LwSearchItem *item)
{
    GList* iter;
    GtkBuilder *builder;
    GtkWidget *notebook;
    int page_num;

    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    iter = g_list_nth (_tab_searchitems, page_num);
    iter->data = item;
    gw_tabs_update_on_deck_historylist_by_searchitem (item);

    //Update the interface
    gw_tabs_guarantee_first ();
    gw_tabs_set_current_tab_text (item->queryline->string);
    gw_main_set_entry_text_by_searchitem (item);
}


//!
//! @brief Updates the tab and surrounding interface to reflect the current tab
//!
//! @param item pointer to a LwSearchItem to use to update the interface.
//!
void gw_tabs_update_appearance_with_searchitem (LwSearchItem *item)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

    //Set display status of tabs
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), (pages > 1));
    //gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), TRUE);
    
    GtkWidget *close_menuitem = GTK_WIDGET (gtk_builder_get_object (builder, "close_menuitem"));

    gtk_menu_item_set_label (GTK_MENU_ITEM (close_menuitem), gettext("_Close Tab"));
    //if (pages > 0) gtk_menu_item_set_label (GTK_MENU_ITEM (close_menuitem), gettext("_Close Tab"));
    //gtk_widget_set_sensitive (close_menuitem, (pages > 0));
    if (pages > 1) gtk_menu_item_set_label (GTK_MENU_ITEM (close_menuitem), gettext("_Close Tab"));
    else gtk_menu_item_set_label (GTK_MENU_ITEM (close_menuitem), gettext("_Close"));

    //Force correct querytext
    gw_main_set_entry_text_by_searchitem (item);
    gw_main_set_total_results_label_by_searchitem (item);
    gw_main_set_search_progressbar_by_searchitem (item);
    gw_main_set_entry_text_by_searchitem (item);
    gw_main_set_main_window_title_by_searchitem (item);
    gw_main_set_dictionary_by_searchitem (item);
}


//!
//! @brief Updates the tab and surrounding interface to reflect the current tab
//!
void gw_tabs_update_appearance ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int current_page;
    LwSearchItem *item;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    item = g_list_nth_data (_tab_searchitems, current_page);

    gw_tabs_update_appearance_with_searchitem (item);

    gw_main_set_font (NULL, NULL);
}


//!
//! @brief Sets the current SearchItem in the historylist to the one in the argument
//!
void gw_tabs_update_on_deck_historylist_by_searchitem (LwSearchItem *item)
{
    //Declaratios
    LwHistoryList *hl;

    //Initializations
    hl = lw_historylist_get_list (GW_TARGET_RESULTS);

    hl->current = item;
}


//!
//! @brief Sets the current SearchItem in the HistoryList to the one in the current tab
//!
void gw_tabs_update_on_deck_historylist_item_by_current_tab ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int page_num;
    LwSearchItem *item;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    item = g_list_nth_data (_tab_searchitems, page_num);

    gw_tabs_update_on_deck_historylist_by_searchitem (item);
}


//!
//! @brief Makes sure that at least one tab is available to output search results.
//!
void gw_tabs_guarantee_first ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int pages;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));

    if (pages == 0)
    {
      gw_tabs_new_cb (NULL, NULL);
      gw_tabs_update_appearance ();
    }
}


//!
//! @brief Sets the title text of the current tab.
//! @param TEXT The text to set to the tab
//!
void gw_tabs_set_current_tab_text (const char* TEXT)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int page_num;
    GtkWidget *container;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GList *hchildren;
    GList *vchildren;
    GtkWidget *label;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    container = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);
    hbox = GTK_WIDGET (gtk_notebook_get_tab_label(GTK_NOTEBOOK (notebook), GTK_WIDGET (container)));
    hchildren = gtk_container_get_children (GTK_CONTAINER (hbox));
    vbox = GTK_WIDGET (hchildren->data);
    vchildren = gtk_container_get_children (GTK_CONTAINER (vbox));
    label = GTK_WIDGET (vchildren->data);

    gtk_label_set_text (GTK_LABEL (label), TEXT);

    //Cleanup
    g_list_free (hchildren);
    g_list_free (vchildren);
}


//!
//! @brief Creats a new tab.  The focus and other details are handled by gw_tabs_new_cb ()
//!
int gw_tabs_new ()
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Create contents
    GtkWidget *scrolledwindow = GTK_WIDGET (gtk_scrolled_window_new (NULL, NULL));
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    GtkWidget *textview = GTK_WIDGET (gtk_text_view_new ());
    GObject *textbuffer = G_OBJECT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)));
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (textview), 10);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (textview), 10);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    //gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD);

    g_signal_connect( G_OBJECT (textview), "drag_motion", G_CALLBACK (gw_main_drag_motion_1_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "focus_in_event", G_CALLBACK (gw_main_update_clipboard_on_focus_change_cb), textview);
    g_signal_connect( G_OBJECT (textview), "button_press_event", G_CALLBACK (gw_main_get_position_for_button_press_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "motion_notify_event", G_CALLBACK (gw_main_get_iter_for_motion_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "drag_drop", G_CALLBACK (gw_main_drag_drop_1_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "button_release_event", G_CALLBACK (gw_main_get_iter_for_button_release_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "drag_leave", G_CALLBACK (gw_main_drag_leave_1_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "drag_data_received", G_CALLBACK (gw_main_search_drag_data_recieved_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "key_press_event", G_CALLBACK (gw_main_focus_change_on_key_press_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "populate_popup", G_CALLBACK (gw_main_populate_popup_with_search_options_cb), NULL);
    g_signal_connect( G_OBJECT (textview), "scroll_event", G_CALLBACK (gw_main_scroll_or_zoom_cb), NULL);

    gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
    gtk_widget_show_all (GTK_WIDGET (scrolledwindow));

    //Create create tab label
    GtkWidget *vbox;
    GtkWidget *hbox = GTK_WIDGET (gtk_hbox_new(FALSE, 3));
    GtkWidget *label = GTK_WIDGET (gtk_label_new(gettext("(Empty)")));
    GtkWidget *close_button = GTK_WIDGET (gtk_button_new ());
    gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
    GtkWidget *button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
    gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (close_button), 0);
    gtk_misc_set_padding (GTK_MISC (button_image), 0, 0);
    gtk_widget_set_size_request (GTK_WIDGET (button_image), 14, 14);

    //Declarations
    GtkCssProvider *provider;
    char *style_data;
    GtkStyleContext *context;

    //Initializations
    style_data = "* {\n"
                 "-GtkButton-default-border : 0;\n"
                 "-GtkButton-default-outside-border : 0;\n"
                 "-GtkButton-inner-border: 0;\n"
                 "-GtkWidget-focus-line-width : 0;\n"
                 "-GtkWidget-focus-padding : 0;\n"
                 "padding: 0;\n"
                 "}";
    provider = gtk_css_provider_new ();
    context = gtk_widget_get_style_context (close_button);

    gtk_css_provider_load_from_data (provider,  style_data, strlen(style_data), NULL); 
    gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    //Put all the elements together
    gtk_container_add (GTK_CONTAINER (close_button), button_image);
    g_signal_connect (G_OBJECT (close_button), "clicked", G_CALLBACK (gw_tabs_remove_cb), scrolledwindow);
    vbox = GTK_WIDGET (gtk_vbox_new(FALSE, 0));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 1);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
    vbox = GTK_WIDGET (gtk_vbox_new(FALSE, 0));
    gtk_box_pack_start (GTK_BOX (vbox), close_button, FALSE, FALSE, 1);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Finish
    int current = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    int position = gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, hbox);
    gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (notebook), scrolledwindow, TRUE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), position);
    gw_main_buffer_initialize_tags();
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), current);
    gw_main_buffer_initialize_marks (textbuffer);
    _tab_searchitems = g_list_append (_tab_searchitems, NULL);
    gw_tabs_update_appearance ();
    gw_main_update_toolbar_buttons ();

    return position;
}


//!
//! @brief Append a tag to the end of the tags
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_tabs_new_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    int position;
    GtkWidget *notebook;

    //Initializations
    builder = gw_common_get_builder ();
    position = gw_tabs_new ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));

    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), position);
    gw_main_set_entry_text_by_searchitem (NULL);
    gw_main_grab_focus_by_target (GW_TARGET_ENTRY);
    gw_main_set_dictionary(0);
    gw_tabs_update_on_deck_historylist_item_by_current_tab ();

    gw_tabs_update_appearance ();
}


//!
//! @brief Remove the tab where the close button is clicked
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_tabs_remove_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int pages;
    int page_num;
    GList *iter;
    LwSearchItem *item;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    page_num = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), GTK_WIDGET (data));

    //Sanity check
    if (pages < 2) gtk_main_quit ();

    gw_main_cancel_search_by_tab_number (page_num);
    gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);

    iter = g_list_nth (_tab_searchitems, page_num);
    if (iter != NULL)
    {
      item = iter->data;
      if (iter->data != NULL && item->total_results > 0)
      {
        lw_historylist_add_searchitem_to_history (GW_HISTORYLIST_RESULTS, item);
        gw_main_update_history_popups ();
      }
      else if (item != NULL)
      {
        lw_searchitem_free (item);
        iter->data = NULL;
        gw_main_grab_focus_by_target (GW_TARGET_ENTRY);
      }
    }
    _tab_searchitems = g_list_delete_link (_tab_searchitems, iter);

    gw_tabs_update_on_deck_historylist_item_by_current_tab ();
    gw_tabs_update_appearance ();

    if (pages == 1) gw_main_grab_focus_by_target (GW_TARGET_ENTRY);
}


//!
//! @brief Remove the tab where the close button is clicked
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_tabs_remove_current_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int pages;
    int page_num;
    GList *iter;
    LwSearchItem *item;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    iter = g_list_nth (_tab_searchitems, page_num);

    if (pages < 2) gtk_main_quit ();

    gw_main_cancel_search_by_tab_number (page_num);

    if (iter != NULL)
    {
      item = iter->data;
      if (item != NULL && item->total_results > 0)
      {
        lw_historylist_add_searchitem_to_history (GW_HISTORYLIST_RESULTS, item);
        gw_main_update_history_popups ();
      }
      else if (item != NULL)
      {
        lw_searchitem_free (item);
        item = NULL;
      }
    }
    _tab_searchitems = g_list_delete_link (_tab_searchitems, iter);

    gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);
    gw_tabs_update_on_deck_historylist_item_by_current_tab ();
    gw_tabs_update_appearance ();

    if (pages == 1) gw_main_grab_focus_by_target (GW_TARGET_ENTRY);
}


//!
//! @brief Do the side actions required when a tab switch takes place
//!
//! Various side elements should be updated when at tab switch occurs
//! such as the progress bar, querybar, dictionry selection etc.
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_tabs_switch_cb (GtkNotebook *notebook, GtkWidget *page, int page_num, gpointer data)
{
    //Declarations
    LwSearchItem *item;

    //Initializations
    item = g_list_nth_data (_tab_searchitems, page_num);

    gw_tabs_update_appearance_with_searchitem (item);
    gw_tabs_update_on_deck_historylist_by_searchitem (item);
}


//!
//! @brief Cycles to the next tab 
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_tabs_next_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));

    gtk_notebook_next_page (GTK_NOTEBOOK (notebook));
    gw_tabs_update_appearance ();
    gw_tabs_update_on_deck_historylist_item_by_current_tab ();
}


//!
//! @brief Cycles to the previous tab 
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_tabs_previous_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));

    gtk_notebook_prev_page (GTK_NOTEBOOK (notebook));
    gw_tabs_update_appearance ();
    gw_tabs_update_on_deck_historylist_item_by_current_tab ();
}


//!
//! @brief Sets up an initites a new search in a new tab
//!
//! @param widget Currently unused widget pointer
//! @param data A gpointer to a LwSearchItem that hold the search information
//!
G_MODULE_EXPORT void gw_tabs_new_with_search_cb (GtkWidget *widget, gpointer data)
{
  printf("new with search\n");
    //Declarations
    GtkBuilder *builder;
    LwSearchItem *item;
    LwSearchItem *current_item;
    GtkWidget *notebook;
    int page_num;

    //Initializations
    builder = gw_common_get_builder ();
    item = (LwSearchItem*) data;
    current_item = g_list_nth_data (gw_tabs_get_searchitem_list (), page_num);
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));

    if (item != NULL)
    {
      gw_tabs_new_cb (NULL, NULL);
      gw_main_set_dictionary_by_searchitem (item);
      gw_main_set_entry_text_by_searchitem (item);
      gw_main_search_cb (NULL, NULL);
    }
}


G_MODULE_EXPORT void gw_tabs_no_results_search_for_dictionary_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    LwDictInfo* di;

    //Initializations
    di = (LwDictInfo*) data;

    gw_main_set_dictionary (di->load_position);
}

//!
//! @brief Frees the LwSearchItem memory that is attached to the activate tab callback
//!
//! @param widget Currently unused widget pointe
//! @param data gpointer to a LwSearchItem to be freed
//!
G_MODULE_EXPORT void gw_tabs_destroy_tab_menuitem_searchitem_data_cb (GObject *object, gpointer data)
{
    //Declarations
    LwSearchItem *item;

    //Initializations
    item = (LwSearchItem*) data;

    if (item != NULL)
    {
      lw_searchitem_free (item);
      item = NULL;
    }
}


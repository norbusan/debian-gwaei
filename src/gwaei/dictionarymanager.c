#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static GtkListStore *_model = NULL;
static GtkTreeView *_view = NULL;
enum { IMAGE, POSITION, NAME, LONG_NAME, ENGINE, SHORTCUT, DICT_POINTER, TOTAL_FIELDS };
static gulong _list_update_handler_id;


G_MODULE_EXPORT void gw_dictionarymanager_list_store_row_changed_action_cb (GtkTreeModel *model,
                                                                            GtkTreePath *path,
                                                                            gpointer data)
{
    g_signal_handler_block (_model, _list_update_handler_id);
    int position = 0;
    LwDictInfo *di = NULL;
    gpointer ptr;
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (_model), &iter))
    {
      do {
        gtk_tree_model_get (GTK_TREE_MODEL (_model), &iter, DICT_POINTER, &ptr, -1);
        if (ptr != NULL)
        {
          di = (LwDictInfo*) ptr;
          di->load_position = position;
          position++;
        }
      }
      while (gtk_tree_model_iter_next (GTK_TREE_MODEL (_model), &iter));
    }
    lw_dictinfolist_save_dictionary_order_pref ();
    gw_dictionarymanager_update_items ();
    g_signal_handler_unblock (_model, _list_update_handler_id);
}



//!
//! @brief Sets up the dictionary manager.  This is the backbone of every portion of the GUI that allows editing dictionaries
//!
void gw_dictionarymanager_initialize ()
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    //Setup the model and view
    _model = gtk_list_store_new (
        TOTAL_FIELDS, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_STRING, 
        G_TYPE_POINTER);

    _view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "manage_dictionaries_treeview"));
    gtk_tree_view_set_model (GTK_TREE_VIEW (_view), GTK_TREE_MODEL (_model));

    //Create the columns and renderer for each column
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, " ");
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, "icon-name", IMAGE, NULL);
    gtk_tree_view_append_column (_view, column);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new_with_attributes ("#", renderer, "text", POSITION, NULL);
    gtk_tree_view_append_column (_view, column);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new_with_attributes (gettext("Name"), renderer, "text", LONG_NAME, NULL);
    gtk_tree_view_column_set_min_width (column, 100);
    gtk_tree_view_append_column (_view, column);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new_with_attributes (gettext("Engine"), renderer, "text", ENGINE, NULL);
    gtk_tree_view_append_column (_view, column);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
    column = gtk_tree_view_column_new_with_attributes (gettext("Shortcut"), renderer, "text", SHORTCUT, NULL);
    gtk_tree_view_append_column (_view, column);

    GtkWidget *combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (_model));
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, FALSE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", LONG_NAME);

    gw_dictionarymanager_update_items ();
    _list_update_handler_id = g_signal_connect (G_OBJECT (_model), "row-deleted", 
                                                G_CALLBACK (gw_dictionarymanager_list_store_row_changed_action_cb), NULL);
}


void gw_dictionarymanager_free ()
{
  g_object_unref (_model);
  gtk_widget_destroy (GTK_WIDGET (_view));
}


//!
//! Sets updates the list of dictionaries against the list in the global dictlist
//!
void gw_dictionarymanager_update_items ()
{
    if (_list_update_handler_id > 0) g_signal_handler_block (_model, _list_update_handler_id);
    GtkBuilder *builder = gw_common_get_builder ();

    //Clear the previous items all over the gui
    gtk_list_store_clear (GTK_LIST_STORE (_model));

    GtkMenuShell *shell = GTK_MENU_SHELL (gtk_builder_get_object (builder, "dictionary_popup"));
    if (shell != NULL)
    {
      GList     *children = NULL;
      children = gtk_container_get_children (GTK_CONTAINER (shell));
      while (children != NULL )
      {
        gtk_widget_destroy(children->data);
        children = g_list_delete_link (children, children);
      }
    }

    //Start filling in the new items
    LwDictInfo *di = NULL;
    GtkTreeIter treeiter;
    char *longname = NULL;
    char *iconname = NULL;
    char *shortcutname = NULL;
    char *order_number = NULL;
    char *favorite_icon = "emblem-favorite";
    GList *iter = NULL;
    GtkAccelGroup* accel_group = GTK_ACCEL_GROUP (gtk_builder_get_object (builder, "main_accelgroup"));
    GSList* group = NULL;
    GtkWidget *item = NULL;

    
    for (iter = lw_dictinfolist_get_list(); iter != NULL; iter = iter->next)
    {
      //Recreate the liststore
      di = (LwDictInfo*) iter->data;
      if (di->load_position == 0) iconname = favorite_icon;
      if (di->load_position < 9) shortcutname = g_strdup_printf ("Alt-%d", (di->load_position + 1));
      order_number = g_strdup_printf ("%d", (di->load_position + 1));
      gtk_list_store_append (GTK_LIST_STORE (_model), &treeiter);
      gtk_list_store_set (_model, &treeiter,
                          IMAGE, iconname,
                          POSITION, order_number,
                          NAME, di->shortname,
                          LONG_NAME, di->longname,
                          ENGINE, lw_util_get_engine_name (di->engine),
                          SHORTCUT, shortcutname,
                          DICT_POINTER, di,
                                                 -1);

      //Refill the menu
      item = GTK_WIDGET (gtk_radio_menu_item_new_with_label (group, di->longname));
      group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
      gtk_menu_shell_append (GTK_MENU_SHELL (shell),  GTK_WIDGET (item));
      if (di->load_position == 0) gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
      g_signal_connect(G_OBJECT (item), "toggled", G_CALLBACK (gw_main_dictionary_changed_action_cb), NULL);
      if (di->load_position < 9) gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, (GDK_KEY_0 + di->load_position + 1), GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
      gtk_widget_show (item);

      //Cleanup
      g_free (order_number);
      order_number = NULL;
      iconname = NULL;
      g_free (shortcutname);
      shortcutname = NULL;
    }

    //Fill in the other menu items
    item = GTK_WIDGET (gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    gtk_widget_show (GTK_WIDGET (item));

    item = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("_Cycle Up")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (gw_main_cycle_dictionaries_backward_cb), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, GDK_KEY_Up, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (item));

    item = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("Cycle _Down")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (gw_main_cycle_dictionaries_forward_cb), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, GDK_KEY_Down, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (item));

    GtkWidget *combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
    if (_list_update_handler_id > 0) g_signal_handler_unblock (_model, _list_update_handler_id);
}



G_MODULE_EXPORT void gw_dictionarymanager_cursor_changed_cb (GtkTreeView *treeview, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "remove_dictionary_button"));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (_view);
    GtkTreeIter iter;

    GtkTreeModel *tmodel = GTK_TREE_MODEL (_model);
    gboolean has_selection = gtk_tree_selection_get_selected (selection, &tmodel, &iter);
    gtk_widget_set_sensitive (GTK_WIDGET (button), has_selection);
}


G_MODULE_EXPORT void gw_dictionarymanager_remove_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *button;
    GtkTreeViewColumn *focus_column;
    GtkTreePath *path;
    GtkTreeIter iter;
    GList *list;
    GError *error;
    GtkTreeSelection *selection;
    GtkTreeModel *tmodel;
    gboolean has_selection;
    gint* indices;
    LwDictInfo *di;

    //Initializations
    builder = gw_common_get_builder ();
    button = GTK_WIDGET (gtk_builder_get_object (builder, "remove_dictionary_button"));
    selection = gtk_tree_view_get_selection (_view);
    tmodel = GTK_TREE_MODEL (_model);
    has_selection = gtk_tree_selection_get_selected (selection, &tmodel, &iter);
    error = NULL;

    //Sanity check
    if (!has_selection) return;

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (_model), &iter);
    indices = gtk_tree_path_get_indices (path);
    list = lw_dictinfolist_get_dict_by_load_position (*indices);

    if (list != NULL)
    {
      di = list->data;
      lw_dictinfo_uninstall (di, NULL, &error);
      gw_dictionarymanager_update_items ();
    }

    //Cleanup
    gtk_tree_path_free (path);

    gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
}


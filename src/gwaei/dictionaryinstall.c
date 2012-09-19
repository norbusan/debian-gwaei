#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static GtkListStore *_encoding_store = NULL;
static GtkListStore *_compression_store = NULL;
static GtkListStore *_engine_store = NULL;
static GtkListStore *_dictionary_store = NULL;

enum { SHORT_NAME, LONG_NAME, DICTINST_PTR, CHECKBOX_STATE, DICTIONARY_STORE_TOTAL_FIELDS };
enum { ENGINE_STORE_ID, ENGINE_STORE_NAME, ENGINE_STORE_TOTAL_FIELDS };
enum { COMPRESSION_STORE_ID, COMPRESSION_STORE_NAME, COMPRESSION_STORE_TOTAL_FIELDS };
enum { ENCODING_STORE_ID, ENCODING_STORE_NAME, ENCODING_STORE_TOTAL_FIELDS };

static LwDictInst *_di = NULL;

static void _update_add_button_sensitivity (void);

static void _clear_details_box ()
{
    GtkBuilder *builder;
    GtkWidget *hbox;
    GList *children;
    GList *iter;

    builder = gw_common_get_builder ();
    hbox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_details_hbox"));
    children = gtk_container_get_children (GTK_CONTAINER (hbox));

    //Clear the GUI elements
    for (iter = children; iter != NULL; iter = iter->next)
    {
      gtk_widget_destroy (GTK_WIDGET (iter->data));
    }
    g_list_free (children);
    children = NULL;
}


static void _fill_details_box (LwDictInst *di)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *parent = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_details_hbox"));
    GtkWidget *button = NULL;
    GtkWidget *image = NULL;
    GtkWidget *entry = NULL;
    GtkWidget *label = NULL;
    GtkWidget *table = NULL;
    GtkWidget *hbox = NULL;
    GtkWidget *combobox = NULL;
    GtkWidget *checkbox = NULL;
    GtkCellRenderer *renderer = NULL;
    gchar *markup = NULL;
    gboolean editable;

    table = gtk_table_new (7, 2, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 1);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    editable = !di->builtin;

    //First row
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    markup = g_strdup_printf(gettext("<b>%s Install Details</b>"), di->longname);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), markup);
    g_free (markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (hbox), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Second row
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    markup = g_strdup_printf("%s", di->description);
    label = gtk_label_new (NULL);
    gtk_widget_set_size_request (GTK_WIDGET (label), 300, -1);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_markup(GTK_LABEL (label), markup);
    g_free (markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (hbox), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Third row
    label = gtk_label_new (gettext("Filename: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 0, 1);
    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), di->filename);
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstall_filename_entry_changed_cb), di);
    gtk_table_attach_defaults (GTK_TABLE (table), entry, 1, 2, 0, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (entry), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Forth row
    label = gtk_label_new (gettext("Engine: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 1, 2);
    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", ENGINE_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (_engine_store));

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 1, 2);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->engine);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstall_engine_combobox_changed_cb), di);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);


    //Fifth row
    label = gtk_label_new (gettext("Source: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 2, 3);

    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), di->uri[GW_DICTINST_NEEDS_DOWNLOADING]);
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstall_source_entry_changed_cb), di);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (entry), TRUE, TRUE, 0);
    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_dictionaryinstall_select_file_cb), entry);
    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    gtk_widget_set_sensitive (GTK_WIDGET (button), !editable);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 2, 3);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_dictionaryinstall_reset_default_uri_cb), di);

    //Sixth row
    label = gtk_label_new (gettext("Encoding: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 3, 4);
    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", ENCODING_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (_encoding_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->encoding);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstall_encoding_combobox_changed_cb), di);

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 3, 4);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Seventh row
    label = gtk_label_new (gettext("Compression: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 4, 5);
    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", COMPRESSION_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (_compression_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->compression);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstall_compression_combobox_changed_cb), di);

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 4, 5);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Eighth row
    checkbox = gtk_check_button_new_with_label (gettext("Split Places from Names Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->split);
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstall_split_checkbox_changed_cb), di);
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 2, 5, 6);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Ninth row
    checkbox = gtk_check_button_new_with_label (gettext("Merge Radicals into Kanji Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->merge);
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstall_merge_checkbox_changed_cb), di);
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 2, 6, 7);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (table), FALSE, FALSE, 5);
    gtk_widget_show_all (GTK_WIDGET (table));

    _di = di;
}


void gw_dictionaryinstall_initialize ()
{
    gw_common_load_ui_xml ("dictionaryinstall.ui");

    GtkBuilder *builder = gw_common_get_builder ();
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeView *view;
    GList *list;
    LwDictInst *di;
    GtkTreeIter treeiter;
    int i;

    //Set up the dictionary liststore
    _dictionary_store = gtk_list_store_new (DICTIONARY_STORE_TOTAL_FIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
    for (list = lw_dictinstlist_get_list (); list != NULL; list = list->next)
    {
      di = (LwDictInst*) list->data;
      gtk_list_store_append (GTK_LIST_STORE (_dictionary_store), &treeiter);
      gtk_list_store_set (
        _dictionary_store, &treeiter,
        SHORT_NAME, di->shortname,
        LONG_NAME, di->longname,
        DICTINST_PTR, di,
        CHECKBOX_STATE, FALSE, 
        -1
      );
    }

    //Set up the Engine liststore
    _engine_store = gtk_list_store_new (ENGINE_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < GW_ENGINE_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (_engine_store), &treeiter);
      gtk_list_store_set (
        _engine_store, &treeiter,
        ENGINE_STORE_ID, i,
        ENGINE_STORE_NAME, lw_util_get_engine_name(i),
        -1
      );
    }

    //Set up the Compression liststore
    _compression_store = gtk_list_store_new (COMPRESSION_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < GW_COMPRESSION_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (_compression_store), &treeiter);
      gtk_list_store_set (
        _compression_store, &treeiter,
        COMPRESSION_STORE_ID, i,
        COMPRESSION_STORE_NAME, lw_util_get_compression_name(i),
        -1
      );
    }

    //Set up the Encoding liststore
    _encoding_store = gtk_list_store_new (ENCODING_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < GW_ENCODING_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (_encoding_store), &treeiter);
      gtk_list_store_set (
        _encoding_store, &treeiter,
        ENCODING_STORE_ID, i,
        ENCODING_STORE_NAME, lw_util_get_encoding_name(i),
        -1
      );
    }

    //Setup the dictionary list view
    view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "dictionary_install_treeview"));
    gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (_dictionary_store));
    g_signal_connect (G_OBJECT (view), "cursor-changed", G_CALLBACK (gw_dictionaryinstall_cursor_changed_cb), NULL);

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 5);
    column = gtk_tree_view_column_new_with_attributes (" ", renderer, "active", CHECKBOX_STATE, NULL);
    gtk_tree_view_append_column (view, column);
    g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (gw_dictionaryinstall_listitem_toggled_cb), NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 0);
    column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", LONG_NAME, NULL);
    gtk_tree_view_append_column (view, column);
}

void gw_dictionaryinstall_free ()
{
    g_object_unref (_encoding_store);
    _encoding_store = NULL;

    g_object_unref (_compression_store);
    _compression_store = NULL;

    g_object_unref (_engine_store);
    _engine_store = NULL;

    g_object_unref (_dictionary_store);
    _dictionary_store = NULL;
}


//!
//! @brief opens the dictionary install dialog
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_dictionaryinstall_show_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));
    GtkWidget *settings_window = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window" ));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (settings_window));
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show (GTK_WIDGET (dialog));
}

/*

G_MODULE_EXPORT void do_toggle_other_dictionary_show (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *checkbox = GTK_WIDGET (data);
    GtkWidget *table = GTK_WIDGET (widget);
    GtkWidget *dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));

    gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbox));
    if (active) gtk_widget_show (table);
    else gtk_widget_hide (table);
}
*/


//CALLBACKS//////////////////////////////////////

G_MODULE_EXPORT void gw_dictionaryinstall_filename_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    const char *value = gtk_entry_get_text (GTK_ENTRY (widget));

    lw_dictinst_set_filename (di, value);

    _update_add_button_sensitivity ();
}

G_MODULE_EXPORT void gw_dictionaryinstall_engine_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    int value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    di->engine = value;
}

G_MODULE_EXPORT void gw_dictionaryinstall_source_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    const char *value = gtk_entry_get_text (GTK_ENTRY (widget));

    //Set the LwDictInst value
    lw_dictinst_set_download_source (di, value);

    //Update the preference if approprate
    if (di->schema != NULL && di->key != NULL)
    {
      lw_pref_set_string_by_schema (di->schema, di->key, value);
    }

    _update_add_button_sensitivity ();
}


G_MODULE_EXPORT void gw_dictionaryinstall_reset_default_uri_cb (GtkWidget *widget, gpointer data)
{
    LwDictInst* di = (LwDictInst*) data;
    char value[200];

    if (di->schema == NULL || di->key == NULL) return;

    lw_pref_reset_value_by_schema (di->schema, di->key);
}


G_MODULE_EXPORT void gw_dictionaryinstall_select_file_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *dialog;
    GtkWidget *entry;

    builder = gw_common_get_builder ();
    entry = GTK_WIDGET (data);
    window = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));
    dialog = gtk_file_chooser_dialog_new (
      "Select File",
      GTK_WINDOW (window),
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL
    );
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      gtk_entry_set_text (GTK_ENTRY (entry), filename);
      g_free (filename);
    }
    gtk_widget_hide (dialog);
}


G_MODULE_EXPORT void gw_dictionaryinstall_encoding_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    int value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictinst_set_encoding (di, value);
}

G_MODULE_EXPORT void gw_dictionaryinstall_compression_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    int value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictinst_set_compression (di, value);
}

G_MODULE_EXPORT void gw_dictionaryinstall_split_checkbox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    gboolean value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictinst_set_split (di, value);
}

G_MODULE_EXPORT void gw_dictionaryinstall_merge_checkbox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    gboolean value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictinst_set_merge (di, value);
}





G_MODULE_EXPORT void gw_dictionaryinstall_cursor_changed_cb (GtkTreeView *view, gpointer data)
{
    //Declarations
    GtkTreeSelection *selection;
    GtkWidget *checkbox;
    GtkWidget *hbox;
    GtkWidget *dialog;
    LwDictInst *di;
    GtkTreeIter iter;
    gboolean show_details;
    gboolean has_selection;
    GtkTreeModel *model;
    GtkBuilder *builder;
    gboolean editable;
    int height;
    int width;

    //Initializations
    builder = gw_common_get_builder ();
    checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "show_dictionary_detail_checkbutton"));
    hbox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_details_hbox"));
    dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
    model = GTK_TREE_MODEL (_dictionary_store);
    has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
    show_details = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbox));
    editable = FALSE;

    //Set the approprate contents of the detail pane
    if (has_selection)
    {
      _clear_details_box();
      gtk_tree_model_get (model, &iter, DICTINST_PTR, &di, -1);
      _fill_details_box (di);
      editable = !di->builtin;
    }

    //Set the approprate show/hide state of the detail pane
    if (has_selection && (show_details || editable))
      gtk_widget_show (hbox);
    else
      gtk_widget_hide (hbox);

    //Make the window shrink if the detail pane disappeared
    gtk_window_get_size (GTK_WINDOW (dialog), &width, &height);
    gtk_window_resize (GTK_WINDOW (dialog), 1, height);
}


G_MODULE_EXPORT void gw_dictionaryinstall_listitem_toggled_cb (GtkCellRendererToggle *renderer, 
                                                               gchar *path,
                                                               gpointer data)
{
    GtkTreeIter iter;
    gboolean state;
    LwDictInst *di;
    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (_dictionary_store), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (_dictionary_store), &iter, CHECKBOX_STATE, &state, DICTINST_PTR, &di, -1);
    gtk_list_store_set (GTK_LIST_STORE (_dictionary_store), &iter, CHECKBOX_STATE, !state, -1);
    di->selected = !state;

    _update_add_button_sensitivity ();
}


G_MODULE_EXPORT void gw_dictionaryinstall_detail_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkTreePath *path;
    GtkTreeViewColumn *column;
    GtkBuilder *builder;
    GtkTreeView *view;

    //Declarations
    builder = gw_common_get_builder ();
    view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "dictionary_install_treeview"));

    //Trigger the list item selection callback
    gtk_tree_view_get_cursor (view, &path, &column);
    gtk_tree_view_set_cursor (view, path, column, FALSE);

    //Cleanup
    gtk_tree_path_free (path);
}


//!
//! @brief Checks the validity of the LwDictInst data and sets the add button sensitivity accordingly
//!
static void _update_add_button_sensitivity ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *button;
    gboolean sensitivity;

    //Initializations
    builder = gw_common_get_builder ();
    button = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_add_button"));
    sensitivity = lw_dictinstlist_data_is_valid ();

    //Finalize
    gtk_widget_set_sensitive (button, sensitivity);
}


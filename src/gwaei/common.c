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
//! @file src/common.c
//!
//! @brief Common functions used between many files
//!


#include <stdlib.h>
#include <string.h>

#include <gwaei/gwaei.h>

static GtkBuilder *_builder = NULL;
static GList* _load_window_prefs (void);
static void _load_window_prefs_for_id (const char*, int*, int*, int*, int*);
static void _save_window_prefs_for_id (const char*);

struct _GwWindowData
{
  char *id;
  int x;
  int y;
  int width;
  int height;
};
typedef struct _GwWindowData GwWindowData;

GwWindowData* gw_windowdata_new (const char* id, int x, int y, int width, int height)
{
  if (id == NULL) return NULL;

  GwWindowData *temp = (GwWindowData*) malloc (sizeof(GwWindowData));

  temp->id = g_strdup (id);
  temp->x = x;
  temp->y = y;
  temp->width = width;
  temp->height = height;
  
  return temp;
}

void gw_windowdata_free (GwWindowData *wd)
{
  g_free(wd->id);
  free(wd);
}

void gw_common_initialize ()
{
  _builder = gtk_builder_new ();
}

void gw_common_free ()
{
  g_object_unref (_builder);
  _builder = NULL;
}


//!
//! @brief Gets the window position data for a specific window id as given in the gtkbuilder xml
//!
//! @param id The gtkbuilder id for the window
//! @param x An integer pointer to fill with the x position
//! @param y An integer pointer to fill with the y position
//! @param width An integer pointer to fill with the width position
//! @param height An integer pointer to fill with the height position
//!
static void _load_window_prefs_for_id (const char* id, int* x, int* y, int* width, int* height)
{
    GwWindowData *wd = NULL;
    //Get the window pref data
    GList *list = _load_window_prefs ();
    GList *iter = NULL;
    for (iter = list; iter != NULL; iter = iter->next)
    {
      wd = (GwWindowData*) iter->data;
      if (strcmp(wd->id, id) == 0) break;
    }
    if (iter != NULL) //Doesn't exist
    {
      *x = wd->x;
      *y = wd->y;
      *width = wd->width;
      *height = wd->height;
    }
    for (iter = list; iter != NULL; iter = iter->next) gw_windowdata_free ((GwWindowData*)(iter->data));
    g_list_free (list);
    list = NULL;
}


//!
//! @brief Sets the position preferences of the window
//!
//! @param window_id The gtkbuilder window id which also acts as a preference key
//!
static void _initialize_window_attributes (char* id)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, id));
    int x = 0, y = 0, width = 100, height = 100;

    _load_window_prefs_for_id (id, &x, &y, &width, &height);

    //Apply the height and width if they are sane
    if (strcmp (id, "main_window") == 0)
    {
      //Make sure the size is semi-sane
      if (width >= 100 && width <= gdk_screen_width() && height >= 100 && height <= gdk_screen_height()) 
      {
        gtk_window_resize (GTK_WINDOW (window), width, height);
      }
      //Center the window if it is partially off screen
    }
    else if (strcmp (id, "radicals_window") == 0)
    {
      //Height checking
      GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, "radicals_window"));
      GtkWidget *scrolledwindow = GTK_WIDGET (gtk_builder_get_object (builder, "radical_selection_scrolledwindow"));

      //Get the natural size
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
      gtk_widget_set_size_request (GTK_WIDGET (window), -1, -1);
      gtk_widget_queue_draw (GTK_WIDGET (window));
      int width, height;
      gtk_window_get_size (GTK_WINDOW (window), &width, &height);

      //Set the final size
      if (height > gdk_screen_height ())
        gtk_widget_set_size_request (GTK_WIDGET (window), -1, gdk_screen_height() - 100);
      else
        gtk_widget_set_size_request (GTK_WIDGET (window), -1, height);

      //Allow a vertical scrollbar
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    }

    //Apply the x and y if they are within the screen size
    if (x > 0 && x < gdk_screen_width() && y > 0 && y < gdk_screen_height()) {
      gtk_window_move (GTK_WINDOW (window), x, y);
    }
    //Otherwise center the window
    else
    {
      int half_width, half_height;
      gtk_window_get_size (GTK_WINDOW (window), &width, &height);
      half_width = (gdk_screen_width() / 2) - (width / 2);
      half_height =  (gdk_screen_height() / 2) - (height / 2);
      gtk_window_move (GTK_WINDOW (window), half_width, half_height);
    }

}


//!
//! @brief Returns a glist of all of the saved window position and size data.
//!
//! @returns A GList containing GwWindowData objects
//!
static GList* _load_window_prefs ()
{
    char* pref = (char*) malloc(300 * sizeof(char));
    lw_pref_get_string_by_schema (pref, GW_SCHEMA_BASE, GW_KEY_WINDOW_POSITIONS, 300);
    char **pref_array = NULL;;
    pref_array = g_strsplit (pref, ";", -1);
    char **ptr = NULL;
    char **window_data = NULL;
    GList *list = NULL;
    GwWindowData* wd = NULL;
    
    //Format of pref is "window:xcoord,ycoord,width,height;"
    for (ptr = pref_array; *ptr != NULL; ptr++)
    {
      window_data = g_strsplit (*ptr, ",", -1);
      if (g_strv_length(window_data) == 5)
      {
        char* id = window_data[0];
        int x = (int) g_ascii_strtoll (window_data[1], NULL, 10);
        int y = (int) g_ascii_strtoll (window_data[2], NULL, 10);
        int width = (int) g_ascii_strtoll (window_data[3], NULL, 10);
        int height = (int) g_ascii_strtoll (window_data[4], NULL, 10);
        wd = gw_windowdata_new (id, x, y, width, height);
        if (wd != NULL) list = g_list_append (list, (gpointer) wd);
      }
      g_strfreev(window_data);
      window_data = NULL;
    }

    g_strfreev (pref_array);
    pref_array = NULL;
    free(pref);
    pref = NULL;

    return list;
}


//!
//! @brief Saves the window position and size settings identified by its id
//!
//! @param id An id identifying the window as set in the gtkbuilder xml
//!
static void _save_window_prefs_for_id (const char *id)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, id));
    GList *list = _load_window_prefs();
    GList *iter = NULL;
    GwWindowData *wd = NULL;
    int x, y, width, height;
    gtk_window_get_position (GTK_WINDOW (window), &x, &y);
    gtk_window_get_size (GTK_WINDOW (window), &width, &height);

    g_assert (id != NULL && window != NULL);

    //Update the window data
    for (iter = list; iter != NULL; iter = iter->next)
    {
      wd = (GwWindowData*) iter->data;
      if (strcmp(id, wd->id) == 0)
      {
        wd->x = x;
        wd->y = y;
        wd->width = width;
        wd->height = height;
        break;
      }
    }
    if (iter == NULL) //This condition only occures when there is no stored window data to update
    {
      wd = gw_windowdata_new (id, x, y, width, height);
      list = g_list_append (list, (gpointer) wd);
    }

    //Create a string reflecting the updated values
    char *pref = NULL;
    char** atoms = (char**) malloc((g_list_length (list) + 1) * sizeof(char*));
    char** atoms_iter = atoms;
    for (iter = list; iter != NULL; iter = iter->next)
    {
      wd = (GwWindowData*) iter->data;
      *atoms_iter = g_strdup_printf("%s,%d,%d,%d,%d", wd->id, wd->x, wd->y, wd->width, wd->height);
      gw_windowdata_free (wd);
      atoms_iter++;
    }
    *atoms_iter = NULL;
    g_list_free (list);

    pref = g_strjoinv (";", atoms);
    lw_pref_set_string_by_schema (GW_SCHEMA_BASE, GW_KEY_WINDOW_POSITIONS, pref);
    g_free (pref);
    pref = NULL;

    g_strfreev (atoms);
    atoms = NULL;
}


//!
//! @brief Saves the coordinates of a windomw, then closes it
//!
//! @param window_id The gtkbuilder id of the window
//!
void gw_common_hide_window (const char* id)
{
    g_assert (id != NULL);
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, id));

    _save_window_prefs_for_id (id);
    gtk_widget_hide (window);
}


//!
//! @brief Shows a window, using the prefs to initialize it's position and does some other checks too
//!
//! @param The gtkbuilder id of the window
//!
void gw_common_show_window (char *id)
{
    g_assert (id != NULL);

    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, id));
    GtkWidget *main_window;
    if (gtk_widget_get_visible (window) == TRUE) return;

    if (strcmp(id, "main_window") == 0 || strcmp (id, "radicals_window") == 0 || strcmp (id, "kanjipad_window") == 0)
    {
      if (strcmp (id, "radicals_window") == 0 || strcmp (id, "kanjipad_window") == 0)
      {
        main_window = GTK_WIDGET (gtk_builder_get_object(builder, "main_window"));
        gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (main_window));
        gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_UTILITY);
      }
      _initialize_window_attributes (id);
      gtk_widget_show (window);
      _initialize_window_attributes (id);
    }
    else if (strcmp (id, "settings_window") == 0)
    {
      main_window = GTK_WIDGET (gtk_builder_get_object(builder, "main_window"));
      gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (main_window));
      gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
      gtk_widget_show (window);
    }
    else
    {
      gtk_widget_show (window);
    }
}


//!
//! @brief Gets the GtkBuilder pointer
//!
GtkBuilder* gw_common_get_builder ()
{
    return _builder;
}



//!
//! @brief Loads the gtk builder xml file from the usual paths
//!
//! @param filename The filename of the xml file to look for
//!
gboolean gw_common_load_ui_xml (const char *filename) {
    g_assert (_builder != NULL);

    //Declarations
    char *paths[4];
    char **iter;
    char *path;
    gboolean loaded;

    //Initializations
    paths[0] = g_build_filename ("ui", filename, NULL);
    paths[1] = g_build_filename ("..", "share", PACKAGE, filename, NULL);
    paths[2] = g_build_filename (DATADIR2, PACKAGE, filename, NULL);
    paths[3] = NULL;
    loaded = FALSE;

    //Search for the files
    for (iter = paths; *iter != NULL && loaded == FALSE; iter++)
    {
      path = *iter;
      if (g_file_test (path, G_FILE_TEST_IS_REGULAR) && gtk_builder_add_from_file (_builder, path,  NULL))
      {
        gtk_builder_connect_signals (_builder, NULL);
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


//!
//! @brief Retrieves a special GtkWidget designated by the LwTargetOuput signature
//!
//! This function would get the target textview instead of the textbuffer.
//! The focus is on getting the frontend widget.
//!
//! @param TARGET A LwTargetOutput designating the target
//!
GtkWidget* gw_common_get_widget_by_target (const LwTargetOutput TARGET)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *container;
    GtkWidget *notebook;
    GtkWidget *page;
    int page_number;

    switch (TARGET)
    {
      case GW_TARGET_RESULTS:
        notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
        page_number =  gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
        if (page_number == -1) return NULL;
        page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_number);
        if (page == NULL) return NULL;
        return gtk_bin_get_child (GTK_BIN (page));
      case GW_TARGET_ENTRY:
        return GTK_WIDGET (gtk_builder_get_object (builder, "search_entry"));
      default:
        return NULL;
    }
}


//!
//! @brief To be written
//!
gboolean gw_common_widget_equals_target (GtkWidget* widget, const LwTargetOutput TARGET)
{
    GtkWidget* target;
    target = gw_common_get_widget_by_target (TARGET);
    return (GTK_WIDGET (widget) == GTK_WIDGET (target));
}


//!
//! @brief Retrieves a special gobject designated by the LwTargetOuput signature
//!
//! This function would get the target textbuffer instead of the targettext view for example.
//! The focus is on getting the backend widget.
//!
//! @param TARGET A LwTargetOutput designating the target
//!
gpointer gw_common_get_gobject_by_target (const LwTargetOutput TARGET)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *notebook;
    GtkWidget *page;
    int page_number;
    GtkWidget *view;

    switch (TARGET)
    {
      case GW_TARGET_RESULTS:
          notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
          page_number = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
          if (page_number == -1) return NULL;
          page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_number);
          if (page == NULL) return NULL;
          if ((view = gtk_bin_get_child (GTK_BIN (page))) == NULL) return NULL;
          return G_OBJECT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
      default:
          return NULL;
    }
}


//!
//! @brief Optionally displays an error dialog and frees memory from an error
//!
//! @param error A pointer to an error pointer
//! @param show_dialog Whether to show a dialog or not.
//!
void gw_common_handle_error (GError **error, GtkWindow *parent, gboolean show_dialog)
{
    //Sanity checks
    if (error == NULL || *error == NULL) return;

    //Declarations
    GtkWidget *dialog;
    gint response;

    //Handle the error
    if (show_dialog)
    {
      dialog = gtk_message_dialog_new_with_markup (parent,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "<b>%s</b>\n\n%s",
                                                   "An Error Occured",
                                                   (*error)->message
                                                  );
      g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
      gtk_widget_show_all (GTK_WIDGET (dialog));
      response = gtk_dialog_run (GTK_DIALOG (dialog));
    }
    else
    {
      printf("ERROR: %s\n", (*error)->message);
    }

    //Cleanup
    g_error_free (*error);
    *error = NULL;
}



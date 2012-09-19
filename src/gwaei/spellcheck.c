#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static GList *_corrections = NULL;
static GMutex *_mutex = NULL;
static gboolean _needs_spellcheck = FALSE;
static char* _query_text = NULL;
static gboolean _sensitive = TRUE;

static void _menuitem_activated_cb (GtkWidget*, gpointer);
static void _populate_popup_cb (GtkEntry*, GtkMenu*, gpointer);
static gboolean _draw_underline_cb (GtkWidget*, cairo_t*, gpointer);
static void _queue_spellcheck_cb (GtkEditable*, gpointer);
static gboolean _update_spellcheck_timeout (gpointer);
static void _update_button_sensitivities (void);
static int _timer = 0;


struct _SpellingReplacementData {
  GtkEntry *entry;
  int start_offset;
  int end_offset;
  char* replacement_text;
};
typedef struct _SpellingReplacementData _SpellingReplacementData;


struct _StreamWithData {
  int stream;
  gpointer data;
};
typedef struct _StreamWithData _StreamWithData;



static void _free_menuitem_data (GtkWidget *widget, gpointer data)
{
  //Declarations
  _SpellingReplacementData *srd;

  //Initializations
  srd = data;

  //Cleanup
  g_free (srd->replacement_text);
  free (srd);
}

static gpointer _infunc (gpointer data)
{
  //Declarations
  _StreamWithData *swd;
  FILE *file;
  size_t chunk;
  int stream;
  char *text;

  //Initializations
  swd = data;
  stream = swd->stream;
  text = swd->data;
  file = fdopen(stream, "w");

  if (file != NULL)
  {
    if (ferror(file) == 0 && feof(file) == 0)
    {
      chunk = fwrite(text, sizeof(char), strlen(text), file);
    }

    fclose(file);
  }

  return NULL;
}

static gpointer _outfunc (gpointer data)
{
  //Declarations
  const int MAX = 500;
  _StreamWithData *swd;
  FILE *file;
  int stream;
  char buffer[MAX];

  //Initializations
  swd = data;
  stream = swd->stream;
  file = fdopen (swd->stream, "r");

  if (file != NULL)
  {
    g_mutex_lock (_mutex);

    //Clear out the old links
    while (_corrections != NULL)
    {
      g_free (_corrections->data);
      _corrections = g_list_delete_link (_corrections, _corrections);
    }

    //Add the new links
    while (file != NULL && ferror(file) == 0 && feof(file) == 0 && fgets(buffer, MAX, file) != NULL)
    {
      if (buffer[0] != '@' && buffer[0] != '*' && buffer[0] != '#' && strlen(buffer) > 1)
        _corrections = g_list_append (_corrections, g_strdup (buffer));
    }
    g_mutex_unlock (_mutex);

    //Cleanup
    fclose (file);
  }

  return NULL;
}

//!
//! @brief Attached the spellchecking listeners to a GtkEntry.  It can only be attached once.
//! @param entry A GtkEntry to attach to
//!
void gw_spellcheck_attach_to_entry (GtkEntry *entry)
{
  //Sanity check
  g_assert (_mutex == NULL);

  //Initializations
  _mutex = g_mutex_new ();

  g_signal_connect_after (G_OBJECT (entry), "draw", G_CALLBACK (_draw_underline_cb), NULL);
  g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (_queue_spellcheck_cb), NULL);
  g_signal_connect (G_OBJECT (entry), "populate-popup", G_CALLBACK (_populate_popup_cb), NULL);
  g_timeout_add (300, (GSourceFunc) _update_spellcheck_timeout, (gpointer) entry);
}



//
//Callbacks
//

static void _menuitem_activated_cb (GtkWidget *widget, gpointer data)
{
  //Declarations
  _SpellingReplacementData *srd;
  char *text;
  char *buffer;
  char *replacement;
  int start_offset;
  int end_offset;
  int index;

  //Initializations
  srd = data;
  replacement = srd->replacement_text;
  start_offset = srd->start_offset;
  end_offset = srd->end_offset;
  text = g_strdup (gtk_entry_get_text (GTK_ENTRY (srd->entry)));
  buffer = (char*) malloc (sizeof(char) * (strlen(replacement) + strlen(text)));

  strcpy(buffer, text);
  strcpy (buffer + start_offset, replacement);
  strcat (buffer, text + end_offset);

  index = gtk_editable_get_position (GTK_EDITABLE (srd->entry));
  if (index > end_offset || index > start_offset + strlen(replacement))
    index = index - (end_offset - start_offset) + strlen(replacement);
  gtk_entry_set_text (GTK_ENTRY (srd->entry), buffer);
  gtk_editable_set_position (GTK_EDITABLE (srd->entry), index);

  //Cleanup
  free (buffer);
  g_free (text);
}


static int _get_y_offset (GtkEntry *entry)
{
    //Declarations
    PangoRectangle rect;
    PangoLayout *layout;

    int allocation_offset;
    int layout_offset;
    int rect_offset;

    //Initializations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    rect_offset = rect.height;
    allocation_offset = gtk_widget_get_allocated_height (GTK_WIDGET (entry));
    gtk_entry_get_layout_offsets (GTK_ENTRY (entry), NULL, &layout_offset);

    return (((allocation_offset - rect_offset) / 2) - layout_offset);
}

static int _get_x_offset (GtkEntry *entry)
{
    //Declarations
    PangoRectangle rect;
    PangoLayout *layout;

    int allocation_offset;
    int layout_offset;
    int rect_offset;

    //Initializations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    rect_offset = rect.width;
    allocation_offset = gtk_widget_get_allocated_height (GTK_WIDGET (entry));
    gtk_entry_get_layout_offsets (GTK_ENTRY (entry), &layout_offset, NULL);

    return (layout_offset);
}

static int _get_string_index (GtkEntry *entry, int x, int y)
{
    //Declarations
    int layout_index;
    int entry_index;
    int trailing;
    PangoLayout *layout;

    //Initalizations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    if (pango_layout_xy_to_index (layout, x * PANGO_SCALE, y * PANGO_SCALE, &layout_index, &trailing))
      entry_index = gtk_entry_layout_index_to_text_index (GTK_ENTRY (entry), layout_index);
    else
      entry_index = -1;

    return entry_index;
}


void _populate_popup_cb (GtkEntry *entry, GtkMenu *menu, gpointer data)
{
    //Declarations
    GtkWidget *menuitem;
    char **split;
    char **info;
    char **replacements;
    const char *text;
    GList *iter;

    int index;
    int xpointer, ypointer, xoffset, yoffset, x, y;
    _SpellingReplacementData *srd;
    int start_offset, end_offset;
    int i;

    //Initializations
    text = gtk_entry_get_text (GTK_ENTRY (entry));
    gtk_widget_get_pointer (GTK_WIDGET (entry), &xpointer, &ypointer);
    xoffset = _get_x_offset (entry);
    yoffset = _get_y_offset (entry);
    x = xpointer - xoffset;
    y = yoffset; //Since a GtkEntry is single line, we want the y to always be in the area
    index =  _get_string_index (entry, x, y);

    g_mutex_lock (_mutex);
    for (iter = _corrections; index > -1 && iter != NULL; iter = iter->next)
    {
      //Create the start and end offsets 
      split = g_strsplit (iter->data, ":", 2);
      info = g_strsplit (split[0], " ", -1); 
      start_offset = (int) g_ascii_strtoull (info[3], NULL, 10);
      end_offset = strlen(info[1]) + start_offset;

      //If the mouse position is between the offsets, create the popup menuitems
      if (index >= start_offset && index <= end_offset)
      {
        replacements = g_strsplit (split[1], ",", -1);

        //Separator
        if (replacements[0] != NULL)
        {
          menuitem = gtk_separator_menu_item_new ();
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        }

        //Menuitems
        for (i = 0; replacements[i] != NULL; i++)
        {
          g_strstrip(replacements[i]);
          menuitem = gtk_menu_item_new_with_label (replacements[i]);
          srd = (_SpellingReplacementData*) malloc (sizeof(_SpellingReplacementData));
          srd->entry = entry;
          srd->start_offset = start_offset;
          srd->end_offset = end_offset;
          srd->replacement_text = g_strdup (replacements[i]);
          g_signal_connect (G_OBJECT (menuitem), "destroy", G_CALLBACK (_free_menuitem_data), (gpointer) srd);
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (_menuitem_activated_cb), (gpointer) srd);
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        }
        g_strfreev (replacements);
      }
      g_strfreev (split);
      g_strfreev (info);
    }
    g_mutex_unlock (_mutex);
}

gboolean _get_line_coordinates (GtkEntry *entry, int startindex, int endindex, int *x, int *y, int *x2, int *y2)
{
    //Declarations
    int index;
    PangoLayout *layout;
    PangoRectangle rect;
    PangoLayoutIter *iter;
    int xoffset, yoffset;

    //Initializations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    iter = pango_layout_get_iter (layout);
    xoffset = _get_x_offset (GTK_ENTRY (entry));
    yoffset = _get_y_offset (GTK_ENTRY (entry));
    *x = *y = *x2 = *y2 = 0;

    do {
      index = pango_layout_iter_get_index (iter);
      pango_layout_iter_get_char_extents  (iter, &rect);
      if (index == startindex)
      {
        *x = PANGO_PIXELS (rect.x) + xoffset;
        *y = PANGO_PIXELS (rect.y) + yoffset;
      }
      if (index == endindex - 1)
      {
        *x2 = PANGO_PIXELS (rect.width + rect.x) + xoffset;
        *y2 = PANGO_PIXELS (rect.height + rect.y) + yoffset;
      }
    } while (pango_layout_iter_next_char (iter));

    //Cleanup
    pango_layout_iter_free (iter);

    return (*x > 0 && *y > 0 && *x2 > 0 && *y2 > 0);
}

void _draw_line (cairo_t *cr, int x, int y, int x2, int y2)
{
    //Declarations
    int ydelta;
    int xdelta;
    int i;
    gboolean up;

    //Initializations
    xdelta = 2;
    ydelta = 2;
    up = FALSE;
    y += ydelta;
    x++;

    cairo_set_line_width (cr, 0.8);
    cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 0.8);

    cairo_move_to (cr, x, y2);
    for (i = x + xdelta; i < x2; i += xdelta)
    {
      if (up)
        y2 -= ydelta;
      if (!up)
        y2 += ydelta;
      up = !up;

      cairo_line_to (cr, i, y2);
    }
    cairo_stroke (cr);
}



gboolean _draw_underline_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  //Declarations
  gint x, y, x2, y2;
  PangoLayout *layout;
  GList *iter;
  char **info;
  char **atoms;
  int start_offset, end_offset;

  //Initializations
  layout = gtk_entry_get_layout (GTK_ENTRY (widget));

  g_mutex_lock (_mutex);
  for (iter = _corrections; iter != NULL; iter = iter->next)
  {
    if (iter->data == NULL) continue;

    info = g_strsplit (iter->data, ":", -1);
    atoms = g_strsplit (info[0], " ", -1);

    start_offset = (int) g_ascii_strtoull (atoms[3], NULL, 10);
    end_offset = strlen(atoms[1]) + start_offset;
    start_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), start_offset);
    end_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), end_offset);

    //Calculate the line
    if (_get_line_coordinates (GTK_ENTRY (widget), start_offset, end_offset, &x, &y, &x2, &y2))
    {
      _draw_line (cr, x, y, x2, y2);
    }

    g_strfreev (info);
    g_strfreev (atoms);
  }
  g_mutex_unlock (_mutex);

  return FALSE;
}


void _queue_spellcheck_cb (GtkEditable *editable, gpointer data)
{
    g_mutex_lock (_mutex);

    _update_button_sensitivities ();

    _timer = 0;

    if (_query_text == NULL)
      _query_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (editable)));

    //Declarations
    const char *query;

    //Initializations
    query = gtk_entry_get_text (GTK_ENTRY (editable));

    if (strcmp(_query_text, query) != 0)
    {
      //Clear out the old links
      while (_corrections != NULL)
      {
        g_free (_corrections->data);
        _corrections = g_list_delete_link (_corrections, _corrections);
      }

      g_free (_query_text);
      _query_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (editable)));

      _needs_spellcheck = TRUE;
    }
    g_mutex_unlock (_mutex);
}

static gboolean _update_spellcheck_timeout (gpointer data)
{
    if (_timer < 3)
    {
      _timer++;
      return TRUE;
    }

    //Declarations
    gboolean spellcheck_pref;
    int rk_conv_pref;
    gboolean want_conv;
    GtkEditable *editable;
    char *query;
    gboolean is_convertable_to_hiragana;
    const int MAX = 300;
    char kana[MAX];
    gboolean exists;
    GError *error;

    char *argv[] = { ENCHANT, "-a", "-d", "en", NULL};
    GPid pid;
    int stdin_stream;
    int stdout_stream;
    gboolean success;
    GThread *outthread;
    _StreamWithData indata;
    _StreamWithData outdata;
    
    //Initializations
    rk_conv_pref = lw_pref_get_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA);
    want_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
    editable = GTK_EDITABLE (data);
    query = gtk_editable_get_chars (editable, 0, -1);
    is_convertable_to_hiragana = (want_conv && lw_util_str_roma_to_hira (query, kana, MAX));
    spellcheck_pref = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK);
    exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);
    error = NULL;


    //Sanity checks
    if (
      exists == FALSE    || 
      strlen(query) == 0 || 
      !spellcheck_pref   || 
      !_sensitive        || 
      !_needs_spellcheck || 
      is_convertable_to_hiragana
    )
    {
      g_free (query);
      return TRUE;
    }

    _needs_spellcheck = FALSE;

    success = g_spawn_async_with_pipes (
      NULL, 
      argv,
      NULL,
      0,
      NULL,
      NULL,
      &pid,
      &stdin_stream,
      &stdout_stream,
      NULL,
      &error
    );

    if (success)
    {
      indata.stream = stdin_stream;
      indata.data = query;
      outdata.stream = stdout_stream;
      outdata.data = query;

      _infunc ((gpointer) &indata);
      outthread = g_thread_create (_outfunc, (gpointer) &outdata, TRUE, &error);

      g_thread_join (outthread);

      g_spawn_close_pid (pid);
    }

    //Cleanup
    if (query != NULL)
    {
      g_free (query);
    }
    if (error !=NULL) 
    {
      fprintf(stderr, "ERROR: %s\n", error->message);
      g_error_free (error);
    }

    gtk_widget_queue_draw (GTK_WIDGET (data));
  
    return TRUE;
}

//!
//! @brief Callback to toggle spellcheck in the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_spellcheck_toggle_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean state;

    //Initializations
    state = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK);

    lw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK, !state);

    if (_query_text != NULL) g_free (_query_text);
    _query_text = g_strdup ("FORCE UPDATE");
}



//!
//! @brief Sets the gui widgets consistently to the requested state
//!
//! The function makes sure that both of the widgets in the gui are the same
//! when the user clicks a one of them to change the settings.
//!
//! @param request the requested state for spellchecking widgets
//!
void gw_spellcheck_set_enabled (gboolean request)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *checkbox;
    GtkWidget *toolbutton;
    GtkWidget *entry;

    //Initializations
    builder = gw_common_get_builder ();
    checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "query_spellcheck"));
    toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, "spellcheck_toolbutton"));
    entry = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_ENTRY));

    g_signal_handlers_block_by_func (checkbox, gw_spellcheck_toggle_cb, NULL);
    g_signal_handlers_block_by_func (toolbutton, gw_spellcheck_toggle_cb, NULL);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), request);
    gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (toolbutton), request);
    _queue_spellcheck_cb (GTK_EDITABLE (entry), NULL);

    g_signal_handlers_unblock_by_func (checkbox, gw_spellcheck_toggle_cb, NULL);
    g_signal_handlers_unblock_by_func (toolbutton, gw_spellcheck_toggle_cb, NULL);
}

static void _update_button_sensitivities ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *checkbox;
    GtkWidget *toolbutton;
    GtkWidget *entry;
    gboolean exists;

    //Initializations
    builder = gw_common_get_builder ();
    entry = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_ENTRY));
    checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "query_spellcheck"));
    toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, "spellcheck_toolbutton"));
    exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);

    if (exists && !_sensitive)
    {
      _sensitive = exists;
      gtk_widget_set_sensitive (GTK_WIDGET (checkbox), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (toolbutton), TRUE);
      g_free (_query_text);
      _query_text = g_strdup ("FORCE UPDATE");
    }
    else if (!exists && _sensitive)
    {
      _sensitive = exists;
      gtk_widget_set_sensitive (GTK_WIDGET (checkbox), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (toolbutton), FALSE);
      g_free (_query_text);
      _query_text = g_strdup ("FORCE UPDATE");
    }

}

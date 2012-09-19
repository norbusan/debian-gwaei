#ifndef GW_SEARCHWINDOW_PRIVATE_INCLUDED
#define GW_SEARCHWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

typedef enum {
  GW_SEARCHWINDOW_TIMEOUTID_PROGRESS,
  GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING,
  GW_SEARCHWINDOW_TIMEOUTID_APPEND_RESULT,
  TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS
} GwSearchWindowTimeoutId;

typedef enum {
#ifdef WITH_HUNSPELL
  GW_SEARCHWINDOW_SIGNALID_SPELLCHECK,
#endif
  GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING,
  GW_SEARCHWINDOW_SIGNALID_TABBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_MENUBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT,
  GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT,
  GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION,
  GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_CHANGED,
  GW_SEARCHWINDOW_SIGNALID_RADICALSWINDOW_CLOSED,
  GW_SEARCHWINDOW_SIGNALID_KANJIPADWINDOW_CLOSED,
  TOTAL_GW_SEARCHWINDOW_SIGNALIDS
} GwSearchWindowSignalId;


struct _GwSearchWindowPrivate {
  GtkNotebook *notebook;

  GtkMenuBar *menubar;
  GtkMenu *toolbar_menu;

  GtkToolbar *primary_toolbar;
  GtkToolButton *spellcheck_toolbutton;

  GtkToolbar *search_toolbar;
  GtkEntry *entry;
  GtkComboBox *combobox;
  GtkToolButton *submit_toolbutton;
  GtkToolItem *menu_toolbutton;

  GtkWidget *statusbar;
  GtkLabel *statusbar_label;
  GtkProgressBar *statusbar_progressbar;

  LwDictionary *dictionary;

  //History
  GwHistory *history;

  //Main variables
  guint *timeoutid;
  guint *signalid;

  gint previous_tip;
  gint font_size;

  gboolean new_tab; 
  gboolean always_show_tabbar;

  GwSpellcheck *spellcheck;

  //Feedback variables
  LwSearch *feedback_item;
  long feedback;
  LwSearchStatus feedback_status;

  //Mouse variables
  LwSearch *mouse_item;
  gint mouse_button_press_x;
  gint mouse_button_press_y;
  gint mouse_button_press_root_x;
  gint mouse_button_press_root_y;
  gunichar mouse_button_character;
  gchar* mouse_hovered_word; 

  //Keep searching variables
  gint keep_searching_delay;
  gchar *keep_searching_query;
  gboolean keep_searching_enabled;

  gboolean text_selected;

  GwRadicalsWindow *radicalswindow;
  GwKanjipadWindow *kanjipadwindow;
};

#define GW_SEARCHWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_SEARCHWINDOW, GwSearchWindowPrivate))

G_END_DECLS

#endif

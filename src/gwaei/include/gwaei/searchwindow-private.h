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
  GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT,
  GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT,
  GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION,
  GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED,
  GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED,
  GW_SEARCHWINDOW_SIGNALID_VOCABULARY_CHANGED,
  TOTAL_GW_SEARCHWINDOW_SIGNALIDS
} GwSearchWindowSignalId;

struct _GwSearchWindowPrivate {
  GtkNotebook *notebook;

  GtkToolbar *primary_toolbar;
  GtkToolButton *spellcheck_toolbutton;

  GtkToolbar *search_toolbar;
  GtkEntry *entry;
  GtkComboBox *combobox;
  GtkToolButton *submit_toolbutton;
  GtkLabel *search_entry_label;

  GtkWidget *statusbar;
  GtkLabel *statusbar_label;
  GtkProgressBar *statusbar_progressbar;

  GtkMenu *dictionary_popup;
  GtkMenu *history_popup;
  GtkMenu *vocabulary_popup;
  GtkMenu *forward_popup;
  GtkMenu *back_popup;

  GtkAction *previous_tab_action;
  GtkAction *next_tab_action;
  GtkAction *close_action;
  GtkAction *cut_action;
  GtkAction *copy_action;
  GtkAction *paste_action;
  GtkAction *select_all_action;
  GtkAction *back_action;
  GtkAction *forward_action;
  GtkAction *append_action;
  GtkAction *save_as_action;
  GtkAction *print_action;
  GtkAction *print_preview_action;
  GtkAction *zoom_in_action;
  GtkAction *zoom_out_action;
  GtkAction *zoom_100_action;

  GtkToggleAction *show_toolbar_toggleaction;
  GtkToggleAction *show_statusbar_toggleaction;
  GtkToggleAction *show_radicals_toggleaction;
  GtkToggleAction *show_kanjipad_toggleaction;

  LwDictInfo *dictinfo;

  //History
  LwHistory *history;

  //Main variables
  guint timeoutid[TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS];
  guint signalid[TOTAL_GW_SEARCHWINDOW_SIGNALIDS];

  int previous_tip;
  int font_size;

  gboolean new_tab; 

  GwSpellcheck *spellcheck;

  //Feedback variables
  LwSearchItem *feedback_item;
  long feedback;
  LwSearchStatus feedback_status;

  //Mouse variables
  LwSearchItem *mouse_item;
  gint mouse_button_press_x;
  gint mouse_button_press_y;
  gint mouse_button_press_root_x;
  gint mouse_button_press_root_y;
  gunichar mouse_button_character;
  char* mouse_hovered_word; 

  //Keep searching variables
  int keep_searching_delay;
  char *keep_searching_query;
  gboolean keep_searching_enabled;

  gboolean text_selected;

  GwRadicalsWindow *radicalswindow;
  GwKanjipadWindow *kanjipadwindow;
};

#define GW_SEARCHWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_SEARCHWINDOW, GwSearchWindowPrivate))

G_END_DECLS

#endif

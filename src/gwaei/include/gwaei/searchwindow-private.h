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
  GW_SEARCHWINDOW_SIGNALID_SPELLCHECK,
  GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING,
  GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT,
  GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT,
  GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION,
  GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED,
  GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED,
  TOTAL_GW_SEARCHWINDOW_SIGNALIDS
} GwSearchWindowSignalId;

struct _GwSearchWindowPrivate {
  GtkEntry *entry;
  GtkNotebook *notebook;
  GtkToolbar *toolbar;
  GtkWidget *statusbar;
  GtkComboBox *combobox;
  LwDictInfo *dictinfo;

  //Tabs
  GList *tablist; //!< Stores the current search item set to each tab

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
};

#define GW_SEARCHWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_SEARCHWINDOW, GwSearchWindowPrivate))

G_END_DECLS

#endif

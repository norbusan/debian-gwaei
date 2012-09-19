#ifndef GW_VOCABULARYWINDOW_PRIVATE_INCLUDED
#define GW_VOCABULARYWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS


typedef enum {
  GW_VOCABULARYWINDOW_TIMEOUTID_UNUSED,
  TOTAL_GW_VOCABULARYWINDOW_TIMEOUTIDS
} GwVocabularyWindowTimeoutId;

typedef enum {
  GW_VOCABULARYWINDOW_SIGNALID_UNUSED,
  GW_VOCABULARYWINDOW_SIGNALID_CHANGED,
  GW_VOCABULARYWINDOW_SIGNALID_SHUFFLE_CHANGED,
  GW_VOCABULARYWINDOW_SIGNALID_TRIM_CHANGED,
  GW_VOCABULARYWINDOW_SIGNALID_TRACK_RESULTS_CHANGED,
  GW_VOCABULARYWINDOW_SIGNALID_LIST_ORDER_CHANGED,
  GW_VOCABULARYWINDOW_SIGNALID_TOOLBAR_TOGGLED,
  GW_VOCABULARYWINDOW_SIGNALID_POSITION_COLUMN_TOGGLED,
  GW_VOCABULARYWINDOW_SIGNALID_SCORE_COLUMN_TOGGLED,
  GW_VOCABULARYWINDOW_SIGNALID_TIMESTAMP_COLUMN_TOGGLED,
  TOTAL_GW_VOCABULARYWINDOW_SIGNALIDS
} GwVocabularyWindowSignalId;

struct _GwVocabularyWindowPrivate {
  GtkTreeView  *list_treeview;
  GtkToolbar   *list_toolbar;

  GtkTreeView  *word_treeview;
  GtkToolbar   *word_toolbar;
  GtkToolbar   *study_toolbar;
  GtkToggleToolButton *edit_toolbutton;
  GtkPaned     *paned;

  GtkMenuItem *cut_menuitem;
  GtkMenuItem *copy_menuitem;
  GtkMenuItem *paste_menuitem;
  GtkMenuItem *delete_menuitem;

  GtkAction *save_action;
  GtkAction *revert_action;
  GtkAction *kanji_definition_flashcards_action;
  GtkAction *definition_kanji_flashcards_action;
  GtkAction *kanji_furigana_flashcards_action;
  GtkAction *furigana_kanji_flashcards_action;
  GtkAction *definition_furigana_flashcards_action;
  GtkAction *furigana_definition_flashcards_action;

  GtkToggleAction *shuffle_toggleaction;
  GtkToggleAction *trim_toggleaction;
  GtkToggleAction *track_results_toggleaction;
  GtkToggleAction *show_toolbar_toggleaction;
  GtkToggleAction *show_position_column_toggleaction;
  GtkToggleAction *show_score_column_toggleaction;
  GtkToggleAction *show_timestamp_column_toggleaction;

  gboolean has_changes;
  gboolean shuffle;
  gboolean trim;
  gboolean track;

  //Main variables
  guint timeoutid[TOTAL_GW_VOCABULARYWINDOW_TIMEOUTIDS];
  guint signalid[TOTAL_GW_VOCABULARYWINDOW_SIGNALIDS];

  GtkCellRenderer *renderer[TOTAL_GW_VOCABULARYWORDSTORE_COLUMNS];

  GtkTreeViewColumn *position_column;
  GtkTreeViewColumn *score_column;
  GtkTreeViewColumn *timestamp_column;

  gint paned_initial_size;
};

#define GW_VOCABULARYWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindowPrivate))

G_END_DECLS

#endif

#ifndef GW_FLASHCARDWINDOW_PRIVATE_INCLUDED
#define GW_FLASHCARDWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

typedef enum {
  GW_FLASHCARDWINDOW_TIMEOUTID_UNUSED,
  TOTAL_GW_FLASHCARDWINDOW_TIMEOUTIDS
} GwFlashCardWindowTimeoutId;

typedef enum {
  GW_FLASHCARDWINDOW_SIGNALID_UNUSED,
  TOTAL_GW_FLASHCARDWINDOW_SIGNALIDS
} GwFlashCardWindowSignalId;

struct _GwFlashCardWindowPrivate {
    GtkLabel *card_label;
    GtkScrolledWindow *card_scrolledwindow;
    GtkToolButton *check_answer_toolbutton;
    GtkToolButton *next_card_toolbutton;
    GtkToolButton *dont_know_toolbutton;
    GtkToggleButton *track_togglebutton;
    GtkEntry *answer_entry;
    GtkLabel *correct_label;
    GtkLabel *incorrect_label;
    GtkProgressBar *status_progressbar;
    GtkLabel *status_label;
    GtkLabel *finished_label;
    GtkToolbar *card_toolbar;

    GtkBox *content_box;
    GtkBox *finished_box;

    GtkTreeModel *model;
    GtkTreeIter iter;

    gchar *flash_cards_type;
    gchar *question_title;
    gchar *list_name;
    gchar *question;
    gchar *answer;

    gint total_cards;
    gint cards_left;
    gint incorrect_guesses;
    gint correct_guesses;

    gint source_question_column;
    gint source_answer_column;

    gint64 time;

    gboolean track;
};

#define GW_FLASHCARDWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_FLASHCARDWINDOW, GwFlashCardWindowPrivate))

G_END_DECLS

#endif

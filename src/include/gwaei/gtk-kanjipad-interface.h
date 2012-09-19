#ifndef GW_GTK_KANJIPAD_INTERFACE_INCLUDED
#define GW_GTK_KANJIPAD_INTERFACE_INCLUDED

//Defines
#define MAX_GUESSES 10

//Class
struct _GwKanjipad {
  GtkWidget *drawing_widget;
  GtkWidget *candidate_widget;

  gint annotate;
  GList *strokes;

  //Private
  GdkPixmap *pixmap;
  GdkPixmap *kpixmap;
  GList *curstroke;
  int instroke;

  //Public
  char kselected[2];
  char kanji_candidates[MAX_GUESSES][2];
  int total_candidates;
  int engine_pid;
  GIOChannel *from_engine;
  GIOChannel *to_engine;
  char *data_file;
};
typedef struct _GwKanjipad GwKanjipad;

//Object
GwKanjipad *pa;

//Methods
GwKanjipad* padarea_create (GtkWidget*);
void padarea_clear (GwKanjipad*);
void padarea_set_annotate (GwKanjipad*, gint);
void padarea_changed_callback (GwKanjipad*);
void padarea_init_engine (GwKanjipad*);

void gw_kanjipad_set_target_text_widget (GtkWidget*);
GtkWidget* gw_kanjipad_get_target_text_widget (void);
void gw_kanjipad_initialize (GtkBuilder*);
void gw_kanjipad_free_resources ();


#endif

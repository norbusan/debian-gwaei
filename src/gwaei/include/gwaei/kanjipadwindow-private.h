#ifndef GW_KANJIPADWINDOW_PRIVATE_INCLUDED
#define GW_KANJIPADWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwKanjipadWindowPrivate {
  GtkDrawingArea *drawingarea;
  GtkDrawingArea *candidates;
  GtkButton *close_button;

  gint annotate;
  GList *strokes;
  cairo_surface_t *surface;
  cairo_surface_t *ksurface;
  GList *curstroke;
  gboolean instroke;
  char kselected[2];
  char kanji_candidates[GW_KANJIPADWINDOW_MAX_GUESSES][2];
  int total_candidates;
  GPid engine_pid;
  GIOChannel *from_engine;
  GIOChannel *to_engine;
  guint iowatchid;
};

#define GW_KANJIPADWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_KANJIPADWINDOW, GwKanjipadWindowPrivate))

G_END_DECLS

#endif

#ifndef GW_KANJIPAD_INTERFACE_CANDIDATEAREA_INCLUDED
#define GW_KANJIPAD_INTERFACE_CANDIDATEAREA_INCLUDED

gboolean candidatearea_configure_event (GtkWidget*, GdkEventConfigure*, GwKanjipad*);
gboolean candidatearea_draw_cb (GtkWidget*, cairo_t*, GwKanjipad*);
gboolean candidatearea_button_press_event (GtkWidget*, GdkEventButton*, GwKanjipad*);
void _candidatearea_draw (GtkWidget*, GwKanjipad*);
void gw_kanjipad_candidatearea_initialize (GwKanjipad*);

#endif


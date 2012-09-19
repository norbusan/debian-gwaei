#ifndef GW_KANJIPAD_CANDIDATEAREA_INCLUDED
#define GW_KANJIPAD_CANDIDATEAREA_INCLUDED

void gw_kanjipadwindow_initialize_candidates (GwKanjipadWindow*);
void gw_kanjipadwindow_draw_candidates (GwKanjipadWindow*);

gboolean gw_kanjipadwindow_candidatearea_configure_event_cb (GtkWidget*, GdkEventConfigure*, gpointer);
gboolean gw_kanjipadwindow_candidatearea_draw_cb (GtkWidget*, cairo_t*, gpointer);
gboolean gw_kanjipadwindow_candidatearea_button_press_event_cb (GtkWidget*, GdkEventButton*, gpointer);

#endif


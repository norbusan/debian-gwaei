#ifndef GW_KANJIPADWINDOW_INCLUDED
#define GW_KANJIPADWINDOW_INCLUDED

G_BEGIN_DECLS

#include <gwaei/window.h>

typedef enum {
  GW_KANJIPADWINDOW_CLASS_SIGNALID_KANJI_SELECTED,
  TOTAL_GW_KANJIPADWINDOW_CLASS_SIGNALIDS
} GwKanjipadWindowClassSignalId;

//Boilerplate
typedef struct _GwKanjipadWindow GwKanjipadWindow;
typedef struct _GwKanjipadWindowClass GwKanjipadWindowClass;
typedef struct _GwKanjipadWindowPrivate GwKanjipadWindowPrivate;

#define GW_TYPE_KANJIPADWINDOW              (gw_kanjipadwindow_get_type())
#define GW_KANJIPADWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_KANJIPADWINDOW, GwKanjipadWindow))
#define GW_KANJIPADWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_KANJIPADWINDOW, GwKanjipadWindowClass))
#define GW_IS_KANJIPADWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_KANJIPADWINDOW))
#define GW_IS_KANJIPADWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_KANJIPADWINDOW))
#define GW_KANJIPADWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_KANJIPADWINDOW, GwKanjipadWindowClass))

#define GW_KANJIPADWINDOW_MAX_GUESSES 10

struct _GwKanjipadWindow {
  GwWindow window;
  GwKanjipadWindowPrivate *priv;
};

struct _GwKanjipadWindowClass {
  GwWindowClass parent_class;
  guint signalid[TOTAL_GW_KANJIPADWINDOW_CLASS_SIGNALIDS];
  void (*kanji_selected) (GwKanjipadWindow *window);
};

GtkWindow* gw_kanjipadwindow_new (GtkApplication *application);
GType gw_kanjipadwindow_get_type (void) G_GNUC_CONST;

#include "kanjipadwindow-callbacks.h"
#include "kanjipad-candidatearea.h"
#include "kanjipad-drawingarea.h"

G_END_DECLS

#endif

#ifndef GW_RADICALSWINDOW_INCLUDED
#define GW_RADICALSWINDOW_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwRadicalsWindow GwRadicalsWindow;
typedef struct _GwRadicalsWindowClass GwRadicalsWindowClass;
typedef struct _GwRadicalsWindowPrivate GwRadicalsWindowPrivate;

#define GW_TYPE_RADICALSWINDOW              (gw_radicalswindow_get_type())
#define GW_RADICALSWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_RADICALSWINDOW, GwRadicalsWindow))
#define GW_RADICALSWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_RADICALSWINDOW, GwRadicalsWindowClass))
#define GW_IS_RADICALSWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_RADICALSWINDOW))
#define GW_IS_RADICALSWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_RADICALSWINDOW))
#define GW_RADICALSWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_RADICALSWINDOW, GwRadicalsWindowClass))


struct _GwRadicalsWindow {
  GwWindow window;
  GwRadicalsWindowPrivate *priv;
};

struct _GwRadicalsWindowClass {
  GwWindowClass parent_class;
};


GtkWindow* gw_radicalswindow_new (GtkApplication*);
GType gw_radicalswindow_get_type (void) G_GNUC_CONST;

char* gw_radicalswindow_strdup_all_selected (GwRadicalsWindow*);
char* gw_radicalswindow_strdup_prefered_stroke_count (GwRadicalsWindow*);
void gw_radicalswindow_deselect_all_radicals (GwRadicalsWindow*);
void gw_radicalswindow_set_strokes_checkbox_state (GwRadicalsWindow*, gboolean);
void gw_radicalswindow_set_button_sensitive_when_label_is (GwRadicalsWindow*, const char*);
void gw_radicalswindow_update_strokes_checkbox_state (GwRadicalsWindow*);

#include "radicalswindow-callbacks.h"

G_END_DECLS

#endif

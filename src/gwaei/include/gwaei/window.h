#ifndef GW_WINDOW_INCLUDED
#define GW_WINDOW_INCLUDED

G_BEGIN_DECLS

#include <gwaei/application.h>

//Boilerplate
typedef struct _GwWindow GwWindow;
typedef struct _GwWindowClass GwWindowClass;
typedef struct _GwWindowPrivate GwWindowPrivate;

#define GW_TYPE_WINDOW              (gw_window_get_type())
#define GW_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_WINDOW, GwWindow))
#define GW_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_WINDOW, GwWindowClass))
#define GW_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_WINDOW))
#define GW_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_WINDOW))
#define GW_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_WINDOW, GwWindowClass))

struct _GwWindow {
  GtkWindow window;
  GwWindowPrivate *priv;
};

struct _GwWindowClass {
  GtkWindowClass parent_class;
};

//Methods
GtkWindow* gw_window_new (void);
GType gw_window_get_type (void) G_GNUC_CONST;

void gw_window_unload_xml (GwWindow*);

GObject* gw_window_get_object (GwWindow*, const gchar*);
void gw_window_set_application (GwWindow*, GwApplication*);
GwApplication* gw_window_get_application (GwWindow*);
GtkWidget* gw_window_get_toplevel (GwWindow*);
GtkAccelGroup *gw_window_get_accel_group (GwWindow*);

void gw_window_set_is_important (GwWindow*, gboolean);
gboolean gw_window_is_important (GwWindow*);
void gw_window_load_size (GwWindow*);
void gw_window_save_size (GwWindow*);

#include "window-callbacks.h"

G_END_DECLS

#endif

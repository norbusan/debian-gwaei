#ifndef GW_INSTALLPROGRESSWINDOW_INCLUDED
#define GW_INSTALLPROGRESSWINDOW_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwInstallProgressWindow GwInstallProgressWindow;
typedef struct _GwInstallProgressWindowClass GwInstallProgressWindowClass;
typedef struct _GwInstallProgressWindowPrivate GwInstallProgressWindowPrivate;

#define GW_TYPE_INSTALLPROGRESSWINDOW              (gw_installprogresswindow_get_type())
#define GW_INSTALLPROGRESSWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_INSTALLPROGRESSWINDOW, GwInstallProgressWindow))
#define GW_INSTALLPROGRESSWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_INSTALLPROGRESSWINDOW, GwInstallProgressWindowClass))
#define GW_IS_INSTALLPROGRESSWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_INSTALLPROGRESSWINDOW))
#define GW_IS_INSTALLPROGRESSWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_INSTALLPROGRESSWINDOW))
#define GW_INSTALLPROGRESSWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_INSTALLPROGRESSWINDOW, GwInstallProgressWindowClass))

#define GW_INSTALLPROGRESSWINDOW_KEEP_SEARCHING_MAX_DELAY 3

struct _GwInstallProgressWindow {
  GwWindow window;
  GwInstallProgressWindowPrivate *priv;
};

struct _GwInstallProgressWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_installprogresswindow_new (GtkApplication *application);
GType gw_installprogresswindow_get_type (void) G_GNUC_CONST;

void gw_installprogresswindow_start (GwInstallProgressWindow*);

#include "installprogresswindow-callbacks.h"

G_END_DECLS

#endif

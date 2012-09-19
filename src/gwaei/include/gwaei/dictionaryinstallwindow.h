#ifndef GW_DICTIONARYINSTALLWINDOW_INCLUDED
#define GW_DICTIONARYINSTALLWINDOW_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwDictionaryInstallWindow GwDictionaryInstallWindow;
typedef struct _GwDictionaryInstallWindowClass GwDictionaryInstallWindowClass;
typedef struct _GwDictionaryInstallWindowPrivate GwDictionaryInstallWindowPrivate;

#define GW_TYPE_DICTIONARYINSTALLWINDOW              (gw_dictionaryinstallwindow_get_type())
#define GW_DICTIONARYINSTALLWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_DICTIONARYINSTALLWINDOW, GwDictionaryInstallWindow))
#define GW_DICTIONARYINSTALLWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_DICTIONARYINSTALLWINDOW, GwDictionaryInstallWindowClass))
#define GW_IS_DICTIONARYINSTALLWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_DICTIONARYINSTALLWINDOW))
#define GW_IS_DICTIONARYINSTALLWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_DICTIONARYINSTALLWINDOW))
#define GW_DICTIONARYINSTALLWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_DICTIONARYINSTALLWINDOW, GwDictionaryInstallWindowClass))

struct _GwDictionaryInstallWindow {
  GwWindow window;
  GwDictionaryInstallWindowPrivate *priv;
};

struct _GwDictionaryInstallWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_dictionaryinstallwindow_new (GtkApplication *application);
GType gw_dictionaryinstallwindow_get_type (void) G_GNUC_CONST;

void gw_dictionaryinstallwindow_sync_interface (GwDictionaryInstallWindow*);
void gw_dictionaryinstallwindow_clear_details_box (GwDictionaryInstallWindow*);
void gw_dictionaryinstallwindow_fill_details_box (GwDictionaryInstallWindow*, LwDictionary*);

#include "dictionaryinstallwindow-callbacks.h"

G_END_DECLS

#endif

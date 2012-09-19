#ifndef GW_INSTALLPROGRESSWINDOW_PRIVATE_INCLUDED
#define GW_INSTALLPROGRESSWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwInstallProgressWindowPrivate {
  LwDictInst *di;

  GtkLabel *label;
  GtkLabel *sublabel;
  GtkProgressBar* progressbar;
  GtkButton *cancel_button;

  double install_fraction;
  GMutex mutex;
};

#define GW_INSTALLPROGRESSWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_INSTALLPROGRESSWINDOW, GwInstallProgressWindowPrivate))

G_END_DECLS

#endif

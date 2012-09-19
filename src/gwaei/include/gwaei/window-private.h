#ifndef GW_WINDOW_PRIVATE_INCLUDED
#define GW_WINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwWindowPrivate {
  GtkBuilder *builder;
  GwApplication *application;
  GMenuModel *menu_model;
  GtkMenuBar *menubar;
  gchar* ui_xml;
  gboolean important;
  GtkWidget *toplevel;
  GtkAccelGroup *accelgroup;
  gint x, y, width, height;
};

#define GW_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_WINDOW, GwWindowPrivate))

G_END_DECLS

#endif


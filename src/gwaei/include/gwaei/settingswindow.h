#ifndef GW_SETTINGSWINDOW_INCLUDED
#define GW_SETTINGSWINDOW_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwSettingsWindow GwSettingsWindow;
typedef struct _GwSettingsWindowClass GwSettingsWindowClass;
typedef struct _GwSettingsWindowPrivate GwSettingsWindowPrivate;

#define GW_TYPE_SETTINGSWINDOW              (gw_settingswindow_get_type())
#define GW_SETTINGSWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_SETTINGSWINDOW, GwSettingsWindow))
#define GW_SETTINGSWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_SETTINGSWINDOW, GwSettingsWindowClass))
#define GW_IS_SETTINGSWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_SETTINGSWINDOW))
#define GW_IS_SETTINGSWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_SETTINGSWINDOW))
#define GW_SETTINGSWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_SETTINGSWINDOW, GwSettingsWindowClass))

#define GW_SETTINGSWINDOW_KEEP_SEARCHING_MAX_DELAY 3

struct _GwSettingsWindow {
  GwWindow window;
  GwSettingsWindowPrivate *priv;
};

struct _GwSettingsWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_settingswindow_new (GtkApplication *application);
GType gw_settingswindow_get_type (void) G_GNUC_CONST;

void gw_settingswindow_update_interface (GwSettingsWindow*);

void gw_settingswindow_initialize_dictionary_order_list (GwSettingsWindow*);
void gw_settings_set_dictionary_source (GtkWidget*, const char*);
void gw_settingswindow_update_global_font_label (GwSettingsWindow*, const char*);
void gw_settingswindow_update_custom_font_button (GwSettingsWindow*, const char*);
void gw_settingswindow_update_dictionary_order_list (GwSettingsWindow*);
void gw_settingswindow_increment_order_list_processes (GwSettingsWindow*);
void gw_settingswindow_decrement_order_list_processes (GwSettingsWindow*);
void gw_settingswindow_set_use_global_document_font_checkbox (GwSettingsWindow*, gboolean);
void gw_settingswindow_set_search_as_you_type (GwSettingsWindow*, gboolean);
gboolean gw_settingswindow_get_search_as_you_type (GwSettingsWindow*);
void gw_settingswindow_check_for_dictionaries (GwSettingsWindow*);

#include "settingswindow-callbacks.h"

G_END_DECLS

#endif

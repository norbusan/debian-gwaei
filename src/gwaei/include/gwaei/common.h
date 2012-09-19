#ifndef GW_COMMON_INCLUDED
#define GW_COMMON_INCLUDED


#define GW_MAX_FONT_MAGNIFICATION  6
#define GW_MIN_FONT_MAGNIFICATION -6
#define GW_DEFAULT_FONT_MAGNIFICATION 0
#define GW_FONT_ZOOM_STEP 2
#define GW_MAX_FONT_SIZE 100
#define GW_MIN_FONT_SIZE 6
#define GW_DEFAULT_FONT_SIZE 12
#define GW_DEFAULT_FONT "Sans 12"


void gw_common_initialize (void);
void gw_common_free (void);

void gw_common_show_window (char*);
void gw_common_hide_window (const char*);

GtkBuilder* gw_common_get_builder (void);
gboolean gw_common_load_ui_xml (const char*);
GtkWidget* gw_common_get_widget_by_target (const LwTargetOutput);
gpointer gw_common_get_gobject_by_target (const LwTargetOutput);
gboolean gw_common_widget_equals_target (GtkWidget*, const LwTargetOutput);

void gw_common_handle_error (GError**, GtkWindow*, gboolean);

#endif

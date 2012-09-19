#ifndef GW_GTK_RADICALS_INTERFACE_INCLUDED
#define GW_GTK_RADICALS_INTERFACE_INCLUDED


typedef enum {
  GW_RADARRAY_STROKES,
  GW_RADARRAY_REPRESENTATIVE,
  GW_RADARRAY_ACTUAL,
  GW_RADARRAY_NAME,
  GW_RADARRAY_TOTAL
} GwRadicalArrayField;

void gw_radsearchtool_initialize (void);
void gw_radsearchtool_free (void);

char* gw_radsearchtool_strdup_all_selected_radicals (void);
char* gw_radsearchtool_strdup_prefered_stroke_count (void);
void gw_radsearchtool_deselect_all_radicals (void);
void gw_radsearchtool_set_strokes_checkbox_state (gboolean);
void gw_radsearchtool_set_button_sensitive_when_label_is (const char*);
void gw_radsearchtool_update_strokes_checkbox_state (void);


#endif

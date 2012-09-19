#ifndef GW_RADICALSWINDOW_PRIVATE_INCLUDED
#define GW_RADICALSWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwRadicalsWindowPrivate {
  GtkToggleButton *strokes_checkbutton;
  GtkToolPalette *toolpalette;
  GtkSpinButton *strokes_spinbutton;

  char cache[300 * 4];
};

typedef enum {
  GW_RADARRAY_STROKES,
  GW_RADARRAY_REPRESENTATIVE,
  GW_RADARRAY_ACTUAL,
  GW_RADARRAY_NAME,
  GW_RADARRAY_TOTAL
} GwRadicalArrayField;

#define GW_RADICALSWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_RADICALSWINDOW, GwRadicalsWindowPrivate))

G_END_DECLS

#endif

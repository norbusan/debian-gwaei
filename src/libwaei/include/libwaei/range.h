#ifndef LW_RANGE_INCLUDED
#define LW_RANGE_INCLUDED

G_BEGIN_DECLS

struct _LwRange {
  gchar *identifier;
  gint lower;
  gint higher;
};

typedef struct _LwRange LwRange;

LwRange* lw_range_new_from_pattern (const gchar*);
void lw_range_free (LwRange*);

gboolean lw_range_pattern_is_valid (const gchar*);
gboolean lw_range_string_is_in_range (LwRange*, const gchar*);
gboolean lw_range_int_is_in_range (LwRange*, gint);

G_END_DECLS

#endif

#ifndef LW_REGEX_INCLUDED
#define LW_REGEX_INCLUDED

#include <glib.h>
#include <libwaei/utilities.h>

G_BEGIN_DECLS

#define LW_RE_COMPILE_FLAGS (G_REGEX_CASELESS | G_REGEX_OPTIMIZE)
#define LW_RE_LOCATE_FLAGS  (0)
#define LW_RE_EXIST_FLAGS   (0)

void lw_regex_initialize (void);
void lw_regex_free (void);

typedef enum {
  LW_RE_NUMBER,
  LW_RE_STROKES,
  LW_RE_GRADE,
  LW_RE_FREQUENCY,
  LW_RE_JLPT,
  LW_RE_TOTAL
} LwRegexDataIndex;

extern GRegex *lw_re[LW_RE_TOTAL + 1];

G_END_DECLS

#endif

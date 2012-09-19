#ifndef LW_REGEX_INCLUDED
#define LW_REGEX_INCLUDED

#define LW_RE_COMPILE_FLAGS (G_REGEX_CASELESS | G_REGEX_OPTIMIZE)
#define LW_RE_LOCATE_FLAGS  (0)
#define LW_RE_EXIST_FLAGS   (0)

#include <glib.h>
#include <libwaei/dict.h>
#include <libwaei/utilities.h>

typedef enum
{
  LW_RELEVANCE_HIGH,
  LW_RELEVANCE_MEDIUM,
  LW_RELEVANCE_LOW,
  LW_RELEVANCE_LOCATE,
  LW_RELEVANCE_TOTAL
} LwRelevance;


void lw_regex_initialize (void);
void lw_regex_free (void);

GRegex* lw_regex_kanji_new (const char*, LwDictType, LwRelevance, GError**);
GRegex* lw_regex_furi_new (const char*, LwDictType, LwRelevance, GError**);
GRegex* lw_regex_romaji_new (const char*, LwDictType, LwRelevance, GError**);
GRegex* lw_regex_mix_new (const char*, LwDictType, LwRelevance, GError**);
GRegex* lw_regex_new (const char*, LwDictType, LwRelevance, GError**);


typedef enum {
  LW_RE_NUMBER,
  LW_RE_STROKES,
  LW_RE_GRADE,
  LW_RE_FREQUENCY,
  LW_RE_JLPT,
/*
  LW_RE_WORD_I_ADJ_PASTFORM,
  LW_RE_WORD_I_ADJ_NEGATIVE,
  LW_RE_WORD_I_ADJ_TE_FORM,
  LW_RE_WORD_I_ADJ_CAUSATIVE,
  LW_RE_WORD_I_ADJ_CONDITIONAL,
  LW_RE_WORD_NA_ADJ_PASTFORM,
  LW_RE_WORD_NA_ADJ_NEGATIVE,
  LW_RE_WORD_NA_ADJ_TE_FORM,
  LW_RE_WORD_NA_ADJ_CAUSATIVE,
  LW_RE_WORD_NA_ADJ_CONDITIONAL,
*/
  LW_RE_TOTAL
} LwRegexDataIndex;

extern GRegex *lw_re[LW_RE_TOTAL + 1];

#endif

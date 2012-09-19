#ifndef GW_REGEX_INCLUDED
#define GW_REGEX_INCLUDED

#define GW_REGEX_EFLAGS_EXIST    (REG_EXTENDED | REG_ICASE | REG_NOSUB)
#define GW_REGEX_EFLAGS_LOCATE   (REG_EXTENDED | REG_ICASE)


void gw_regex_initialize_constant_regular_expressions(void);
void gw_regex_free_constant_regular_expressions (void);

char* gw_regex_locate_offset (char*, char*, regex_t*, gint*, gint*);
gboolean gw_regex_create_kanji_high_regex (regex_t*, char*, int);
gboolean gw_regex_create_kanji_med_regex (regex_t*, char*, int);
gboolean gw_regex_create_furi_high_regex (regex_t*, char*, int);
gboolean gw_regex_create_furi_med_regex (regex_t*, char*, int);
gboolean gw_regex_create_roma_high_regex (regex_t*, char*, int);
gboolean gw_regex_create_roma_med_regex (regex_t*, char*, int);
gboolean gw_regex_create_mix_high_regex (regex_t*, char*, int);
gboolean gw_regex_create_mix_med_regex (regex_t*, char*, int);
gboolean gw_regex_create_exact_regex (regex_t*, char*, int);


gboolean gw_regex_locate_boundary_byte_pointers (const char*, char*, char**, char **);


typedef enum {
  GW_RE_DICT_ENGLISH,
  GW_RE_DICT_RADICAL,
  GW_RE_DICT_KANJI,
  GW_RE_DICT_PLACES,
  GW_RE_DICT_NAMES,
  GW_RE_DICT_MIX,

  GW_RE_QUERY_STROKES,
  GW_RE_QUERY_GRADE,
  GW_RE_QUERY_FREQUENCY,
  GW_RE_QUERY_JLPT,

  GW_RE_FILENAME_GZ,
  GW_RE_COLOR_HEXCOLOR,

  GW_RE_WORD_I_ADJ_PASTFORM,
  GW_RE_WORD_I_ADJ_NEGATIVE,
  GW_RE_WORD_I_ADJ_TE_FORM,
  GW_RE_WORD_I_ADJ_CAUSATIVE,
  GW_RE_WORD_I_ADJ_CONDITIONAL,
  GW_RE_WORD_NA_ADJ_PASTFORM,
  GW_RE_WORD_NA_ADJ_NEGATIVE,
  GW_RE_WORD_NA_ADJ_TE_FORM,
  GW_RE_WORD_NA_ADJ_CAUSATIVE,
  GW_RE_WORD_NA_ADJ_CONDITIONAL,

  GW_RE_TOTAL
} GwInitialDictonaryRegexIndex;

regex_t *gw_re[GW_RE_TOTAL];



#endif

#ifndef LW_QUERYLINE_INCLUDED
#define LW_QUERYLINE_INCLUDED

#include <libwaei/range.h>

G_BEGIN_DECLS

#define LW_QUERY(object) (LwQuery*) object
#define LW_QUERY_DELIMITOR_PRIMARY_CHARACTER '&'
#define LW_QUERY_DELIMITOR_PRIMARY_STRING "&"
#define LW_QUERY_DELIMITOR_SUPPLIMENTARY_CHARACTER '|'
#define LW_QUERY_DELIMITOR_SUPPLIMENTARY_STRING "|"

typedef enum {
  LW_QUERY_FLAG_DELIMIT_WHITESPACE = (1 << 0),
  LW_QUERY_FLAG_DELIMIT_MORPHOLOGY =  (1 << 1),
  LW_QUERY_FLAG_ROMAJI_TO_FURIGANA = (1 << 2),
  LW_QUERY_FLAG_HIRAGANA_TO_KATAKANA = (1 << 3),
  LW_QUERY_FLAG_KATAKANA_TO_HIRAGANA = (1 << 4),
  LW_QUERY_FLAG_ROOT_WORD = (1 << 5)
} LwQueryFlags;

typedef enum {
  LW_QUERY_TYPE_MIX,
  LW_QUERY_TYPE_KANJI,
  LW_QUERY_TYPE_FURIGANA,
  LW_QUERY_TYPE_ROMAJI,
  TOTAL_LW_QUERY_TYPES
} LwQueryType;

typedef enum {
  LW_QUERY_RANGE_TYPE_STROKES,
  LW_QUERY_RANGE_TYPE_GRADE,
  LW_QUERY_RANGE_TYPE_JLPT,
  LW_QUERY_RANGE_TYPE_FREQUENCY,
  TOTAL_LW_QUERY_RANGE_TYPES
} LwQueryRangeType;

struct _LwQuery {
    gchar *text;
    gchar ***tokenlist;
    GList ***regexgroup;
    LwRange **rangelist;
    gboolean parsed;
    LwQueryFlags flags;
#ifdef WITH_MECAB
    gchar *morphology;
#endif
};
typedef struct _LwQuery LwQuery;


LwQuery* lw_query_new ();
void lw_query_free (LwQuery*);

void lw_query_clean (LwQuery*);
const gchar* lw_query_get_text (LwQuery*);
gboolean lw_query_is_parsed (LwQuery*);

void lw_query_init_regexgroup (LwQuery*);
void lw_query_init_tokens (LwQuery*);
void lw_query_init_rangelist (LwQuery*);

void lw_query_clear (LwQuery*);

void lw_query_tokenlist_append_primary (LwQuery*, LwQueryType, const gchar*);
void lw_query_tokenlist_append_supplimentary (LwQuery*, LwQueryType, gint, const gchar*);

gchar* lw_query_get_supplimentary (LwQuery*, LwRelevance, LwQueryType, const gchar*, LwQueryType*);
gchar** lw_query_tokenlist_get (LwQuery*, LwQueryType);

void lw_query_rangelist_set (LwQuery*, LwQueryRangeType, LwRange*);
LwRange* lw_query_rangelist_get (LwQuery*, LwQueryRangeType);

GList* lw_query_regexgroup_get (LwQuery*, LwQueryType, LwRelevance);
void lw_query_regexgroup_append (LwQuery*, LwQueryType, LwRelevance, GRegex*);

G_END_DECLS

#endif

#ifndef LW_WORD_INCLUDED
#define LW_WORD_INCLUDED

G_BEGIN_DECLS

typedef enum {
  LW_WORD_FIELD_KANJI,
  LW_WORD_FIELD_FURIGANA,
  LW_WORD_FIELD_DEFINITIONS,
  LW_WORD_FIELD_CORRECT_GUESSES,
  LW_WORD_FIELD_INCORRECT_GUESSES,
  LW_WORD_FIELD_TIMESTAMP,
  TOTAL_LW_WORD_FIELDS
} LwWordField;


struct _LwWord {
  gchar *fields[TOTAL_LW_WORD_FIELDS];
  gchar *score;
  gchar *days;
  gint correct_guesses;
  gint incorrect_guesses;
  gint32 timestamp;
};

typedef struct _LwWord LwWord;

#define LW_WORD(obj) (LwWord*)obj

LwWord* lw_word_new (void);
LwWord* lw_word_new_from_string (const gchar*);
void lw_word_free (LwWord*);

void lw_word_set_kanji (LwWord*, const gchar*);
const gchar* lw_word_get_kanji (LwWord*);

void lw_word_set_furigana (LwWord*, const gchar*);
const gchar* lw_word_get_furigana (LwWord*);

void lw_word_set_definitions (LwWord*, const gchar*);
const gchar* lw_word_get_definitions (LwWord*);

gint lw_word_get_correct_guesses (LwWord*);
void lw_word_set_correct_guesses (LwWord*, gint);

gint lw_word_get_incorrect_guesses (LwWord*);
void lw_word_set_incorrect_guesses (LwWord*, gint);

gint lw_word_get_score (LwWord*);
const gchar* lw_word_get_score_as_string (LwWord*);

guint32 lw_word_timestamp_to_hours (gint64);
void lw_word_set_timestamp (LwWord*, gint64);
void lw_word_update_timestamp (LwWord*);
void lw_word_set_hours (LwWord*, guint32);
guint32 lw_word_get_hours (LwWord*);
const gchar* lw_word_get_timestamp_as_string (LwWord*);

gchar* lw_word_to_string (LwWord*);
void lw_word_update_timestamp (LwWord*);
gint64 lw_word_get_timestamp (LwWord*);
const gchar* lw_word_get_timestamp_as_string (LwWord*);

G_END_DECLS

#endif

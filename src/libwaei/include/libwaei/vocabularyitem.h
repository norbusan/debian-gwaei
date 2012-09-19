#ifndef LW_VOCABULARYITEM_INCLUDED
#define LW_VOCABULARYITEM_INCLUDED

G_BEGIN_DECLS

typedef enum {
  LW_VOCABULARYITEM_FIELD_KANJI,
  LW_VOCABULARYITEM_FIELD_FURIGANA,
  LW_VOCABULARYITEM_FIELD_DEFINITIONS,
  LW_VOCABULARYITEM_FIELD_CORRECT_GUESSES,
  LW_VOCABULARYITEM_FIELD_INCORRECT_GUESSES,
  LW_VOCABULARYITEM_FIELD_TIMESTAMP,
  TOTAL_LW_VOCABULARYITEM_FIELDS
} LwVocabularyItemField;


struct _LwVocabularyItem {
  gchar *fields[TOTAL_LW_VOCABULARYITEM_FIELDS];
  gchar *score;
  gchar *days;
  gint correct_guesses;
  gint incorrect_guesses;
  gint32 timestamp;
};

typedef struct _LwVocabularyItem LwVocabularyItem;

#define LW_VOCABULARYITEM(obj) (LwVocabularyItem*)obj

LwVocabularyItem* lw_vocabularyitem_new ();
LwVocabularyItem* lw_vocabularyitem_new_from_string (const gchar*);
void lw_vocabularyitem_free (LwVocabularyItem*);

void lw_vocabularyitem_set_kanji (LwVocabularyItem*, const gchar*);
const gchar* lw_vocabularyitem_get_kanji (LwVocabularyItem*);

void lw_vocabularyitem_set_furigana (LwVocabularyItem*, const gchar*);
const gchar* lw_vocabularyitem_get_furigana (LwVocabularyItem*);

void lw_vocabularyitem_set_definitions (LwVocabularyItem*, const gchar*);
const gchar* lw_vocabularyitem_get_definitions (LwVocabularyItem*);

gint lw_vocabularyitem_get_correct_guesses (LwVocabularyItem*);
void lw_vocabularyitem_set_correct_guesses (LwVocabularyItem*, gint);

gint lw_vocabularyitem_get_incorrect_guesses (LwVocabularyItem*);
void lw_vocabularyitem_set_incorrect_guesses (LwVocabularyItem*, gint);

gint lw_vocabularyitem_get_score (LwVocabularyItem*);
const gchar* lw_vocabularyitem_get_score_as_string (LwVocabularyItem*);

guint32 lw_vocabularyitem_timestamp_to_hours (gint64);
void lw_vocabularyitem_set_timestamp (LwVocabularyItem*, gint64);
void lw_vocabularyitem_update_timestamp (LwVocabularyItem*);
void lw_vocabularyitem_set_hours (LwVocabularyItem*, guint32);
guint32 lw_vocabularyitem_get_hours (LwVocabularyItem*);
const gchar* lw_vocabularyitem_get_timestamp_as_string (LwVocabularyItem*);

gchar* lw_vocabularyitem_to_string (LwVocabularyItem*);
void lw_vocabularyitem_update_timestamp (LwVocabularyItem*);
gint64 lw_vocabularyitem_get_timestamp (LwVocabularyItem *item);
const gchar* lw_vocabularyitem_get_timestamp_as_string (LwVocabularyItem *item);

G_END_DECLS

#endif

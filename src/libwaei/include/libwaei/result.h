#ifndef LW_RESULT_INCLUDED
#define LW_RESULT_INCLUDED

#include <libwaei/io.h>

G_BEGIN_DECLS

#define LW_RESULT(object) (LwResult*) object

//!
//! @brief Primitive for storing lists of dictionaries
//!

struct _LwResult {
    gchar text[LW_IO_MAX_FGETS_LINE];     //!< Character array holding the result line for the pointers to reference

    //General result things
    LwRelevance relevance;
    gchar *def_start[50];        //!< Pointer to the definitions
    gint def_total;              //!< Total definitions found for a result
    gchar *number[50];           //!< Pointer to the numbers of the definitions
    gchar *kanji_start;          //!< The kanji portion of the definition
    gchar *furigana_start;       //!< The furigana portion of a definition
    gchar *classification_start; //!< The classification of the word type of the Japanese word.

    //Kanji things
    gchar *strokes;      //!< Pointer to the number of strokes of a kanji
    gchar *frequency;    //!< Pointer to the frequency number of a kanji
    gchar *readings[3];  //!< Pointer to the readings of a kanji
    gchar *meanings;     //!< Pointer to the meanings of a kanji
    gchar *grade;        //!< Pointer to the grade of a kanji
    gchar *jlpt;         //!< Pointer to the JLPT level of a kanji
    gchar *kanji;        //!< Pointer to the kanji itself
    gchar *radicals;     //!< Pointer to a kanji's radicals

    gboolean important; //!< Weather a word/phrase has a high frequency of usage.

};
typedef struct _LwResult LwResult;


LwResult* lw_result_new (void);
void lw_result_free (LwResult*);
void lw_result_init (LwResult*);
void lw_result_deinit (LwResult*);

gboolean lw_result_is_similar (LwResult*, LwResult*);
void lw_result_clear (LwResult*);

G_END_DECLS

#endif

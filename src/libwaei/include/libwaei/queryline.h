#ifndef LW_QUERYLINE_INCLUDED
#define LW_QUERYLINE_INCLUDED

G_BEGIN_DECLS

#define LW_QUERYLINE(object) (LwQueryLine*) object
#define LW_QUERYLINE_MAX_ATOMS 20

struct _LwQueryLine {
    //Storage for the original query string
    char *string;

    //Result from morphological parsing (NULL if none or not relevant)
    char *morphology;

    //General search regexes
    GRegex*** re_kanji;
    GRegex*** re_furi;
    GRegex*** re_roma;
    GRegex*** re_mix;

    //Specific regexes for the kanji dictionary
    GRegex*** re_strokes;
    GRegex*** re_frequency;
    GRegex*** re_grade;
    GRegex*** re_jlpt;
};
typedef struct _LwQueryLine LwQueryLine;


LwQueryLine* lw_queryline_new (void );
void lw_queryline_free (LwQueryLine*);
void lw_queryline_init (LwQueryLine*);
void lw_queryline_deinit (LwQueryLine*);

int lw_queryline_parse_edict_string (LwQueryLine*l, LwPreferences*, const char*, GError**);
int lw_queryline_parse_kanjidict_string (LwQueryLine*, LwPreferences*, const char*, GError**);
int lw_queryline_parse_exampledict_string (LwQueryLine*, LwPreferences*, const char*, GError**);
int lw_queryline_parse_edict_string (LwQueryLine*, LwPreferences*, const char*, GError**);

G_END_DECLS

#endif

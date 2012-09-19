#ifndef GW_QUERYLINE_OBJECT_INCLUDED
#define GW_QUERYLINE_OBJECT_INCLUDED

#define GW_QUERYLINE_MAX_ATOMS 20

struct _LwQueryLine {
    //Storage for the original query string
    char *string;

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

int lw_queryline_parse_edict_string (LwQueryLine*l, const char*, GError**);
int lw_queryline_parse_kanjidict_string (LwQueryLine*, const char*, GError**);
int lw_queryline_parse_exampledict_string (LwQueryLine*, const char*, GError**);
int lw_queryline_parse_edict_string (LwQueryLine*, const char*, GError**);

#endif

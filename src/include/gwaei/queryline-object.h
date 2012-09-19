#ifndef GW_QUERYLINE_OBJECT_INCLUDED
#define GW_QUERYLINE_OBJECT_INCLUDED

#define MAX_ATOMS 20
#define MAX_ATOM_LENGTH 100

#define GW_QUERYLINE_EXIST 0
#define GW_QUERYLINE_LOCATE 1
#define GW_QUERYLINE_LOW 2
#define GW_QUERYLINE_MED 3
#define GW_QUERYLINE_HIGH 4

struct _GwQueryLine {
    char string[MAX_QUERY];
    char hira_string[MAX_QUERY];

    regex_t kanji_regex[5][MAX_ATOMS];
    int kanji_total;
    regex_t furi_regex[5][MAX_ATOMS];
    int furi_total;
    regex_t roma_regex[5][MAX_ATOMS];
    int roma_total;
    regex_t mix_regex[5][MAX_ATOMS];
    int mix_total;

    //Kanji things
    regex_t strokes_regex[5][MAX_ATOMS];
    int strokes_total;
    regex_t frequency_regex[5][MAX_ATOMS];
    int frequency_total;
    regex_t grade_regex[5][MAX_ATOMS];
    int grade_total;
    regex_t jlpt_regex[5][MAX_ATOMS];
    int jlpt_total;
};
typedef struct _GwQueryLine GwQueryLine;


GwQueryLine* gw_queryline_new (void );
void gw_queryline_free (GwQueryLine*);

int gw_queryline_parse_edict_string (GwQueryLine*l, const char*);
int gw_queryline_parse_kanjidict_string (GwQueryLine*, const char*);
int gw_queryline_parse_exampledict_string (GwQueryLine*, const char*);
int gw_queryline_parse_edict_string (GwQueryLine*, const char*);

#endif

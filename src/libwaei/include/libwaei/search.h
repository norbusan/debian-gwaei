#ifndef LW_SEARCH_INCLUDED
#define LW_SEARCH_INCLUDED

#include <stdio.h>

#include <libwaei/query.h>
#include <libwaei/result.h>
#include <libwaei/dictionary.h>

G_BEGIN_DECLS

#define LW_SEARCH(object) (LwSearch*) object
#define LW_SEARCH_DATA_FREE_FUNC(object) (LwSearchDataFreeFunc)object

//!
//! @brief Search status types
//!
typedef enum
{
  LW_SEARCHSTATUS_IDLE,
  LW_SEARCHSTATUS_SEARCHING,
  LW_SEARCHSTATUS_FINISHING,
  LW_SEARCHSTATUS_CANCELING
} LwSearchStatus;


typedef enum
{
  //First 16 bits are made to mirror the LwQueryFlags
  LW_SEARCH_FLAG_DELIMIT_WHITESPACE = (1 << 0),
  LW_SEARCH_FLAG_DELIMIT_MORPHOLGY =  (1 << 1),
  LW_SEARCH_FLAG_ROMAJI_TO_FURIGANA = (1 << 2),
  LW_SEARCH_FLAG_HIRAGANA_TO_KATAKANA = (1 << 3),
  LW_SEARCH_FLAG_KATAKANA_TO_HIRAGANA = (1 << 4),
  LW_SEARCH_FLAG_ROOT_WORD = (1 << 5),
  //Last 16 bits are specific to LwSearchFlags
  LW_SEARCH_FLAG_EXACT = (1 << 6)
} LwSearchFlags;

typedef void(*LwSearchDataFreeFunc)(gpointer);

//!
//! @brief Primitive for storing search item information
//!
//Object
struct _LwSearch {
    LwQuery* query;                 //!< Result line to store parsed result
    LwDictionary* dictionary;                 //!< Pointer to the dictionary used

    FILE* fd;                               //!< File descriptor for file search position
    GThread *thread;                        //!< Thread the search is processed in
    GMutex mutex;                          //!< Mutext to help ensure threadsafe operation

    LwSearchStatus status;                  //!< Used to test if a search is in progress.
    LwSearchFlags flags;
    gchar *scratch_buffer;                   //!< Scratch space
    glong current;                           //!< Current line in the dictionary file

    gint max;

    gint total_results[TOTAL_LW_RELEVANCE];

    gboolean cancel;

    GList *results[TOTAL_LW_RELEVANCE];

    LwResult* result;               //!< Result line to store parsed result

    gpointer data;                 //!< Pointer to a buffer that stays constant unlike when the target attribute is used

    gint64 timestamp;

    LwSearchDataFreeFunc free_data_func;
};
typedef struct _LwSearch LwSearch;

//Methods
LwSearch* lw_search_new (LwDictionary*, const gchar*, LwSearchFlags, GError**);
void lw_search_free (LwSearch*);

void lw_search_cleanup_search (LwSearch*);
void lw_search_clear_results (LwSearch*);
void lw_search_prepare_search (LwSearch*);

gboolean lw_search_compare (LwSearch *, const LwRelevance);
gboolean lw_search_is_equal (LwSearch*, LwSearch*);

void lw_search_set_data (LwSearch*, gpointer, LwSearchDataFreeFunc);
gpointer lw_search_get_data (LwSearch*);
void lw_search_free_data (LwSearch*);
gboolean lw_search_has_data (LwSearch*);

gboolean  lw_search_has_results (LwSearch*);
LwResult* lw_search_get_result (LwSearch*);
void lw_search_parse_result_string (LwSearch*);
void lw_search_cancel (LwSearch*);

void lw_search_lock (LwSearch*);
void lw_search_unlock (LwSearch*);

void lw_search_set_status (LwSearch*, LwSearchStatus);
LwSearchStatus lw_search_get_status (LwSearch*);

double lw_search_get_progress (LwSearch*);
gboolean lw_search_read_line (LwSearch*);

void lw_search_start (LwSearch*, gboolean);

gint lw_search_get_total_results (LwSearch*);
gint lw_search_get_total_relevant_results (LwSearch*);
gint lw_search_get_total_irrelevant_results (LwSearch*);

void lw_search_set_flags (LwSearch*, LwSearchFlags);
LwSearchFlags lw_search_get_flags (LwSearch*);
LwSearchFlags lw_search_get_flags_from_preferences (LwPreferences*);

G_END_DECLS

#endif

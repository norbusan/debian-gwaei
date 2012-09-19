#ifndef LW_SEARCHITEM_INCLUDED
#define LW_SEARCHITEM_INCLUDED

#include <stdio.h>

#include <libwaei/queryline.h>
#include <libwaei/resultline.h>
#include <libwaei/dictinfo.h>

G_BEGIN_DECLS

#define LW_SEARCHITEM(object) (LwSearchItem*) object
#define LW_SEARCHITEM_DATA_FREE_FUNC(object) (LwSearchItemDataFreeFunc)object
#define LW_HISTORY_TIME_TO_RELEVANCE 20

//!
//! @brief Search status types
//!
typedef enum
{
  LW_SEARCHSTATUS_IDLE,
  LW_SEARCHSTATUS_SEARCHING,
  LW_SEARCHSTATUS_FINISHING
} LwSearchStatus;

typedef void(*LwSearchItemDataFreeFunc)(gpointer);

//!
//! @brief Primitive for storing search item information
//!
//Object
struct _LwSearchItem {
    LwQueryLine* queryline;                 //!< Result line to store parsed result
    LwDictInfo* dictionary;                 //!< Pointer to the dictionary used

    FILE* fd;                               //!< File descriptor for file search position
    GThread *thread;                        //!< Thread the search is processed in
    GMutex mutex;                          //!< Mutext to help ensure threadsafe operation

    LwSearchStatus status;                  //!< Used to test if a search is in progress.
    char *scratch_buffer;                   //!< Scratch space
    long current;                           //!< Current line in the dictionary file
    int history_relevance_idle_timer;       //!< Helps determine if something is added to the history or not

    int total_relevant_results;             //!< Total results guessed to be highly relevant to the query
    int total_irrelevant_results;           //!< Total results guessed to be vaguely relevant to the query
    int total_results;                      //!< Total results returned from the search

    GList *results_high;                    //!< Buffer storing mediumly relevant result for later display
    GList *results_medium;                  //!< Buffer storing mediumly relevant result for later display
    GList *results_low;                     //!< Buffer storing lowly relevant result for later display

    LwResultLine* resultline;               //!< Result line to store parsed result

    gpointer data;                 //!< Pointer to a buffer that stays constant unlike when the target attribute is used
    LwSearchItemDataFreeFunc free_data_func;
};
typedef struct _LwSearchItem LwSearchItem;

//Methods
LwSearchItem* lw_searchitem_new (const char*, LwDictInfo*, LwPreferences*, GError**);
void lw_searchitem_free (LwSearchItem*);
void lw_searchitem_init (LwSearchItem*, const char*, LwDictInfo*, LwPreferences*, GError**);
void lw_searchitem_deinit (LwSearchItem*);

void lw_searchitem_cleanup_search (LwSearchItem*);
void lw_searchitem_clear_results (LwSearchItem*);
void lw_searchitem_prepare_search (LwSearchItem*);

gboolean lw_searchitem_run_comparison (LwSearchItem*, const LwRelevance);
gboolean lw_searchitem_is_equal (LwSearchItem*, LwSearchItem*);
gboolean lw_searchitem_has_history_relevance (LwSearchItem*, gboolean);
void lw_searchitem_increment_history_relevance_timer (LwSearchItem*);

void lw_searchitem_set_data (LwSearchItem*, gpointer, LwSearchItemDataFreeFunc);
gpointer lw_searchitem_get_data (LwSearchItem*);
void lw_searchitem_free_data (LwSearchItem*);
gboolean lw_searchitem_has_data (LwSearchItem*);

gboolean lw_searchitem_should_check_results (LwSearchItem*);
LwResultLine* lw_searchitem_get_result (LwSearchItem*);
void lw_searchitem_parse_result_string (LwSearchItem*);
void lw_searchitem_cancel_search (LwSearchItem*);

void lw_searchitem_lock (LwSearchItem*);
void lw_searchitem_unlock (LwSearchItem*);

void lw_searchitem_set_status (LwSearchItem*, LwSearchStatus);
LwSearchStatus lw_searchitem_get_status (LwSearchItem*);

double lw_searchitem_get_progress (LwSearchItem*);

G_END_DECLS

#endif

#ifndef LW_DICTINFOLIST_INCLUDED
#define LW_DICTINFOLIST_INCLUDED

#include <libwaei/dictinfo.h>

G_BEGIN_DECLS

#define LW_DICTINFOLIST(object) (LwDictInfoList*) object

#define EXTENDS_LW_DICTINFOLIST \
  GList *list; \
  GMutex mutex; \
  int max;

//!
//! @brief Primitive for storing lists of dictionaries
//!
struct _LwDictInfoList
{
  EXTENDS_LW_DICTINFOLIST
};
typedef struct _LwDictInfoList LwDictInfoList;


LwDictInfoList* lw_dictinfolist_new (const int);
void lw_dictinfolist_free (LwDictInfoList*);
void lw_dictinfolist_init (LwDictInfoList*, const int);
void lw_dictinfolist_deinit (LwDictInfoList*);

void lw_dictinfolist_add_dictionary (LwDictInfoList*, const LwDictType, const char*);

LwDictInfo* lw_dictinfolist_get_dictinfo (LwDictInfoList*, const LwDictType, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_by_filename (LwDictInfoList*, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_by_idstring (LwDictInfoList*, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_fuzzy (LwDictInfoList*, const char*);
LwDictInfo* lw_dictinfolist_get_dictinfo_by_load_position (LwDictInfoList*, int);
gboolean lw_dictinfolist_check_if_loaded (LwDictInfoList*, const LwDictType, const char*);
void lw_dictinfolist_update_load_orders (LwDictInfoList*);
int lw_dictinfolist_get_total (LwDictInfoList*);
void lw_dictinfolist_clear (LwDictInfoList*);

void lw_dictinfolist_preform_postprocessing_by_name (LwDictInfoList*, char*, GError**);
void lw_dictinfolist_load_order (LwDictInfoList*, LwPreferences*);
void lw_dictinfolist_save_order (LwDictInfoList*, LwPreferences*);

void lw_dictinfolist_reload (LwDictInfoList*);
void lw_dictinfolist_sort_and_normalize_order (LwDictInfoList*);

G_END_DECLS


#endif

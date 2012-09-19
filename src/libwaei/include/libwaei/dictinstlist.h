#ifndef LW_DICTINSTLIST_INCLUDED
#define LW_DICTINSTLIST_INCLUDED

#define LW_DICTINSTLIST(object) (LwDictInstList*) object

struct _LwDictInstList {
  GList *list;
  gboolean cancel;
};
typedef struct _LwDictInstList LwDictInstList;


LwDictInstList* lw_dictinstlist_new (LwPreferences *pm);
void lw_dictinstlist_free (LwDictInstList*);

gboolean lw_dictinstlist_data_is_valid (LwDictInstList*);
LwDictInst* lw_dictinstlist_get_dictinst_fuzzy (LwDictInstList*, const char*);
LwDictInst* lw_dictinstlist_get_dictinst_by_idstring (LwDictInstList*, const char*);
LwDictInst* lw_dictinstlist_get_dictinst_by_filename (LwDictInstList*, const char*);
void lw_dictinstlist_set_cancel_operations (LwDictInstList*, gboolean);

#endif

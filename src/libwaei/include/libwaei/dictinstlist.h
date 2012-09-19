#ifndef GW_DICTINSTLIST_HEADER_INCLUDED
#define GW_DICTINSTLIST_HEADER_INCLUDED

void lw_dictinstlist_initialize (void);
void lw_dictinstlist_free (void);

GList* lw_dictinstlist_get_list (void);
gboolean lw_dictinstlist_data_is_valid (void);
LwDictInst* lw_dictinstlist_get_dictinst_fuzzy (const char*);
LwDictInst* lw_dictinstlist_get_dictinst_by_idstring (const char*);
LwDictInst* lw_dictinstlist_get_dictinst_by_filename (const char*);
void lw_dictinstlist_set_cancel_operations (gboolean);

#endif

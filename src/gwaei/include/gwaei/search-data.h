#ifndef GW_SEARCHDATA_INCLUDED
#define GW_SEARCHDATA_INCLUDED

#define GW_SEARCHDATA(object) (GwSearchData*)object

struct _GwSearchData {
  GtkTextView *view;
  GwSearchWindow *window;
  LwResult *result;
};
typedef struct _GwSearchData GwSearchData;

GwSearchData* gw_searchdata_new (GtkTextView*, GwSearchWindow*);
void gw_searchdata_free (GwSearchData*);

void gw_searchdata_set_result (GwSearchData*, LwResult*);
LwResult* gw_searchdata_get_result (GwSearchData*);

#endif

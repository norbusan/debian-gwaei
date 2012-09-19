#ifndef W_SEARCHDATA_INCLUDED
#define W_SEARCHDATA_INCLUDED

#define W_SEARCHDATA(object) (WSearchData*)object

struct _WSearchData {
  GMainLoop *loop;
  WApplication *application;
  gboolean less_relevant_header_set;
};
typedef struct _WSearchData WSearchData;

WSearchData* w_searchdata_new (GMainLoop*, WApplication*);
void w_searchdata_free (WSearchData*);

#endif



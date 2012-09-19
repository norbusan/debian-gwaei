#ifndef GW_ADDVOCABULARYWINDOW_INCLUDED
#define GW_ADDVOCABULARYWINDOW_INCLUDED

#include <libwaei/searchitem.h>

G_BEGIN_DECLS

typedef enum {
  GW_ADDVOCABULARYWINDOW_CLASS_SIGNALID_WORD_ADDED,
  TOTAL_GW_ADDVOCABULARYWINDOW_CLASS_SIGNALIDS
} GwAddVocabularyWindowClassSignalId;

typedef enum {
  GW_ADDVOCABULARYWINDOW_FOCUS_LIST,
  GW_ADDVOCABULARYWINDOW_FOCUS_KANJI,
  GW_ADDVOCABULARYWINDOW_FOCUS_FURIGANA,
  GW_ADDVOCABULARYWINDOW_FOCUS_DEFINITIONS,
  GW_ADDVOCABULARYWINDOW_FOCUS_ADD_BUTTON,
  TOTAL_GW_ADDVOCABULARYWINDOW_FOCUSES
} GwAddVocabularyWindowFocus;


//Boilerplate
typedef struct _GwAddVocabularyWindow GwAddVocabularyWindow;
typedef struct _GwAddVocabularyWindowClass GwAddVocabularyWindowClass;
typedef struct _GwAddVocabularyWindowPrivate GwAddVocabularyWindowPrivate;

#define GW_TYPE_ADDVOCABULARYWINDOW              (gw_addvocabularywindow_get_type())
#define GW_ADDVOCABULARYWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_ADDVOCABULARYWINDOW, GwAddVocabularyWindow))
#define GW_ADDVOCABULARYWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_ADDVOCABULARYWINDOW, GwAddVocabularyWindowClass))
#define GW_IS_ADDVOCABULARYWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_ADDVOCABULARYWINDOW))
#define GW_IS_ADDVOCABULARYWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_ADDVOCABULARYWINDOW))
#define GW_ADDVOCABULARYWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_ADDVOCABULARYWINDOW, GwAddVocabularyWindowClass))

struct _GwAddVocabularyWindow {
  GwWindow window;
  GwAddVocabularyWindowPrivate *priv;
};

struct _GwAddVocabularyWindowClass {
  GwWindowClass parent_class;
  guint signalid[TOTAL_GW_ADDVOCABULARYWINDOW_CLASS_SIGNALIDS];
  void (*word_added) (GwAddVocabularyWindow *window);
  gchar *last_selected_list_name;
};

GtkWindow* gw_addvocabularywindow_new (GtkApplication *application);
GType gw_addvocabularywindow_get_type (void) G_GNUC_CONST;

void gw_addvocabularywindow_set_kanji (GwAddVocabularyWindow*, const gchar*);
const gchar* gw_addvocabularywindow_get_kanji (GwAddVocabularyWindow*);

void gw_addvocabularywindow_set_furigana (GwAddVocabularyWindow*, const gchar*);
const gchar* gw_addvocabularywindow_get_furigana (GwAddVocabularyWindow *);

void gw_addvocabularywindow_set_definitions (GwAddVocabularyWindow*, const gchar*);
const gchar* gw_addvocabularywindow_get_definitions (GwAddVocabularyWindow*);

void gw_addvocabularywindow_set_list (GwAddVocabularyWindow*, const gchar*);
const gchar* gw_addvocabularywindow_get_list (GwAddVocabularyWindow*);
GtkListStore* gw_addvocabularywindow_get_wordstore (GwAddVocabularyWindow*);

gboolean gw_addvocabularywindow_validate (GwAddVocabularyWindow*);
void gw_addvocabularywindow_focus_add_button (GwAddVocabularyWindow*);

gboolean gw_addvocabularywindow_get_iter (GwAddVocabularyWindow*, GtkTreeIter*);
void gw_addvocabularywindow_set_focus (GwAddVocabularyWindow*, GwAddVocabularyWindowFocus);

#include "addvocabularywindow-callbacks.h"

G_END_DECLS

#endif


#ifndef GW_VOCABULARYWINDOW_INCLUDED
#define GW_VOCABULARYWINDOW_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwVocabularyWindow GwVocabularyWindow;
typedef struct _GwVocabularyWindowClass GwVocabularyWindowClass;
typedef struct _GwVocabularyWindowPrivate GwVocabularyWindowPrivate;

#define GW_TYPE_VOCABULARYWINDOW              (gw_vocabularywindow_get_type())
#define GW_VOCABULARYWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindow))
#define GW_VOCABULARYWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindowClass))
#define GW_IS_VOCABULARYWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_VOCABULARYWINDOW))
#define GW_IS_VOCABULARYWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_VOCABULARYWINDOW))
#define GW_VOCABULARYWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindowClass))

struct _GwVocabularyWindow {
  GwWindow window;
  GwVocabularyWindowPrivate *priv;
};

struct _GwVocabularyWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_vocabularywindow_new (GtkApplication *application);
GType gw_vocabularywindow_get_type (void) G_GNUC_CONST;

void gw_vocabularywindow_save (GwVocabularyWindow*);
void gw_vocabularywindow_load_selected_vocabulary (GwVocabularyWindow*);
void gw_vocabularywindow_load_vocabulary_by_index (GwVocabularyWindow*, gint);
void gw_vocabularywindow_new_list (GwVocabularyWindow*);
void gw_vocabularywindow_remove_selected_lists (GwVocabularyWindow*);

void gw_vocabularywindow_remove_selected_words (GwVocabularyWindow*);

void gw_vocabularywindow_set_selected_vocabulary (GwVocabularyWindow*);
void gw_vocabularywindow_clean_files (GwVocabularyWindow*);
void gw_vocabularywindow_clean_lists (GwVocabularyWindow*);
gboolean gw_vocabularywindow_list_exists (GwVocabularyWindow*, const gchar*);
void gw_vocabularywindow_reset (GwVocabularyWindow*);
void gw_vocabularywindow_set_has_changes (GwVocabularyWindow*, gboolean);
gboolean gw_vocabularywindow_has_changes (GwVocabularyWindow*);
gchar* gw_vocabularywindow_selected_words_to_string (GwVocabularyWindow*);
gboolean gw_vocabularywindow_current_wordstore_has_changes (GwVocabularyWindow*);
void gw_vocabularywindow_start_flashcards (GwVocabularyWindow*, const gchar*, const gchar*, gint, gint);
GtkListStore* gw_vocabularywindow_get_selected_wordstore (GwVocabularyWindow*);
void gw_searchwindow_update_vocabulary_menuitems (GwSearchWindow*);
void gw_vocabularywindow_set_selected_list (GwVocabularyWindow*, GtkTreePath*);
gboolean gw_vocabularywindow_show_save_dialog (GwVocabularyWindow*);
void gw_vocabularywindow_show_vocabulary_list (GwVocabularyWindow*, gboolean);
void gw_vocabularywindow_update_flashcard_menu_sensitivities (GwVocabularyWindow *window);

#include "vocabularywindow-callbacks.h"

G_END_DECLS

#endif

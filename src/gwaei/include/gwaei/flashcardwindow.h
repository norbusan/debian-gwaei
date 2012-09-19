#ifndef GW_FLASHCARDWINDOW_INCLUDED
#define GW_FLASHCARDWINDOW_INCLUDED

#include "flashcardstore.h"

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwFlashCardWindow GwFlashCardWindow;
typedef struct _GwFlashCardWindowClass GwFlashCardWindowClass;
typedef struct _GwFlashCardWindowPrivate GwFlashCardWindowPrivate;

#define GW_TYPE_FLASHCARDWINDOW              (gw_flashcardwindow_get_type())
#define GW_FLASHCARDWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_FLASHCARDWINDOW, GwFlashCardWindow))
#define GW_FLASHCARDWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_FLASHCARDWINDOW, GwFlashCardWindowClass))
#define GW_IS_FLASHCARDWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_FLASHCARDWINDOW))
#define GW_IS_FLASHCARDWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_FLASHCARDWINDOW))
#define GW_FLASHCARDWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_FLASHCARDWINDOW, GwFlashCardWindowClass))

#define GW_FLASHCARDWINDOW_KEEP_SEARCHING_MAX_DELAY 3

struct _GwFlashCardWindow {
  GwWindow window;
  GwFlashCardWindowPrivate *priv;
};

struct _GwFlashCardWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_flashcardwindow_new (GtkApplication *application);
GType gw_flashcardwindow_get_type (void) G_GNUC_CONST;

void gw_flashcardwindow_set_model (GwFlashCardWindow*, GwFlashCardStore*, const gchar*, const gchar*, const gchar*);
void gw_flashcardwindow_load_iterator (GwFlashCardWindow*, gboolean, gboolean);
void gw_flashcardwindow_increment_incorrect_guesses (GwFlashCardWindow*);
void gw_flashcardwindow_set_incorrect_guesses (GwFlashCardWindow*, gint);
gint gw_flashcardwindow_get_incorrect_guesses (GwFlashCardWindow*);
void gw_flashcardwindow_increment_correct_guesses (GwFlashCardWindow*);
void gw_flashcardwindow_set_correct_guesses (GwFlashCardWindow*, gint);
gint gw_flashcardwindow_get_correct_guesses (GwFlashCardWindow*);
void gw_flashcardwindow_update_progress (GwFlashCardWindow*);
void gw_flashcardwindow_set_card_completed (GwFlashCardWindow*, gboolean);
gboolean gw_flashcardwindow_iterate (GwFlashCardWindow*);
void gw_flashcardwindow_check_answer (GwFlashCardWindow*);
void gw_flashcardwindow_set_finished (GwFlashCardWindow*);
void gw_flashcardwindow_set_track_results (GwFlashCardWindow*, gboolean);

#include "flashcardwindow-callbacks.h"

G_END_DECLS

#endif

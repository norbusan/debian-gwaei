#ifndef GW_SPELLCHECK_INCLUDED
#define GW_SPELLCHECK_INCLUDED

G_BEGIN_DECLS


//Boilerplate
typedef struct _GwSpellcheck GwSpellcheck;
typedef struct _GwSpellcheckClass GwSpellcheckClass;
typedef struct _GwSpellcheckPrivate GwSpellcheckPrivate;

#define GW_TYPE_SPELLCHECK              (gw_spellcheck_get_type())
#define GW_SPELLCHECK(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_SPELLCHECK, GwSpellcheck))
#define GW_SPELLCHECK_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass),  GW_TYPE_SPELLCHECK, GwSpellcheckClass))
#define GW_IS_SPELLCHECK(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_SPELLCHECK))
#define GW_IS_SPELLCHECK_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_SPELLCHECK))
#define GW_SPELLCHECK_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj),  GW_TYPE_SPELLCHECK, GwSpellcheckClass))

struct _GwSpellcheck {
  GObject object;
  GwSpellcheckPrivate *priv;
};

struct _GwSpellcheckClass {
  GObjectClass parent_class;
};

//Methods
GwSpellcheck* gw_spellcheck_new (GwApplication*);
GwSpellcheck* gw_spellcheck_new_with_entry (GwApplication*, GtkEntry*);
GType gw_spellcheck_get_type (void) G_GNUC_CONST;
void gw_spellcheck_set_entry (GwSpellcheck*, GtkEntry*);
gboolean gw_spellcheck_update (GwSpellcheck*);
void gw_spellcheck_queue (GwSpellcheck*);


gint gw_spellcheck_get_layout_x_offset (GwSpellcheck *spellcheck);
gint gw_spellcheck_get_layout_y_offset (GwSpellcheck *spellcheck);

void gw_spellcheck_set_timeout_threshold (GwSpellcheck*, guint);
gboolean gw_spellcheck_clear (GwSpellcheck*);

void gw_spellcheck_record_mouse_cordinates (GwSpellcheck*, GdkEvent*);
void gw_spellcheck_populate_popup (GwSpellcheck*, GtkMenu*);

void gw_spellcheck_load_dictionary (GwSpellcheck*);

#include <gwaei/spellcheck-callbacks.h>

G_END_DECLS

#endif

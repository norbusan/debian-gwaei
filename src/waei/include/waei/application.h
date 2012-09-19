#ifndef W_APPLICATION_INCLUDED
#define W_APPLICATION_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _WApplication WApplication;
typedef struct _WApplicationClass WApplicationClass;
typedef struct _WApplicationPrivate WApplicationPrivate;

#define W_TYPE_APPLICATION              (w_application_get_type())
#define W_APPLICATION(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), W_TYPE_APPLICATION, WApplication))
#define W_APPLICATION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), W_TYPE_APPLICATION, WApplicationClass))
#define W_IS_APPLICATION(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), W_TYPE_APPLICATION))
#define W_IS_APPLICATION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), W_TYPE_APPLICATION))
#define W_APPLICATION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), W_TYPE_APPLICATION, WApplicationClass))

struct _WApplication {
  GObject application;
  WApplicationPrivate *priv;
};

struct _WApplicationClass {
  GObjectClass parent_class;
};

//Methods
GObject* w_application_new ();
GType w_application_get_type (void) G_GNUC_CONST;

gint w_application_run (WApplication *application, int *argc, char **argv[]);

const char* w_application_get_program_name (WApplication*);

void w_application_handle_error (WApplication*, GError**);

LwPreferences* w_application_get_preferences (WApplication*);
LwDictionaryList* w_application_get_installed_dictionarylist (WApplication*);
LwDictionaryList* w_application_get_installable_dictionarylist (WApplication*);

gboolean w_application_get_quiet_switch (WApplication*);
gboolean w_application_get_exact_switch (WApplication*);
gboolean w_application_get_list_switch (WApplication*);
gboolean w_application_get_version_switch (WApplication*);
gboolean w_application_get_color_switch (WApplication*);
const gchar* w_application_get_dictionary_switch_data (WApplication*);
const gchar* w_application_get_install_switch_data (WApplication*);
const gchar* w_application_get_uninstall_switch_data (WApplication*);
const gchar* w_application_get_query_text_data (WApplication*);

G_END_DECLS

#endif

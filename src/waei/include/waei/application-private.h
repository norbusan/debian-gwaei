#ifndef W_APPLICATION_PRIVATE_INCLUDED
#define W_APPLICATION_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _WApplicationPrivate {
  int* argc;
  char*** argv;

  LwPreferences *preferences;
  LwDictInfoList *dictinfolist;
  LwDictInstList *dictinstlist;

  gboolean arg_quiet_switch;
  gboolean arg_exact_switch;
  gboolean arg_list_switch;
  gboolean arg_version_switch;
  gboolean arg_color_switch;

  char* arg_dictionary_switch_data;
  char* arg_install_switch_data;
  char* arg_uninstall_switch_data;
  char* arg_query_text_data;

  GOptionContext *context;
};

#define W_APPLICATION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), W_TYPE_APPLICATION, WApplicationPrivate))

G_END_DECLS

#endif

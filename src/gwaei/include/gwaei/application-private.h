#ifndef GW_APPLICATION_PRIVATE_INCLUDED
#define GW_APPLICATION_PRIVATE_INCLUDED

#include <gwaei/vocabularyliststore.h>

G_BEGIN_DECLS

typedef enum {
  GW_APPLICATION_SIGNALID_MATCH_FG,
  GW_APPLICATION_SIGNALID_MATCH_BG,
  GW_APPLICATION_SIGNALID_HEADER_FG,
  GW_APPLICATION_SIGNALID_HEADER_BG,
  GW_APPLICATION_SIGNALID_COMMENT_FG,
  TOTAL_GW_APPLICATION_SIGNALIDS
} GwApplicationSignalId;

struct _GwApplicationPrivate {
  gint* argc;
  gchar*** argv;

  GError *error;

  LwPreferences *preferences;
  GwDictionaryList *installed_dictionarylist;
  GwDictionaryList *installable_dictionarylist;
  GwSearchWindow *last_focused;

  guint signalid[TOTAL_GW_APPLICATION_SIGNALIDS];

  GOptionContext *context;
  gchar   *arg_dictionary;
  gchar   *arg_query;
  gboolean arg_version_switch;
  gboolean arg_new_vocabulary_window_switch;

  GtkListStore *vocabularyliststore;

  gint block_new_searches;
};

#define GW_APPLICATION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_APPLICATION, GwApplicationPrivate))

G_END_DECLS

#endif

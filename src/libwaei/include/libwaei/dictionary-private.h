#ifndef LW_DICTIONARY_PRIVATE_INCLUDED
#define LW_DICTIONARY_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _LwDictionaryPrivate {
    gchar *filename;
    gchar *name;
    gdouble progress;
    size_t length;       //!< Length of the file
    GMutex mutex;
    LwDictionaryInstall *install;
    gboolean selected;
};

struct _LwDictionaryInstall {
  gchar *name;
  gchar *description;

  LwDictionaryInstallerStatus status;
  gint index;

  gchar *files;
  gchar *downloads;

  gchar **filelist;
  gchar **downloadlist;
  gchar **decompresslist;
  gchar **encodelist;
  gchar **postprocesslist;
  gchar **installlist;
  gchar **installedlist;

  LwPreferences *preferences;
  const gchar *key;
  gboolean builtin;
  gulong listenerid;            //!< An id to hold the g_signal_connect value when the source copy uri pref is set
  LwEncoding encoding;          //!< Path to the raw unziped dictionary file
  gboolean postprocess;
};

#define LW_DICTIONARY_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), LW_TYPE_DICTIONARY, LwDictionaryPrivate));

G_END_DECLS

#endif

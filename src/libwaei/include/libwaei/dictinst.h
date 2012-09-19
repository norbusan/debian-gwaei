#ifndef LW_DICTINST_INCLUDED
#define LW_DICTINST_INCLUDED

#include <libwaei/dict.h>

G_BEGIN_DECLS

#define LW_DICTINST(object) (LwDictInst*) object

#define LW_DICTINST_ERROR "gWaei Dictionary Installer Error"

typedef enum {
  LW_DICTINST_ERROR_SOURCE_PATH,
  LW_DICTINST_ERROR_TARGET_PATH,
  LW_DICTINST_ERROR_FILE_MOVE
} LwDictInstError;

typedef enum {
  LW_DICTINST_NEEDS_DOWNLOADING,
  LW_DICTINST_NEEDS_DECOMPRESSION,
  LW_DICTINST_NEEDS_TEXT_ENCODING,
  LW_DICTINST_NEEDS_POSTPROCESSING,
  LW_DICTINST_NEEDS_FINALIZATION,
  LW_DICTINST_NEEDS_NOTHING,
  LW_DICTINST_TOTAL_URIS
} LwDictInstUri;


struct _LwDictInst {
  EXTENDS_LW_DICT

  char *description;
  char *uri[LW_DICTINST_TOTAL_URIS];
  double progress;
  gboolean selected;
  LwPreferences *pm;
  char *schema;
  char *key;
  gboolean builtin;
  gulong listenerid;            //!< An id to hold the g_signal_connect value when the source copy uri pref is set
  LwCompression compression;    //!< Path to the gziped dictionary file
  LwEncoding encoding;          //!< Path to the raw unziped dictionary file
  LwDictInstUri uri_group_index;
  int uri_atom_index;
  char **current_source_uris;
  char **current_target_uris;
  gboolean split;
  gboolean merge;
  GMutex mutex;
};
typedef struct _LwDictInst LwDictInst;

LwDictInst* lw_dictinst_new_using_pref_uri (const char*, 
                                            const char*,
                                            const char*,
                                            const char*,
                                            LwPreferences*,
                                            const char*,
                                            const char*,
                                            const LwDictType,
                                            const LwCompression,
                                            const LwEncoding,
                                            gboolean, gboolean, gboolean);

LwDictInst* lw_dictinst_new (const char*,
                             const char*,
                             const char*,
                             const char*,
                             const char*,
                             const LwDictType,
                             const LwCompression,
                             const LwEncoding,
                             gboolean, gboolean, gboolean);
void lw_dictinst_free (LwDictInst*);
void lw_dictinst_init (LwDictInst*,
                       const char*,
                       const char*,
                       const char*,
                       const char*,
                       const char*,
                       const LwDictType,
                       const LwCompression,
                       const LwEncoding,
                       gboolean, gboolean, gboolean);
void lw_dictinst_deinit (LwDictInst*);


void lw_dictinst_set_filename (LwDictInst*, const char*);
void lw_dictinst_set_type (LwDictInst*, const LwDictType);
void lw_dictinst_set_encoding (LwDictInst*, const LwEncoding);
void lw_dictinst_set_compression (LwDictInst*, const LwCompression);
void lw_dictinst_set_download_source (LwDictInst*, const char*);
void lw_dictinst_set_split (LwDictInst *di, const gboolean);
void lw_dictinst_set_merge (LwDictInst *di, const gboolean);
void lw_dictinst_set_status (LwDictInst *di, const LwDictInstUri);
gchar* lw_dictinst_get_status_string (LwDictInst*, gboolean);

void lw_dictinst_regenerate_save_target_uris (LwDictInst*);
gboolean lw_dictinst_data_is_valid (LwDictInst*);

gboolean lw_dictinst_install (LwDictInst*, LwIoProgressCallback, gpointer, GError**);
char* lw_dictinst_get_target_uri (LwDictInst*, const LwDictInstUri, const int);
char* lw_dictinst_get_source_uri (LwDictInst*, const LwDictInstUri, const int);

double lw_dictinst_get_total_progress (LwDictInst*, double);
double lw_dictinst_get_process_progress (LwDictInst*, double);

void lw_dictinst_set_cancel_operations (LwDictInst*, gboolean);

void gw_dictinst_update_source_uri_cb (GSettings*, char*, gpointer);

G_END_DECLS

#endif

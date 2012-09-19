#ifndef GW_DICTINST_HEADER_INCLUDED
#define GW_DICTINST_HEADER_INCLUDED

#define GW_DICTINST_ERROR "gWaei Dictionary Installer Error"

typedef enum {
  GW_DICTINST_ERROR_SOURCE_PATH,
  GW_DICTINST_ERROR_TARGET_PATH,
  GW_DICTINST_ERROR_FILE_MOVE
} LwDictInstError;

typedef enum {
  GW_DICTINST_NEEDS_DOWNLOADING,
  GW_DICTINST_NEEDS_DECOMPRESSION,
  GW_DICTINST_NEEDS_TEXT_ENCODING,
  GW_DICTINST_NEEDS_POSTPROCESSING,
  GW_DICTINST_NEEDS_FINALIZATION,
  GW_DICTINST_NEEDS_NOTHING,
  GW_DICTINST_TOTAL_URIS
} LwDictInstUri;


struct _LwDictInst {
  char *filename;
  char *shortname;
  char *longname;
  char *description;
  char *uri[GW_DICTINST_TOTAL_URIS];
  double progress;
  gboolean selected;
  char *schema;
  char *key;
  gboolean builtin;
  gulong listenerid;            //!< An id to hold the g_signal_connect value when the source copy uri pref is set
  gboolean listenerid_is_set;   //!< Allows disconnecting the signal on destruction of the LwDictInst
  LwCompression compression;    //!< Path to the gziped dictionary file
  LwEncoding encoding;          //!< Path to the raw unziped dictionary file
  LwEngine engine;
  LwDictInstUri uri_group_index;
  int uri_atom_index;
  char **current_source_uris;
  char **current_target_uris;
  gboolean split;
  gboolean merge;
  GMutex *mutex;
};
typedef struct _LwDictInst LwDictInst;

LwDictInst* lw_dictinst_new_using_pref_uri (const char*, 
                                            const char*,
                                            const char*,
                                            const char*,
                                            const char*,
                                            const char*,
                                            const LwEngine,
                                            const LwCompression,
                                            const LwEncoding,
                                            gboolean, gboolean, gboolean);

LwDictInst* lw_dictinst_new (const char*,
                             const char*,
                             const char*,
                             const char*,
                             const char*,
                             const LwEngine,
                             const LwCompression,
                             const LwEncoding,
                             gboolean, gboolean, gboolean);

void lw_dictinst_free (LwDictInst*);

void lw_dictinst_set_filename (LwDictInst*, const char*);
void lw_dictinst_set_engine (LwDictInst*, const LwEngine);
void lw_dictinst_set_encoding (LwDictInst*, const LwEncoding);
void lw_dictinst_set_compression (LwDictInst*, const LwCompression);
void lw_dictinst_set_download_source (LwDictInst*, const char*);
void lw_dictinst_set_split (LwDictInst *di, const gboolean);
void lw_dictinst_set_merge (LwDictInst *di, const gboolean);
void lw_dictinst_set_status (LwDictInst *di, const LwDictInstUri);
gchar* lw_dictinst_get_status_string (LwDictInst*, gboolean);

void lw_dictinst_regenerate_save_target_uris (LwDictInst*);
gboolean lw_dictinst_data_is_valid (LwDictInst*);

gboolean lw_dictinst_install (LwDictInst*, LwIoProgressCallback, GError**);
char* lw_dictinst_get_target_uri (LwDictInst*, const LwDictInstUri, const int);
char* lw_dictinst_get_source_uri (LwDictInst*, const LwDictInstUri, const int);

double lw_dictinst_get_total_progress (LwDictInst*, double);
double lw_dictinst_get_process_progress (LwDictInst*, double);

void lw_dictinst_set_cancel_operations (LwDictInst*, gboolean);

#endif

#ifndef LW_DICTIONARY_INSTALLER_INCLUDED
#define LW_DICTIONARY_INSTALLER_INCLUDED

G_BEGIN_DECLS

typedef enum {
  LW_DICTIONARY_INSTALLER_STATUS_UNINSTALLED,
  LW_DICTIONARY_INSTALLER_STATUS_DOWNLOADING,
  LW_DICTIONARY_INSTALLER_STATUS_DECOMPRESSING,
  LW_DICTIONARY_INSTALLER_STATUS_ENCODING,
  LW_DICTIONARY_INSTALLER_STATUS_POSTPROCESSING,
  LW_DICTIONARY_INSTALLER_STATUS_FINISHING,
  LW_DICTIONARY_INSTALLER_STATUS_INSTALLED,
  TOTAL_LW_DICTIONARY_INSTALLER_STATUSES
} LwDictionaryInstallerStatus;

//Methods

LwDictionaryInstall* lw_dictionaryinstall_new (void);
void lw_dictionaryinstall_free (LwDictionaryInstall*);

LwDictionaryInstallerStatus lw_dictionary_installer_get_status (LwDictionary*);
void lw_dictionary_installer_set_status (LwDictionary*, LwDictionaryInstallerStatus);
gint lw_dictionary_installer_get_file_index (LwDictionary*);
gboolean lw_dictionary_installer_is_valid (LwDictionary*);

gboolean lw_dictionary_installer_download (LwDictionary*, GCancellable*, GError**);
gboolean lw_dictionary_installer_decompress (LwDictionary*, GCancellable*, GError**);
gboolean lw_dictionary_installer_convert_encoding (LwDictionary*, GCancellable*, GError**);
gboolean lw_dictionary_installer_postprocess (LwDictionary*, GCancellable*, GError**);
gboolean lw_dictionary_installer_install (LwDictionary*, GCancellable*, GError**);
void lw_dictionary_installer_clean (LwDictionary*, GCancellable*);

gdouble lw_dictionary_installer_get_progress (LwDictionary*);
gdouble lw_dictionary_installer_get_stage_progress (LwDictionary*);
gdouble lw_dictionary_installer_get_total_progress (LwDictionary*);
gchar* lw_dictionary_installer_get_status_message (LwDictionary*, gboolean);

gboolean lw_dictionary_installer_get_postprocessing (LwDictionary*);
void lw_dictionary_installer_set_postprocessing (LwDictionary*, gboolean);
LwEncoding lw_dictionary_installer_get_encoding (LwDictionary*);
void lw_dictionary_installer_set_encoding (LwDictionary*, const LwEncoding);
const gchar* lw_dictionary_installer_get_downloads (LwDictionary*);
void lw_dictionary_installer_set_downloads (LwDictionary*, const gchar*);
void lw_dictionary_installer_reset_downloads (LwDictionary*);

void lw_dictionary_installer_set_files (LwDictionary *dictionary, const gchar*);
const gchar* lw_dictionary_installer_get_files (LwDictionary *dictionary);

const gchar* lw_dictionary_installer_get_description (LwDictionary*);

gboolean lw_dictionary_installer_is_builtin (LwDictionary*);

G_END_DECLS

#endif

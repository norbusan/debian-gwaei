#ifndef LW_IO_INCLUDED
#define LW_IO_INCLUDED

#include <glib.h>

G_BEGIN_DECLS

#define LW_IO_MAX_FGETS_LINE 5000
#define LW_IO_ERROR "libwaei generic error"

typedef int (*LwIoProgressCallback) (double percent, gpointer data);

struct _LwIoProgressCallbackWithData {
  LwIoProgressCallback cb;
  gpointer data;
};
typedef struct _LwIoProgressCallbackWithData LwIoProgressCallbackWithData;

typedef enum  {
  LW_IO_READ_ERROR,
  LW_IO_WRITE_ERROR,
  LW_IO_DECOMPRESSION_ERROR,
  LW_IO_COPY_ERROR,
  LW_IO_DOWNLOAD_ERROR,
  LW_IO_ENCODING_CONVERSION_ERROR
} LwIoErrorTypes;

void lw_io_write_file (const char*, const char*, gchar*, LwIoProgressCallback, gpointer, GError**);
char** lw_io_get_dictionary_file_list (const int);
size_t lw_io_get_filesize (const char*);

gboolean lw_io_create_mix_dictionary (const char*, const char*, const char*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_io_split_places_from_names_dictionary (const char*, const char*, const char*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_io_copy_with_encoding (const char*, const char*, const char*, const char*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_io_copy (const char*, const char*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_io_remove (const char*, GError**);
gboolean lw_io_download (char*, char*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_io_gunzip_file (const char*, const char*, LwIoProgressCallback, gpointer, GError **);
gboolean lw_io_unzip_file (char*, LwIoProgressCallback, gpointer, GError**);

void lw_io_set_savepath (const gchar *);
const gchar* lw_io_get_savepath (void);

void lw_io_set_cancel_operations (gboolean);
long lw_io_get_size_for_uri (const char*);

G_END_DECLS

#endif

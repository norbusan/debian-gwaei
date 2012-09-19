#ifndef LW_IO_INCLUDED
#define LW_IO_INCLUDED

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define LW_IO_MAX_FGETS_LINE 5000
#define LW_IO_ERROR "libwaei generic error"

typedef gint (*LwIoProgressCallback) (gdouble percent, gpointer data);

struct _LwIoProgressCallbackWithData {
  LwIoProgressCallback cb;
  gpointer data;
  GCancellable *cancellable;
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

void lw_io_write_file (const gchar*, const gchar*, gchar*, LwIoProgressCallback, gpointer, GError**);
size_t lw_io_get_filesize (const gchar*);

gboolean lw_io_create_mix_dictionary (const gchar*, const gchar*, const gchar*, LwIoProgressCallback, gpointer, GCancellable*, GError**);
gboolean lw_io_split_places_from_names_dictionary (const gchar*, const gchar*, const gchar*, LwIoProgressCallback, gpointer, GCancellable*, GError**);
gboolean lw_io_copy_with_encoding (const gchar*, const gchar*, const gchar*, const gchar*, LwIoProgressCallback, gpointer, GCancellable*, GError**);
gboolean lw_io_copy (const gchar*, const gchar*, LwIoProgressCallback, gpointer, GCancellable*, GError**);
gboolean lw_io_remove (const gchar*, GCancellable*, GError**);
gboolean lw_io_download (const gchar*, const gchar*, LwIoProgressCallback, gpointer, GCancellable*, GError**);
gboolean lw_io_gunzip_file (const gchar*, const gchar*, LwIoProgressCallback, gpointer, GCancellable*, GError **);
gboolean lw_io_unzip_file (gchar*, LwIoProgressCallback, gpointer, GCancellable*, GError**);

void lw_io_set_savepath (const gchar *);
const gchar* lw_io_get_savepath (void);

long lw_io_get_size_for_uri (const gchar*);

G_END_DECLS

#endif

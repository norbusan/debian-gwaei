#ifndef LW_DICTINFO_INCLUDED
#define LW_DICTINFO_INCLUDED

#include <libwaei/dict.h>
#include <libwaei/resultline.h>

G_BEGIN_DECLS

#define LW_DICTINFO(object) (LwDictInfo*) object

//!
//! @brief Primitive for storing dictionary information
//!
struct _LwDictInfo
{
    EXTENDS_LW_DICT
    int load_position;                //!< load position in the GUI
    long length;                    //!< Length of the file
    LwResultLine *cached_resultlines; //!< Allocated resultline swapped with current_resultline when needed
    LwResultLine *current_resultline; //!< Allocated resultline where the current parsed result data resides
};
typedef struct _LwDictInfo LwDictInfo;


LwDictInfo* lw_dictinfo_new (const LwDictType, const char*);
void lw_dictinfo_free (LwDictInfo*);
void lw_dictinfo_init (LwDictInfo*, const LwDictType, const char*);
void lw_dictinfo_deinit (LwDictInfo*);

gboolean lw_dictinfo_uninstall (LwDictInfo*, LwIoProgressCallback, GError**);
char* lw_dictinfo_get_uri (LwDictInfo*);

G_END_DECLS

#endif

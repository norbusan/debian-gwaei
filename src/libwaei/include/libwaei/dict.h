#ifndef LW_DICT_INCLUDED
#define LW_DICT_INCLUDED

G_BEGIN_DECLS

#define LW_DICT(object) (LwDict*) object

#define EXTENDS_LW_DICT \
  char *filename;  \
  char *longname;  \
  char *shortname; \
  LwDictType type; 

//!
//! @brief Dictionary type assigned by the program.  It determines the parsing algorithm
//!
typedef enum {  
  LW_DICTTYPE_EDICT,         //!< Standard edict format dictionary
  LW_DICTTYPE_KANJI,         //!< Kanjidic format dictionary
  LW_DICTTYPE_EXAMPLES,      //!< Examples format dictionary
  LW_DICTTYPE_UNKNOWN,          //!< Unkown format which should use safe parsing
  TOTAL_LW_DICTTYPES
} LwDictType;

G_END_DECLS

#endif


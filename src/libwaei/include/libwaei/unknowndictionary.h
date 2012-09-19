#ifndef LW_UNKNOWNDICTIONARY_INCLUDED
#define LW_UNKNOWNDICTIONARY_INCLUDED

G_BEGIN_DECLS


//Boilerplate
typedef struct _LwUnknownDictionary LwUnknownDictionary;
typedef struct _LwUnknownDictionaryClass LwUnknownDictionaryClass;
typedef struct _LwUnknownDictionaryPrivate LwUnknownDictionaryPrivate;

#define LW_TYPE_UNKNOWNDICTIONARY              (lw_unknowndictionary_get_type())
#define LW_UNKNOWNDICTIONARY(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_UNKNOWNDICTIONARY, LwUnknownDictionary))
#define LW_UNKNOWNDICTIONARY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_UNKNOWNDICTIONARY, LwUnknownDictionaryClass))
#define LW_IS_UNKNOWNDICTIONARY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_UNKNOWNDICTIONARY))
#define LW_IS_UNKNOWNDICTIONARY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_UNKNOWNDICTIONARY))
#define LW_UNKNOWNDICTIONARY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_UNKNOWNDICTIONARY, LwUnknownDictionaryClass))

struct _LwUnknownDictionary {
  LwDictionary object;
};

struct _LwUnknownDictionaryClass {
  LwDictionaryClass parent_class;
};

//Methods
LwDictionary* lw_unknowndictionary_new (const gchar*);
GType lw_unknowndictionary_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif


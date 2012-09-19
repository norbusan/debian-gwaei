#ifndef LW_EXAMPLEDICTIONARY_INCLUDED
#define LW_EXAMPLEDICTIONARY_INCLUDED

G_BEGIN_DECLS


//Boilerplate
typedef struct _LwExampleDictionary LwExampleDictionary;
typedef struct _LwExampleDictionaryClass LwExampleDictionaryClass;
typedef struct _LwExampleDictionaryPrivate LwExampleDictionaryPrivate;

#define LW_TYPE_EXAMPLEDICTIONARY              (lw_exampledictionary_get_type())
#define LW_EXAMPLEDICTIONARY(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_EXAMPLEDICTIONARY, LwExampleDictionary))
#define LW_EXAMPLEDICTIONARY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_EXAMPLEDICTIONARY, LwExampleDictionaryClass))
#define LW_IS_EXAMPLEDICTIONARY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_EXAMPLEDICTIONARY))
#define LW_IS_EXAMPLEDICTIONARY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_EXAMPLEDICTIONARY))
#define LW_EXAMPLEDICTIONARY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_EXAMPLEDICTIONARY, LwExampleDictionaryClass))

struct _LwExampleDictionary {
  LwDictionary object;
};

struct _LwExampleDictionaryClass {
  LwDictionaryClass parent_class;
};

//Methods
LwDictionary* lw_exampledictionary_new (const gchar*);
GType lw_exampledictionary_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif


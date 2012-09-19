#ifndef GW_TEXTTAGTABLE_INCLUDED
#define GW_TEXTTAGTABLE_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwTextTagTable GwTextTagTable;
typedef struct _GwTextTagTableClass GwTextTagTableClass;
typedef struct _GwTextTagTablePrivate GwTextTagTablePrivate;

#define GW_TYPE_TEXTTAGTABLE              (gw_texttagtable_get_type())
#define GW_TEXTTAGTABLE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_TEXTTAGTABLE, GwTextTagTable))
#define GW_TEXTTAGTABLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_TEXTTAGTABLE, GwTextTagTableClass))
#define GW_IS_TEXTTAGTABLE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_TEXTTAGTABLE))
#define GW_IS_TEXTTAGTABLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_TEXTTAGTABLE))
#define GW_TEXTTAGTABLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_TEXTTAGTABLE, GwTextTagTableClass))

struct _GwTextTagTable {
  GtkTextTagTable tagtable;
  GwTextTagTablePrivate *priv;
};

struct _GwTextTagTableClass {
  GtkTextTagTableClass parent_class;
};

//Methods
GtkTextTagTable* gw_texttagtable_new (GwApplication*);
GType gw_texttagtable_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif

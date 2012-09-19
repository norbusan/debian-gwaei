#ifndef GW_DICTIONARYINSTALLWINDOW_PRIVATE_INCLUDED
#define GW_DICTIONARYINSTALLWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

typedef enum {
   GW_DICTINSTWINDOW_DICTSTOREFIELD_SHORT_NAME,
   GW_DICTINSTWINDOW_DICTSTOREFIELD_LONG_NAME,
   GW_DICTINSTWINDOW_DICTSTOREFIELD_DICTINST_PTR,
   GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, 
   TOTAL_GW_DICTINSTWINDOW_DICTSTOREFIELDS
} GwDictionaryStoreField;

typedef enum {
  GW_DICTINSTWINDOW_ENGINESTOREFIELD_ID,
  GW_DICTINSTWINDOW_ENGINESTOREFIELD_NAME,
  TOTAL_GW_DICTINSTWINDOW_ENGINESTOREFIELDS
} GwEngineStoreField;

typedef enum {
  GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELD_ID, 
  GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELD_NAME,
  TOTAL_GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELDS
} GwCompressionStoreField;

typedef enum {
  GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_ID,
  GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_NAME,
  TOTAL_GW_DICTINSTWINDOW_ENCODINGSTOREFIELDS
} GwEncodingStoreField;


struct _GwDictionaryInstallWindowPrivate {
  GtkListStore *encoding_store;
  GtkListStore *compression_store;
  GtkListStore *engine_store;
  GtkListStore *dictionary_store;
  LwDictInst *di;

  GtkTreeView *view;
  GtkButton *add_button;
  GtkButton *cancel_button;
  GtkToggleButton *details_togglebutton;
  GtkBox *details_hbox;

  //Volatile pointers: They are not available through gtkbuilder
  GtkEntry* filename_entry;
  GtkComboBox* engine_combobox;
  GtkEntry* source_entry;
  GtkButton* source_choose_button;
  GtkButton* source_reset_button;
  GtkComboBox* encoding_combobox;
  GtkComboBox* compression_combobox;
  GtkCheckButton* split_checkbutton;
  GtkCheckButton* merge_checkbutton;
};

#define GW_DICTIONARYINSTALLWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_DICTIONARYINSTALLWINDOW, GwDictionaryInstallWindowPrivate))

G_END_DECLS

#endif

#ifndef GW_GTK_SETTINGS_INTERFACE_INSTALL_LINE_INCLUDED
#define GW_GTK_SETTINGS_INTERFACE_INSTALL_LINE_INCLUDED

//Class
struct _GwUiDictInstallLine {
    GtkWidget *status_icon;
    GtkWidget *status_message;
    GtkWidget *status_progressbar;
    GtkWidget *status_hbox;
    GtkWidget *message_hbox;

    GtkWidget *action_button;
    GtkWidget *action_button_hbox;

    GtkWidget *source_uri_entry;
    GtkWidget *source_browse_button;
    GtkWidget *source_reset_button;
    GtkWidget *source_hbox;

    GtkWidget *advanced_expander;
    GtkWidget *advanced_hbox;

    GwDictInfo *di;
};
typedef struct _GwUiDictInstallLine GwUiDictInstallLine;


//Methods
GwUiDictInstallLine *gw_ui_new_dict_install_line (GwDictInfo*);
void gw_ui_dict_install_set_action_button (GwUiDictInstallLine*, const gchar*, gboolean);
void gw_ui_dict_install_set_message (GwUiDictInstallLine*, const gchar*, const char*);
void gw_ui_progressbar_set_fraction_by_install_line (GwUiDictInstallLine*, const gdouble);
void gw_ui_add_dict_install_line_to_table (GtkTable*, GwUiDictInstallLine*);


#endif

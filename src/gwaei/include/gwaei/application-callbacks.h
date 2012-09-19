#ifndef GW_APPLICATION_CALLBACKS_INCLUDED
#define GW_APPLICATION_CALLBACKS_INCLUDED

void gw_application_quit_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_preferences_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_settingswindow_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_aboutdialog_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_searchwindow_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_vocabularywindow_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_vocabularywindow_index_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_glossary_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_help_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_homepage_cb (GSimpleAction*, GVariant*, gpointer);
#ifdef WITH_HUNSPELL
void gw_application_sync_spellcheck_cb (GSettings*, gchar*, gpointer);
#endif

#endif

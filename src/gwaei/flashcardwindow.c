/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file flashcardwindow.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/flashcardwindow-private.h>


//Static declarations
static void gw_flashcardwindow_attach_signals (GwFlashCardWindow*);
//static void gw_flashcardwindow_remove_signals (GwFlashCardWindow*);

static void gw_flashcardwindow_init_accelerators (GwFlashCardWindow*);

G_DEFINE_TYPE (GwFlashCardWindow, gw_flashcardwindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_flashcardwindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwFlashCardWindow *window;

    //Initializations
    window = GW_FLASHCARDWINDOW (g_object_new (GW_TYPE_FLASHCARDWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "flashcardwindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


static void 
gw_flashcardwindow_init (GwFlashCardWindow *window)
{
    window->priv = GW_FLASHCARDWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwFlashCardWindowPrivate));

/*
    GwFlashCardWindowPrivate *priv;
    priv = window->priv;
*/
}


static void 
gw_flashcardwindow_finalize (GObject *object)
{
    GwFlashCardWindow *window;
    GwFlashCardWindowPrivate *priv;

    window = GW_FLASHCARDWINDOW (object);
    priv = window->priv;

    if (priv->list_name != NULL) g_free (priv->list_name); priv->list_name = NULL;
    if (priv->question_title != NULL) g_free (priv->question_title); priv->question_title = NULL;
    if (priv->question != NULL) g_free (priv->question); priv->question = NULL;
    if (priv->answer != NULL) g_free (priv->answer); priv->answer = NULL;
    if (priv->flash_cards_type != NULL) g_free (priv->flash_cards_type); priv->flash_cards_type = NULL;

    if (priv->model != NULL) 
    {
      g_object_unref (priv->model); priv->model = NULL;
    }

    gw_window_save_size (GW_WINDOW (window));

    G_OBJECT_CLASS (gw_flashcardwindow_parent_class)->finalize (object);
}


static void 
gw_flashcardwindow_constructed (GObject *object)
{
    //Declarations
    GwFlashCardWindow *window;
    GwFlashCardWindowPrivate *priv;
    GtkStyleContext *context;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_flashcardwindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_FLASHCARDWINDOW (object);
    priv = window->priv;

    //Set up the gtkbuilder links
    priv->content_box = GTK_BOX (gw_window_get_object (GW_WINDOW (window), "content_box"));
    priv->finished_box = GTK_BOX (gw_window_get_object (GW_WINDOW (window), "finished_box"));

    priv->card_toolbar = GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "card_toolbar"));
    priv->card_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "card_label"));
    priv->card_scrolledwindow = GTK_SCROLLED_WINDOW (gw_window_get_object (GW_WINDOW (window), "card_scrolledwindow"));
    priv->check_answer_toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "submit_toolbutton"));
    priv->next_card_toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "next_toolbutton"));
    priv->dont_know_toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "dont_know_toolbutton"));
    priv->answer_entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "submit_entry"));
    priv->correct_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "right_label"));
    priv->incorrect_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "wrong_label"));
    priv->status_progressbar = GTK_PROGRESS_BAR (gw_window_get_object (GW_WINDOW (window), "status_progressbar"));
    priv->status_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "status_label"));
    priv->track_togglebutton = GTK_TOGGLE_BUTTON (gw_window_get_object (GW_WINDOW (window), "track_checkbutton"));
    priv->finished_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "finished_label"));

    //Set up the gtk window
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_set_default_size (GTK_WINDOW (window), 450, 300);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_has_resize_grip (GTK_WINDOW (window), TRUE);
    gw_window_set_is_important (GW_WINDOW (window), TRUE);
    gw_window_load_size (GW_WINDOW (window));

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->card_toolbar));
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
    gtk_style_context_add_class (context, "primary-toolbar");
    gtk_widget_reset_style (GTK_WIDGET (priv->card_toolbar));

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->card_scrolledwindow));
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);

    gw_flashcardwindow_init_accelerators (window);
    gw_flashcardwindow_attach_signals (window);

    priv->time = g_get_monotonic_time ();

    gw_window_unload_xml (GW_WINDOW (window));
}


static void
gw_flashcardwindow_class_init (GwFlashCardWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_flashcardwindow_constructed;
  object_class->finalize = gw_flashcardwindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwFlashCardWindowPrivate));
}


static void
gw_flashcardwindow_init_accelerators (GwFlashCardWindow *window)
{
/*
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;

    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));
    //Set menu accelerators
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_window_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
*/
}


static void
gw_flashcardwindow_attach_signals (GwFlashCardWindow *window)
{
    g_signal_connect (G_OBJECT (window), "key-press-event", G_CALLBACK (gw_flashcardwindow_key_press_event_cb), window);
}

/*
static void 
gw_flashcardwindow_remove_signals (GwFlashCardWindow *window)
{
}
*/


void
gw_flashcardwindow_update_title (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gchar *title;

    priv = window->priv;
    title = g_strdup_printf (gettext("%s - gWaei %s Flashcards"), priv->list_name, priv->flash_cards_type);
    if (title != NULL)
    {
      gtk_window_set_title (GTK_WINDOW (window), title);
      g_free (title);
    }
}


void
gw_flashcardwindow_set_model (GwFlashCardWindow *window, 
                              GwFlashCardStore  *store,
                              const gchar       *flash_cards_type,
                              const gchar       *list_name,
                              const gchar       *question_title
                              )
{
    if (window == NULL) return;
    if (store == NULL) return;

    GwFlashCardWindowPrivate *priv;

    priv = window->priv;

    if (priv->model != NULL) g_object_unref (priv->model); priv->model = NULL;
    if (priv->list_name != NULL) g_free (priv->list_name); priv->list_name = NULL;
    if (priv->question_title != NULL) g_free (priv->question_title); priv->question_title = NULL;
    if (priv->flash_cards_type != NULL) g_free (priv->flash_cards_type); priv->flash_cards_type = NULL;

    priv->total_cards = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (store), NULL);
    if (priv->total_cards == 0) return;
    priv->cards_left = priv->total_cards;
    priv->model = GTK_TREE_MODEL (store);
    priv->list_name = g_strdup (list_name);
    priv->question_title = g_strdup (question_title);
    priv->flash_cards_type = g_strdup (flash_cards_type);

    gtk_tree_model_get_iter_first (priv->model, &(priv->iter));
    gw_flashcardwindow_set_correct_guesses (window, 0);
    gw_flashcardwindow_set_incorrect_guesses (window, 0);
    gw_flashcardwindow_update_progress (window);

    gw_flashcardwindow_update_title (window);
    gw_flashcardwindow_load_iterator (window, FALSE, FALSE);
}


gboolean
gw_flashcardwindow_user_answer_is_correct (GwFlashCardWindow *window)
{
    //Declarations
    GwFlashCardWindowPrivate *priv;
    gchar *user;
    gchar *card;
    gboolean is_correct;
    gboolean convertable;
    const gint MAX = 100;
    gchar hiragana[MAX];
    gchar katakana[MAX];

    //Initializations
    priv = window->priv;
    user = lw_util_collapse_string (gtk_entry_get_text (priv->answer_entry));
    card = lw_util_collapse_string (priv->answer);
    convertable = lw_util_str_roma_to_hira (user, hiragana, MAX);

    if (user == NULL || card == NULL || *user == '\0')
    {
      is_correct = FALSE;
    }
    else if (convertable)
    {
      strcpy(katakana, hiragana);
      lw_util_str_shift_hira_to_kata (katakana);
      is_correct = (strstr(card, user) != NULL || strstr(card, hiragana) != NULL || strstr(card, katakana) != NULL);
    }
    else
    {
      is_correct = (strstr(card, user) != NULL);
    }

    if (user != NULL) g_free (card);
    if (card != NULL) g_free (user);

    return is_correct;
}


void
gw_flashcardwindow_load_iterator (GwFlashCardWindow *window, gboolean show_answer, gboolean answer_correct)
{
    GwFlashCardWindowPrivate *priv;
    gchar *markup;

    priv = window->priv;

    if (priv->model == NULL) return;

    if (priv->question != NULL) g_free (priv->question);
    if (priv->answer != NULL) g_free (priv->answer);

    if (show_answer)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (priv->answer_entry), !show_answer);
      gtk_widget_set_sensitive (GTK_WIDGET (priv->check_answer_toolbutton), !show_answer);
      gtk_widget_set_sensitive (GTK_WIDGET (priv->dont_know_toolbutton), !show_answer);
    }
    else
    {
      gtk_widget_set_sensitive (GTK_WIDGET (priv->answer_entry), !show_answer);
      gtk_widget_set_sensitive (GTK_WIDGET (priv->check_answer_toolbutton), !show_answer);
      gtk_widget_set_sensitive (GTK_WIDGET (priv->dont_know_toolbutton), !show_answer);
      gtk_entry_set_text (priv->answer_entry, "");
      gtk_widget_grab_focus (GTK_WIDGET (priv->answer_entry));
    }

    gtk_tree_model_get (priv->model, &(priv->iter), 
        GW_FLASHCARDSTORE_COLUMN_QUESTION, &priv->question, 
        GW_FLASHCARDSTORE_COLUMN_ANSWER, &priv->answer, -1);

    if (priv->question != NULL && priv->answer != NULL)
    {
      if (show_answer && answer_correct)
        markup = g_markup_printf_escaped (
          "<big><b>%s</b>\n%s"
          "<small>\n\n</small>"
          "  <span foreground=\"green\" size=\"larger\" weight=\"semibold\">%s</span>"
          "<small>\n</small>"
          "  <span size=\"smaller\" weight=\"semibold\">%s</span>"
          "<small>\n\n</small>"
          "<b>%s:</b>\n%s</big>\n",
          priv->question_title, priv->question, 
          gettext("You were correct!"),
          gettext("Press [Enter] to continue..."),
          gettext("Answer"), priv->answer
        );
      else if (show_answer && !answer_correct)
        markup = g_markup_printf_escaped (
          "<big><b>%s</b>\n%s"
          "<small>\n\n</small>"
          "  <span foreground=\"red\" size=\"larger\" weight=\"semibold\">%s</span>"
          "<small>\n</small>"
          "  <span size=\"smaller\" weight=\"semibold\">%s</span>"
          "<small>\n\n</small>"
          "<b>%s:</b>\n%s</big>\n",
          priv->question_title, priv->question, 
          gettext("You were incorrect!"),
          gettext("Press [Enter] to continue..."),
          gettext("Answer"), priv->answer
        );
      else
        markup = g_markup_printf_escaped ("<big><b>%s</b>\n%s</big>\n\n\n\n\n\n\n\n\n", 
            priv->question_title, priv->question
        );
      if (markup != NULL)
      {
        gtk_label_set_markup (priv->card_label, markup);
        g_free (markup);
      }
    }
}


gboolean
gw_flashcardwindow_iterate (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gboolean valid;
    gboolean completed;

    priv = window->priv;

    if (priv->model == NULL) return FALSE;

    g_assert (priv->total_cards != 0);

    if (priv->cards_left == 0)
    {
      gw_flashcardwindow_set_finished (window);
      return FALSE;
    }

    valid = TRUE;
    completed = TRUE;

    while (valid && completed)
    {
      valid = gtk_tree_model_iter_next (priv->model, &(priv->iter));
      if (!valid) valid = gtk_tree_model_get_iter_first (priv->model, &(priv->iter));
      gtk_tree_model_get (priv->model, &(priv->iter), GW_FLASHCARDSTORE_COLUMN_IS_COMPLETED, &completed, -1);
    }

    gw_flashcardwindow_load_iterator (window, FALSE, FALSE);

    return (priv->cards_left > 0);
}


void
gw_flashcardwindow_set_card_completed (GwFlashCardWindow *window, gboolean completed)
{
    if (window == NULL) return; 

    GwFlashCardWindowPrivate *priv;
    GwFlashCardStore *store;

    priv = window->priv;
    if (priv->model == NULL) return;
    store = GW_FLASHCARDSTORE (priv->model);

    gw_flashcardstore_set_completed (store, &(priv->iter), completed);
}


void
gw_flashcardwindow_increment_incorrect_guesses (GwFlashCardWindow *window)
{
    gint guesses;
    guesses = gw_flashcardwindow_get_incorrect_guesses (window);
    gw_flashcardwindow_set_incorrect_guesses (window, guesses + 1);
}


gint
gw_flashcardwindow_get_incorrect_guesses (GwFlashCardWindow *window)
{
    //Sanity check
    if (window == NULL) return -1;

    GwFlashCardWindowPrivate *priv;

    priv = window->priv;

    return priv->incorrect_guesses;
}


void
gw_flashcardwindow_set_incorrect_guesses (GwFlashCardWindow *window, gint new_guesses)
{
    //Sanity check
    if (window == NULL) return;
    if (new_guesses < 0) new_guesses = 0;

    //Declarations
    GwFlashCardWindowPrivate *priv;
    GwFlashCardStore *store;
    gchar *text;
    gint guess_delta, old_guesses;

    //Initializations
    priv = window->priv;
    if (priv->model == NULL) return;
    store = GW_FLASHCARDSTORE (priv->model);
    old_guesses = priv->incorrect_guesses;
    guess_delta = new_guesses - old_guesses;
    priv->incorrect_guesses = new_guesses;
    text = g_strdup_printf ("%d", priv->incorrect_guesses);

    //Update the totall correct guesses for the flashcard session
    if (text != NULL)
    {
      gtk_label_set_text (priv->incorrect_label, text);
      g_free (text); text = NULL;
    }

    //Update the tally for this specific word
    old_guesses = gw_flashcardstore_get_incorrect_guesses (store, &(priv->iter));
    new_guesses = old_guesses + guess_delta;
    gw_flashcardstore_set_incorrect_guesses (store, &(priv->iter), new_guesses, priv->track);
}


void
gw_flashcardwindow_increment_correct_guesses (GwFlashCardWindow *window)
{
    gint guesses;
    guesses = gw_flashcardwindow_get_correct_guesses (window);
    gw_flashcardwindow_set_correct_guesses (window, guesses + 1);
}


gint
gw_flashcardwindow_get_correct_guesses (GwFlashCardWindow *window)
{
    //Sanity check
    if (window == NULL) return -1;

    GwFlashCardWindowPrivate *priv;

    priv = window->priv;

    return priv->correct_guesses;
}


void
gw_flashcardwindow_set_correct_guesses (GwFlashCardWindow *window, gint new_guesses)
{
    //Sanity check
    if (window == NULL) return;
    if (new_guesses < 0) return;

    //Declarations
    GwFlashCardWindowPrivate *priv;
    GwFlashCardStore *store;
    gchar *text;
    gint guess_delta, old_guesses;

    //Initializations
    priv = window->priv;
    if (priv->model == NULL) return;
    store = GW_FLASHCARDSTORE (priv->model);
    old_guesses = priv->correct_guesses;
    guess_delta = new_guesses - old_guesses;
    priv->correct_guesses = new_guesses;
    text = g_strdup_printf ("%d", priv->correct_guesses);

    //Update the totall correct guesses for the flashcard session
    if (text != NULL)
    {
      gtk_label_set_text (priv->correct_label, text);
      g_free (text); text = NULL;
    }

    //Update the tally for this specific word
    old_guesses = gw_flashcardstore_get_correct_guesses (store, &(priv->iter));
    new_guesses = old_guesses + guess_delta;
    gw_flashcardstore_set_correct_guesses (store, &(priv->iter), new_guesses, priv->track);
}


void
gw_flashcardwindow_update_progress (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gchar *text;
    gdouble fraction;

    priv = window->priv;
    fraction = (gdouble) (priv->total_cards - priv->cards_left) / (gdouble) priv->total_cards;
    text = g_strdup_printf (ngettext("%d Flashcard Left...", "%d Flashcards Left...", priv->cards_left), priv->cards_left);

    gtk_progress_bar_set_fraction (priv->status_progressbar, fraction);
    gtk_progress_bar_set_text (priv->status_progressbar, text);

    if (text != NULL) g_free (text);
}


void
gw_flashcardwindow_check_answer (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gboolean correct;

    priv = window->priv;
    correct = gw_flashcardwindow_user_answer_is_correct (window);

    if (correct)
    {
      if (priv->cards_left > 0) priv->cards_left--;
      gw_flashcardwindow_set_card_completed (window, TRUE);
      gw_flashcardwindow_increment_correct_guesses (window);
      gw_flashcardwindow_update_progress (window);
    }
    else
    {
      gw_flashcardwindow_increment_incorrect_guesses (window);
    }

    gw_flashcardwindow_load_iterator (window, TRUE, correct);
}



void
gw_flashcardwindow_set_finished (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    GtkLabel *label;
    gchar *markup;
    gint64 time;
    gint64 difference;
    gint hours, minutes, seconds;
    gint percent;

    priv = window->priv;
    label = priv->finished_label;
    time = g_get_monotonic_time ();
    difference = (time - priv->time) / 1000000;
    hours = difference / 60 / 60;
    minutes = difference / 60 % 60;
    seconds = difference % 60 % 60;
    percent = (priv->correct_guesses * 100) / (priv->correct_guesses + priv->incorrect_guesses);

    markup = g_markup_printf_escaped (
      "<big><b>%s</b></big>\n\n"
      "<b>%s:</b> %d%%\n"
      "<b>%s:</b> %02d:%02d:%02d\n",
      gettext("You've finished this flash card deck!"), 
      gettext("Your Grade"), percent, 
      gettext("Completion Time"), hours, minutes, seconds);
    if (markup != NULL)
    {
      gtk_label_set_markup (GTK_LABEL (label), markup);
      g_free (markup);
    }

    gtk_widget_hide (GTK_WIDGET (priv->content_box));
    gtk_widget_show (GTK_WIDGET (priv->finished_box));
}


void
gw_flashcardwindow_set_track_results (GwFlashCardWindow *window, gboolean request)
{
    GwFlashCardWindowPrivate *priv;
    GtkToggleButton *button;
    GtkWidget *toplevel;

    priv = window->priv;

    priv->track = request;
    button = priv->track_togglebutton;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (button, gw_flashcardwindow_track_results_toggled_cb, toplevel);
    gtk_toggle_button_set_active (button, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (button, gw_flashcardwindow_track_results_toggled_cb, toplevel);
}



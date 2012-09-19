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
//! @file src/ncurses.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with ncurses.
//!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>

#include <glib.h>
#include <curses.h>
#include <errno.h>

#include <waei/waei.h>

#define MAX_QUERY 100

static LwDictInfo *di = NULL;
static char query_text[MAX_QUERY];
static GMainLoop *main_loop = NULL;

static gboolean ncurses_switch = FALSE;
static gboolean quiet_switch = FALSE;
static gboolean list_switch = FALSE;
static gboolean version_switch = FALSE;

static char* dictionary_switch_data = NULL;
static char* install_switch_data = NULL;
static char* uninstall_switch_data = NULL;

static WINDOW *mainWindows;
static WINDOW *results;
static WINDOW *search;
static WINDOW *screen;
static int current_row = 0;
static int maxY, maxX;
static int cursesFlag = FALSE;
static int cursesSupportColorFlag = TRUE;	//TODO: use this


//!
//! @brief Print the "no result" message where necessary.
//!
void w_ncurses_no_result (LwSearchItem *item)
{
    wprintw(results,"%s\n\n", gettext("No results found!"));
}


//!
//! @brief This function prints the start banner
//!
//! This function prints the start banner in both
//! simple and ncurses interface.
//!
//! @param query The query string we are searching
//! @param dictionary The name (string) of the dictionary used
//!
static void w_ncurses_start_banner (char *query, char *dictionary)
{
    wprintw(screen, " ");
    // TRANSLATORS: First part of the sentence : 'Searching for "${query}" in the ${dictionary} dictionary'
    wprintw(screen, gettext("Searching for \""));
    wattron(screen, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));
    wprintw(screen,"%s", query);
    wattroff(screen, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));
    // TRANSLATORS: Second part of the sentence : 'Searching for "${query}" in the ${dictionary} dictionary'
    wprintw(screen, gettext("\" in the "));
    wattron(screen, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));
    wprintw(screen," %s", dictionary);
    wattroff(screen, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));
    // TRANSLATORS: Third and last part of the sentence : 'Searching for "${query}" in the ${dictionary} dictionary'
    wprintw(screen, gettext(" dictionary..."));

    wrefresh(screen);
    refresh();
}


//!
//! @brief Print the "less relevant" header where necessary.
//!
//! @param item A LwSearchItem to gleam information from
//!
void w_ncurses_append_less_relevant_header_to_output (LwSearchItem *item)
{
    wattron(results, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));
    wprintw(results,"\n*** ");
    wattroff(results, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));
    wprintw(results,"%s ", gettext("Other Results"));
    wattron(results, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));
    wprintw(results,"***************************\n\n\n");
    wattroff(results, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));
}

//!
//! @brief  Print the "more relevant" header where necessary
//!
//! This function gets called whenever another relevant result
//! is found.  You either need to use it to update the line once
//! written, or make sure that it only prints once.
//!
//! @param item A LwSearchItem to gleam information from
//!
void w_ncurses_append_more_relevant_header_to_output (LwSearchItem *item)
{
}


//!
//! @brief Color Initialization
//!
//! The pairs of colors that will be used in the printing
//! functions are initialized here.
//!
//! @param hasColors True if the shell support colors
//!
static void ncurses_color_init (bool hasColors)
{
	int check;	//Check flag
	if (hasColors)
	{
		check = start_color();
		if (check == ERR){
			cursesSupportColorFlag = FALSE;
			return;
		}
		init_pair(GW_NCCOLORS_GREENONBLACK, COLOR_GREEN, COLOR_BLACK);
		init_pair(GW_NCCOLORS_BLUEONBLACK, COLOR_BLUE, COLOR_BLACK);
		init_pair(GW_NCCOLORS_REDONBLACK, COLOR_RED, COLOR_BLACK);
	}
	else
		cursesSupportColorFlag = FALSE;

	return;
}


//!
//! @brief Create a box around a WINDOW and write a title
//!
//! Create a box around a WINDOW and write a title
//!
//! @param thisWin The window we want a box around
//! @param intro A string that we want to be title of the box
//!
static void ncurses_add_intro_and_box (WINDOW* thisWin, char* intro)
{
	//Draw  a box around the edges of a window
	box(thisWin, ACS_VLINE, ACS_HLINE);

	wattron(thisWin,A_BOLD);
	mvwprintw(thisWin,0,2,intro);
	wattroff(thisWin,A_BOLD);

	wrefresh(thisWin);
	wrefresh(stdscr);

	return;
}


//!
//! @brief Screen Initialization
//!
static int ncurses_screen_init ()
{
	int check, checkSecond;	//Check flag

	mainWindows = initscr();
	if (mainWindows == NULL)
	{
		printf("The main screen cannot be initialized. Exiting...\n");
		return(ERR);
	}

	check = cbreak();
	checkSecond = nodelay(mainWindows, TRUE);
	if ((check == ERR) || (checkSecond == ERR))
	{
		printf("The main screen cannot be configured. Exiting...\n");
		return(ERR);
	}

	ncurses_color_init(has_colors());
	curs_set(0);						/*< No cursor */

	wrefresh(mainWindows);

	getmaxyx(mainWindows, maxY, maxX);

	//alt rett, larg rett, quanto in basso, quanto a destra
	search = newwin(3, maxX, (maxY - 3), 0);
	screen = newwin((maxY - 3), maxX, 0, 0);
	if ((search == NULL) || (screen == NULL))
	{
		printf("The secondary screens cannot be initialized. Exiting...\n");
		return(ERR);
	}

	ncurses_add_intro_and_box(search, gettext("Search: "));
	ncurses_add_intro_and_box(screen, gettext("Results: "));

	results = newpad(500, (maxX - 2));
	if (results == NULL)
	{
		printf("The pad cannot be initialized.\n");
		if (errno == ENOMEM)
			strerror(errno);
		return(ERR);
	}

	cursesFlag = TRUE;	//All good. We are using the ncurses now.

	return (OK);
}


//!
//! @brief Manage the scrolling and the search input
//!
//! This function controls how the scrolling work and the
//! data insertion.
//!
//!	TODO: Manage the scrolling DOWN
//! TODO: Check result
//!
//! @param query Pointer to the query string
//!
static void ncurses_input_and_scrolling(char *query)
{
	// mousemask(ALL_MOUSE_EVENTS, NULL);	<-- IT WORKS WITHOUT THIS
	// wgetnstr(search, query, 250);		<-- If we use this we cannot scroll

	int scrollingControl = 0;	/*< Needed to check the scrolling up */
	int stringControl = 0;		/*< Segfault control */
	wchar_t singleChar;
	keypad(search, TRUE);		/*<  */

	while(TRUE)
	{
		if (stringControl == (MAX_QUERY - 1))
		{
			query[stringControl] = '\0';
			return;
		}
		noecho();
		cbreak(); //characters will be returned one at a time instead of waiting for a newline character
		singleChar = wgetch(search);
		switch(singleChar)
		{
			case KEY_PPAGE:
			case KEY_UP:
				scrollingControl--;
				if (scrollingControl < 0)
					scrollingControl = 0;
				prefresh(results,scrollingControl,0,2,2,(maxY - 5), (maxX - 2));
				break;
			case KEY_NPAGE:
			case KEY_DOWN:
				scrollingControl++;
				prefresh(results,scrollingControl,0,2,2,(maxY - 5), (maxX - 2));
				break;
			case KEY_DC:
			case KEY_LEFT:
			case KEY_RIGHT:
				break;
			case KEY_BACKSPACE:
				if (stringControl > 0)
				{ //SEGFAULT CHECK
					stringControl--;
					query[stringControl] = '\0';
					//wdelch(search); <--- Doesn't work well. So... let's ack!
					wclear(search);
					ncurses_add_intro_and_box(search,gettext("Search: "));
					wmove(search, 1, 2);
					wprintw(search,query);
				}
				break;
			default:
				echo();
				waddch(search, singleChar);
				query[stringControl] = singleChar;
				if (singleChar == '\n' || singleChar == '\0')
					return;
				stringControl++;
		}
	}
}


//!
//! @brief A glib loop acting as the main loop for the ncurses program
//!
//! The loop will keep looping until g_main_loop_quit (main_loop); is called
//! or FALSE is returned;
//!
//! @param data A gpointer passed to the function.  Currently NULL.
//!
static gboolean main_loop_function (gpointer data)
{
      wmove(search, 1, 2);
      ncurses_input_and_scrolling(query_text);
      wmove(search, 1, 2);

      ncurses_add_intro_and_box(screen, gettext("Results: "));
      wclear(results);

      //If there is nothing, exit
      if ((query_text[0] == '\n') || (query_text[0] == '\0'))
      {
        g_main_loop_quit (main_loop);
      }

      //Clear the entry area and prep for the search
      char *ptr = strchr (query_text, '\n');
      if (ptr != NULL) *ptr = '\0';

      // Run some checks and transformation on a user inputed string before using it
      char* sane_query = lw_util_prepare_query (query_text, FALSE);
      if (sane_query != NULL) {
    	  g_stpcpy(query_text, sane_query);
    	  g_free(sane_query);
    	  sane_query = NULL;
      }

      if (quiet_switch == FALSE)
        w_ncurses_start_banner (query_text, di->filename);

      if (query_text[0] == '\0')
        return TRUE;

      LwSearchItem *item = NULL;
      GError *error = NULL;
      item = lw_searchitem_new (query_text, di, GW_TARGET_CONSOLE, &error);
      if (item == NULL)
      {
        wprintw(screen, "There was an error creating your search.");
        wrefresh(screen);
        cbreak(); wgetch(search); endwin();
        query_text[0] = '\0';
        g_error_free (error);
        error = NULL;
        return TRUE;
      }

/*
      WINDOW* subWin;
      subWin = subwin(results, (maxY - 10), (maxX - 10), 2, 2);
      scrollok(subWin,1);
      touchwin(results);
      refresh();
      werase(subWin);
      results = subWin; //Fin qui funziona

      WINDOW *subbb = subpad(pad, (maxY - 10), (maxX - 10), 2, 2);
      scrollok(subbb,1);
      touchwin(pad);
      refresh();
*/

      lw_engine_get_results (item, FALSE, FALSE); //TODO: Print here?? <---

      //Print the number of results
      if (quiet_switch == FALSE)
      {
        char *message_total = ngettext("Found %d result", "Found %d results", item->total_results);
        char *message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", item->total_relevant_results);

        wclear(screen);
        ncurses_add_intro_and_box (screen, gettext("Results: "));
        wprintw(screen, message_total, item->total_results);

        if (item->total_relevant_results != item->total_results)
          wprintw(screen, message_relevant, item->total_relevant_results);

        wrefresh(screen);
        refresh();
      }

      scrollok(results,TRUE);
      prefresh(results,0,0,2,2,(maxY - 5), (maxX - 2));

      wclear(search);
      ncurses_add_intro_and_box(search,gettext("Search: "));

      if (quiet_switch == TRUE)
        wprintw(search, " QUIET ");

      lw_searchitem_free(item);
      item = NULL;

      return TRUE;
}


//!
//! @brief NCURSES Main function
//!
//! TODO: Show the chosen dictionary and search option
//! TODO: Accept a !quit/!q like vim?
//! TODO: Check result
//!
//! Zachary's recommendations:
//! * Pressing "Enter" with no text should bring up a help list of commands
//! * /exit  and /quit should exit the program
//! * /dictionary dictionaryname should switch the dictonary
//! * /exact on should turn no exact searches
//! * quiet mode doesn't really apply to ncurses so you should ignore it totally
//!
//! @param argc The number of arguments
//! @param argv An array of strings
//!
void w_ncurses_start (int argc, char *argv[])
{
    di = NULL;
    GError *error = NULL;
    GOptionContext *context;
    context = g_option_context_new (gettext("- Japanese-English dictionary program that allows regex searches"));
    GOptionEntry entries[] = 
    {
      { "quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet_switch, gettext("Display less information"), NULL },
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &dictionary_switch_data, gettext("Search using a chosen dictionary"), NULL },
      { NULL }
    };
    g_option_context_add_main_entries (context, entries, PACKAGE);
    if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      // TRANSLATORS: The "%s" stands for the error message
      g_print (gettext("Option parsing failed: %s\n"), error->message);
      exit (EXIT_FAILURE);
    }
    if (error != NULL)
    {
      printf("%s\n", error->message);
      g_error_free (error);
      error = NULL;
      exit (EXIT_FAILURE);
    }

    //We weren't interupted by any switches! Now to the search....

    //Set dictionary
    if (dictionary_switch_data == NULL)
      di = lw_dictinfolist_get_dictinfo_fuzzy ("English");
    else
      di = lw_dictinfolist_get_dictinfo_fuzzy (dictionary_switch_data);

    //Set query text
    static char* query_text_data;
    if (argc > 1 && query_text_data == NULL)
    {
      query_text_data = lw_util_get_query_from_args (argc, argv);
      if (query_text_data == NULL)
      {
        printf ("Memory error creating initial query string!\n");
        exit (EXIT_FAILURE);
      }
    }
    if (query_text_data != NULL)
    {
      strncpy (query_text, query_text_data, MAX_QUERY);
      g_free (query_text_data);
      query_text_data = NULL;
    }

    //nCurses initializations
    if (ncurses_screen_init() == ERR) return;

    //Enter the main loop of ncurses
    main_loop = g_main_loop_new (NULL, TRUE);
    g_timeout_add  (100, (GSourceFunc) main_loop_function, NULL);
    g_main_loop_run (main_loop);

    endwin();

    printf("Bye...\n");

    exit(EXIT_SUCCESS);
}


//!
//! @brief Not yet written
//!
void w_ncurses_append_edict_results_to_buffer (LwSearchItem *item)
{
		//Definitions
		int cont = 0;
		LwResultLine *resultline = item->resultline;

		wattron(results, COLOR_PAIR(1));

		//Kanji
		wprintw(results,"%s", resultline->kanji_start);
		//Furigana
		if (resultline->furigana_start)
		  wprintw(results," [%s]", resultline->furigana_start);

		wattroff(results, COLOR_PAIR(1));

		//Other info
		if (resultline->classification_start)
		  wprintw(results," %s", resultline->classification_start);
		//Important Flag
		if (resultline->important)
		  wprintw(results," %s", "P");


		wprintw(results,"\n");
		while (cont < resultline->def_total)
		{
			wattron(results, COLOR_PAIR(2));
			wprintw(results,"\t%s ", resultline->number[cont]);
			wattroff(results, COLOR_PAIR(2));
			wprintw(results,"%s\n", resultline->def_start[cont]);
			cont++;
		}
		wprintw(results,"\n");

		wrefresh(screen);
		wrefresh(results);
		refresh();
}


//!
//! @brief Not yet written
//!
void w_ncurses_append_kanjidict_results_to_buffer (LwSearchItem *item)
{
		char line_started = FALSE;
	    LwResultLine *resultline = item->resultline;

		//Kanji
		wattron(results, COLOR_PAIR(GW_NCCOLORS_GREENONBLACK));
		wprintw(results,"%s\n", resultline->kanji);
		wattroff(results, COLOR_PAIR(GW_NCCOLORS_REDONBLACK));

		if (resultline->radicals)
			wprintw(results,"%s%s\n", gettext("Radicals:"), resultline->radicals);

		if (resultline->strokes)
		{
		  line_started = TRUE;
		  wprintw(results,"%s%s", gettext("Stroke:"), resultline->strokes);
		}

		if (resultline->frequency)
		{
		  if (line_started)
			  wprintw(results," ");
		  line_started = TRUE;
		  wprintw(results,"%s%s", gettext("Freq:"), resultline->frequency);
		}

		if (resultline->grade)
		{
		  if (line_started)
			  wprintw(results," ");
		  line_started = TRUE;
		  wprintw(results,"%s%s", gettext("Grade:"), resultline->grade);
		}

		if (resultline->jlpt)
		{
		  if (line_started)
			  wprintw(results," ");
		  line_started = TRUE;
		  wprintw(results,"%s%s", gettext("JLPT:"), resultline->jlpt);
		}

		if (line_started)
			wprintw(results,"\n");

		if (resultline->readings[0])
			wprintw(results,"%s%s\n", gettext("Readings:"), resultline->readings[0]);
    if (resultline->readings[1])
      wprintw(results,"%s%s\n", gettext("Name:"), resultline->readings[1]);
    if (resultline->readings[2])
      wprintw(results,"%s%s\n", gettext("Radical Name:"), resultline->readings[2]);

		if (resultline->meanings)
			wprintw(results,"%s%s\n", gettext("Meanings:"), resultline->meanings);
		wprintw(results,"\n");
}


//!
//! @brief Not yet written
//!
void w_ncurses_append_examplesdict_results_to_buffer (LwSearchItem *item)
{
		LwResultLine *resultline = item->resultline;
		int i = 0;
		while (resultline->number[i] != NULL && resultline->def_start[i] != NULL)
		{
		  if (resultline->number[i][0] == 'A' || resultline->number[i][0] == 'B')
		  {
			  wattron(results, COLOR_PAIR(GW_NCCOLORS_BLUEONBLACK));
			  wprintw(results,"%s:\t", resultline->number[i]);
			  wattroff(results, COLOR_PAIR(GW_NCCOLORS_BLUEONBLACK));
			  wprintw(results,"%s\n", resultline->def_start[i]);
		  }
		  else
			  wprintw(results,"\t%s\n", resultline->def_start[i]);
		  i++;
		}
}


//!
//! @brief Not yet written
//!
void w_ncurses_append_unknowndict_results_to_buffer (LwSearchItem *item)
{
  	wprintw(results,"%s\n", item->resultline->string);
}


//!
//! @brief Sets up the interface before each search begins
//!
//! @param item A LwSearchItem pointer to get information from
//!
void w_ncurses_pre_search_prep (LwSearchItem *item)
{
}


//!
//! @brief The details to be taken care of after a search is finished
//!
//! This is the function that takes care of things such as hiding progressbars,
//! changing action verbs to past verbs (Searching... vs Found) and for displaying
//! "no results found" pages.  Before this function is called, the searchitem search
//! status changes from GW_SEARCH_SEARCHING to GW_SEARCH_FINISHING.
//!
//! @param item A LwSearchItem pointer to get information from
//!
void w_ncurses_after_search_cleanup (LwSearchItem *item)
{
    //Finish up
    if (item->total_results == 0 &&
        item->target != GW_TARGET_KANJI &&
        item->status == GW_SEARCH_SEARCHING)
    {
      w_ncurses_no_result (item);
    }
}



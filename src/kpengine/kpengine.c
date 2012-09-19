/* KanjiPad - Japanese handwriting recognition front end
 * Copyright (C) 1997 Owen Taylor
 *
 * gWaei is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gWaei is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include "jstroke/jstroke.h"

#define MAX_STROKES 32
#define BUFLEN 1024

static char *stroke_dicts[MAX_STROKES];
static char *progname;
static char *data_file;

void
load_database()
{
  FILE *file;
  int i;

  if (data_file)
    {
      file = fopen (data_file, "rb");
    }
  else
    {
#ifdef G_OS_WIN32
      //char *dir = g_win32_get_package_installation_directory (NULL, NULL);
      char *dir = g_build_filename ("..", "share", "gwaei", NULL);
#else
      char *dir = g_strdup (KP_LIBDIR);
#endif      
      char *fname = g_build_filename (dir, "jdata.dat", NULL);
      file = fopen (fname, "rb");
      
      if (!file)
	      file = fopen ("jdata.dat", "rb");

      g_free (fname);
      g_free (dir);
    }

  if (!file)
    {
      fprintf(stderr,"%s: Can't open %s\n", progname,
	      data_file ? data_file : "jdata.dat");
      exit(1);
    }


  for (i=0;i<MAX_STROKES;i++)
    stroke_dicts[i] = NULL;

  while (1)
    {
      int n_read;
      unsigned int nstrokes;
      unsigned int len;
      int buf[2];

      n_read = fread (buf, sizeof(int), 2, file);
      
      nstrokes = GUINT32_FROM_BE(buf[0]);
      len = GUINT32_FROM_BE(buf[1]);

      if ((n_read != 2) || (nstrokes > MAX_STROKES))
	{
	  fprintf(stderr, "%s: Corrupt stroke database\n", progname);
	  exit(1);
	}

      if (nstrokes == 0)
	break;

      stroke_dicts[nstrokes] = malloc(len);
      n_read = fread(stroke_dicts[nstrokes], 1, len, file);

      if (n_read != len)
	{
	  fprintf(stderr, "%s: Corrupt stroke database", progname);
	  exit(1);
	}
    }
  
  fclose (file);
}

/* From Ken Lunde's _Understanding Japanese Information Processing_
   O'Reilly, 1993 */

void 
sjis2jis(unsigned char *p1, unsigned char *p2)
{
  unsigned char c1 = *p1;
  unsigned char c2 = *p2;
  int adjust = c2 < 159;
  int rowOffset = c1 < 160 ? 112 : 176;
  int cellOffset = adjust ? (c2 > 127 ? 32 : 31) : 126;
  
  *p1 = ((c1 - rowOffset) << 1) - adjust;
  *p2 -= cellOffset;  
}

int
process_strokes (FILE *file)
{
  RawStroke strokes[MAX_STROKES];
  char *buffer = malloc(BUFLEN);
  int buflen = BUFLEN;
  int nstrokes = 0;

  /* Read in strokes from standard in, all points for each stroke
   *  strung together on one line, until we get a blank line
   */
  
  while (1)
    {
      char *p,*q;
      int len;

      if (!fgets(buffer, buflen, file))
	return 0;

      while ((strlen(buffer) == buflen - 1) && (buffer[buflen-2] != '\n'))
	{
	  buflen += BUFLEN;
	  buffer = realloc(buffer, buflen);
	  if (!fgets(buffer+buflen-BUFLEN-1, BUFLEN+1, file))
	    return 0;
	}
      
      len = 0;
      p = buffer;
      
      while (1) {
	while (isspace (*p)) p++;
	if (*p == 0)
	  break;
	strokes[nstrokes].m_x[len] = strtol (p, &q, 0);
	if (p == q)
	  break;
	p = q;
	  
	while (isspace (*p)) p++;
	if (*p == 0)
	  break;
	strokes[nstrokes].m_y[len] = strtol (p, &q, 0);
	if (p == q)
	  break;
	p = q;
	
	len++;
      }
      
      if (len == 0)
	break;
      
      strokes[nstrokes].m_len = len;
      nstrokes++;
      if (nstrokes == MAX_STROKES)
	break;
    }
  
  if (nstrokes != 0 && stroke_dicts[nstrokes])
    {
      int i;
      ListMem *top_picks;
      StrokeScorer *scorer = StrokeScorerCreate (stroke_dicts[nstrokes],
						 strokes, nstrokes);
      if (scorer)
	{
	  StrokeScorerProcess(scorer, -1);
	  top_picks = StrokeScorerTopPicks(scorer);
	  StrokeScorerDestroy(scorer);
	  
	  printf("K");
	  for (i=0;i<top_picks->m_argc;i++)
	    {
	      unsigned char c[2];
	      if (i)
		printf(" ");
	      c[0] = top_picks->m_argv[i][0];
	      c[1] = top_picks->m_argv[i][1];
	      sjis2jis(&c[0],&c[1]);
	      printf("%2x%2x",c[0],c[1]);
	    }
	  
	  free(top_picks);
	}
      printf("\n");

      fflush(stdout);
    }
  return 1;
}

void
usage ()
{
  fprintf(stderr, "Usage: %s [-f/--data-file FILE]\n", progname);
  exit (1);
}

int 
real_main(int argc, char **argv)
{
  int i;
  char *p = progname = argv[0];
  while (*p)
    {
      if (*p == '/') progname = p+1;
      p++;
    }

  for (i=1; i<argc; i++)
    {
      if (!strcmp(argv[i], "--data-file") ||
	  !strcmp(argv[i], "-f"))
	{
	  i++;
	  if (i < argc)
	    data_file = argv[i];
	  else
	    usage();
	}
      else
	{
	  usage();
	}
    }
  
  load_database();

  while (process_strokes (stdin))
    ;

  return 0;
}

#ifdef G_OS_WIN32
#include <windows.h>

/* To avoid a console window coming up while running our
 * program, on Win32, we act as a GUI app... a WinMain()
 * and no main().
 */
#ifdef __GNUC__
#  ifndef _stdcall
#    define _stdcall  __attribute__((stdcall))
#  endif
#endif

int _stdcall
WinMain (struct HINSTANCE__ *hInstance,
	 struct HINSTANCE__ *hPrevInstance,
	 char               *lpszCmdLine,
	 int                 nCmdShow)
{
  return real_main (__argc, __argv);
}
#else
int 
main(int argc, char **argv)
{
  return real_main (argc, argv);
}
#endif

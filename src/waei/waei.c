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
//! @file waei.c
//!
//! @brief Main entrance into the program.
//!
//! Main entrance into the program.
//!

#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <glib.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <waei/gettext.h>
#include <waei/waei.h>


int 
main (int argc, char *argv[])
{
    GObject *application;
    int resolution;

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, GWAEI_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    g_type_init ();

    application = w_application_new ();
    resolution = w_application_run (W_APPLICATION (application), &argc, &argv);

    g_object_unref (application);
    application = NULL;

    return resolution;
}



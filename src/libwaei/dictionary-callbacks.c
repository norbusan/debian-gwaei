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
//!  @file dictionary-callbacks.c
//!
//!  @brief LwDictionary objects represent a loaded dictionary that the program
//!         can use to carry out searches.  You can uninstall dictionaries
//!         by using the object, but you cannot install them. LwDictInst
//!         objects exist for that purpose.
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>

#include <libwaei/dictionary-private.h>

//!
//! @brief A callback that updates the LwInstallDictionary source uri when the pref changes
//! @param setting A GSetting object
//! @param KEY The key of the pref
//! @param data User data passed to the preference listener
//!
void 
lw_dictionary_sync_downloadlist_cb (GSettings *settings, gchar* key, gpointer data)
{
    //Declarations
    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;
    gchar downloads[200];

    //Initialiations
    dictionary = LW_DICTIONARY (data);
    priv = dictionary->priv;
    install = priv->install;
    lw_preferences_get_string (downloads, settings, key, 200);

    if (install->downloads != NULL) g_free (install->downloads); install->downloads = NULL;
    install->downloads = g_strdup (downloads);
}


gint
lw_dictionary_sync_progress_cb (gdouble fraction, gpointer data)
{
    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;
    LwDictionaryClass *klass;
    
    dictionary = LW_DICTIONARY (data);
    priv = dictionary->priv;
    klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));

    if (priv->progress != fraction)
    {
      priv->progress = fraction;
      g_signal_emit (G_OBJECT (dictionary), klass->signalid[LW_DICTIONARY_CLASS_SIGNALID_PROGRESS_CHANGED], 0);    
    }

    return 0;
}


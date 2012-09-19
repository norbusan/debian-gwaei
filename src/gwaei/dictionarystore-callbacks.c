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
//! @file dictionarystore-callbacks.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/dictionarystore-private.h>


G_MODULE_EXPORT void gw_dictionarystore_row_changed_cb (GtkTreeModel *model,
                                                        GtkTreePath  *path,
                                                        gpointer      data)
{
    //Declarations
    GwDictionaryStore *store;

    //Initializations
    store = GW_DICTIONARYSTORE (model);

    gw_dictionarystore_normalize (store);
    gw_dictionarystore_update (store);
}





/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Pauli Virtanen. All Rights Reserved.

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
//! @file morphology.c
//!


#include "../private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>


static LwMorphologyItem *lw_morphologyitem_new ();
static void lw_morphologyitem_free (LwMorphologyItem *item);

static LwMorphologyEngine *_engine = NULL;

//!
//! @brief Analyses input 
//! @param INPUT The input to study
//! @return Returns an allocated LwMorphology object that should be freed with lw_morphology_new_free or NULL on error
//!
LwMorphology* 
lw_morphology_new ()
{
    LwMorphology *result;

    result = g_new0 (LwMorphology, 1);

    return result;
}


//!
//! @brief Frees an allocated LwMorphology object. 
//! @param item The object to free
//!
void 
lw_morphology_free (LwMorphology *item)
{
    if (item->items)
      g_list_free_full (item->items, (GDestroyNotify)lw_morphologyitem_free);
    free(item);
}

//!
//! @brief Allocates a new empty LwMorphologyItem object.
//!
static LwMorphologyItem*
lw_morphologyitem_new ()
{
    LwMorphologyItem *item;
    item = g_new0 (LwMorphologyItem, 1);
    return item;
}

//!
//! @brief Frees an allocated LwMorphologyItem object.
//!
static void 
lw_morphologyitem_free (LwMorphologyItem *item)
{
    if (item->word != NULL)
      g_free (item->word); item->word = NULL;
    if (item->base_form != NULL)
      g_free (item->base_form); item->base_form = NULL;
    if (item->explanation != NULL)
      g_free (item->explanation); item->explanation = NULL;
    free(item);
}


//
// Morphological analysis using the Mecab engine
//

// Keywords used by Mecab dictionaries (ID numbers vary between different
// dictionaries, and cannot be relied on, so just compare UTF-8 strings)
#define ID_VERB          "動詞"
#define ID_NOUN          "名詞"
#define ID_SUFFIX        "接尾辞"
#define ID_POSTPOSITION  "助詞"
#define ID_AUX_VERB      "助動詞"
#define ID_NON_INDEPENDENT   "非自立"

#define PLAIN_COPULA "だ"

//!
//! @brief Initializes the Mecab analysis engine.
//!
LwMorphologyEngine*
lw_morphologyengine_new ()
{
    //Declarations
    static gchar *argv[] = {"mecab", NULL};
    LwMorphologyEngine *engine;

    //Initializations
    engine = g_new0 (LwMorphologyEngine, 1);
    if (engine != NULL)
    {
      engine->mecab = mecab_new (sizeof(argv)/sizeof(gchar*)-1, argv);
      g_mutex_init (&engine->mutex);
    }

    //Error checking
    if (engine->mecab == NULL) {
/*
mecab_strerr CAUSES A SEGFAULT
      if (engine->mecab == NULL) 
        g_warning ("Failed to initialize Mecab engine: %s", mecab_strerror (NULL));
*/
      lw_morphologyengine_free (engine); engine = NULL;
    }
    g_return_val_if_fail (engine != NULL, NULL);

    return engine;
}


LwMorphologyEngine*
lw_morphologyengine_get_default ()
{
  if (_engine == NULL)
  {
    _engine = lw_morphologyengine_new ();
  }
  return _engine;
}

gboolean
lw_morphologyengine_has_default ()
{
  return (_engine != NULL);
}


void
lw_morphologyengine_free (LwMorphologyEngine *engine)
{
    g_return_if_fail (engine != NULL);

    if (engine != NULL)
    {
      if (engine == _engine) _engine = NULL;
      if (engine->mecab != NULL) mecab_destroy (engine->mecab);
      g_mutex_clear (&engine->mutex);
    }
    g_free (engine);
}


//!
//! @brief Convert string from UTF-8 to Mecab's charset.
//!
gchar*
lw_morphologyengine_encode_to_mecab (LwMorphologyEngine *engine, const gchar *WORD, gint nbytes)
{
    const mecab_dictionary_info_t *info = mecab_dictionary_info (engine->mecab);
    gsize bytes_read, bytes_written;
    return g_convert (WORD, nbytes, info->charset, "UTF-8", &bytes_read, &bytes_written, NULL);
}

//!
//! @brief Convert string from Mecab's charset to UTF-8.
//!
gchar*
lw_morphologyengine_decode_from_mecab (LwMorphologyEngine *engine, const gchar *word, gint nbytes)
{
    const mecab_dictionary_info_t *info = mecab_dictionary_info (engine->mecab);
    gsize bytes_read, bytes_written;
    return g_convert (word, nbytes, "UTF-8", info->charset, &bytes_read, &bytes_written, NULL);
}


//!
//! @brief Morphological analysis of input using Mecab
//!
void
lw_morphology_analize (LwMorphologyEngine *engine, LwMorphology *result, const gchar *INPUT_RAW)
{
    if (engine == NULL) return;

    g_return_if_fail (result != NULL && result->items == NULL);

    const mecab_node_t *node;
    gchar **fields = NULL, *surface = NULL;
    gchar *temp;
    gchar *input = NULL;
    LwMorphologyItem *item = NULL;

    g_mutex_lock (&engine->mutex);

    input = lw_morphologyengine_encode_to_mecab (engine, INPUT_RAW, -1);
    if (!input)
      goto fail;
    node = mecab_sparse_tonode(engine->mecab, input);

#define FLUSH_ITEM                                                                            \
        do {                                                                                  \
            if (item->explanation && !g_str_has_prefix(item->explanation, item->base_form)) { \
                temp = g_strdup_printf("%s(%s)", item->explanation, item->base_form);         \
                g_free(item->explanation);                                                    \
                item->explanation = temp;                                                     \
            }                                                                                 \
            result->items = g_list_prepend(result->items, item);                              \
            item = NULL;                                                                      \
        } while (0)

    for (; node; node = node->next) {
      gchar *base_form, *word_class;
      gchar *p;
      gboolean start_word = FALSE;

      if (node->stat != MECAB_NOR_NODE) {
        continue;
      }

      // Parse input
      p = lw_morphologyengine_decode_from_mecab (engine, node->feature, -1);
      if (!p)
        goto fail;
      fields = g_strsplit(p, ",", -1);
      g_free(p);

      if (g_strv_length(fields) < 7) {
        goto fail;
      }

      surface = lw_morphologyengine_decode_from_mecab (engine, node->surface, node->length);
      word_class = fields[0];
      base_form = fields[6];

      p = g_strrstr(base_form, ":");
      if (p) {
        base_form = p+1;
      }
      if (g_str_has_suffix(base_form, PLAIN_COPULA) &&
        !g_str_has_suffix(surface, PLAIN_COPULA)) {
        // Mecab may add the copula to adjectives -- strip it
        base_form[strlen(base_form) - 3] = '\0';
      }

      // Check whether to start a new word here
      start_word = TRUE;
      if (g_str_has_prefix(word_class, ID_SUFFIX) ||
            g_str_has_prefix(word_class, ID_AUX_VERB)) {
        // Skip suffixes & aux. verbs
        start_word = FALSE;
      }
      if (g_str_has_prefix(word_class, ID_POSTPOSITION) &&
            g_utf8_strlen(surface, -1) == 1) {
        // Skip single-letter postpositions (NO, TE, etc.)
        start_word = FALSE;
      }
      if (base_form[0] == '*' || base_form[0] == '\0') {
        // Start a word only if there is some base form listed
        start_word = FALSE;
      }

      // Process input
      if (start_word) {
          if (item) {
            FLUSH_ITEM;
          }
        item = lw_morphologyitem_new();
        item->word = g_strdup (surface);
        item->base_form = g_strdup (base_form);
        item->explanation = NULL;
      }
      else {
        if (item) {
          temp = g_strconcat (item->word, surface, NULL);
          g_free (item->word);
          item->word = temp;
        }
      }

      // Construct explanation
      if (item) {
        if (item->explanation == NULL) {
          item->explanation = g_strdup (surface);
        }
        else {
          temp = g_strdup_printf ("%s-%s", item->explanation, surface);
          g_free (item->explanation);
          item->explanation = temp;
        }
      }

      g_strfreev (fields);
      g_free (surface);
      fields = NULL;
      surface = NULL;
    }

    if (item) {
      FLUSH_ITEM;
    }

    g_free(input);

    g_mutex_unlock (&engine->mutex);

    result->items = g_list_reverse(result->items);

    return;

fail:
    g_mutex_unlock (&engine->mutex);

    if (item != NULL) lw_morphologyitem_free (item);
    if (fields != NULL) g_strfreev (fields);
    if (surface != NULL) g_free (surface);
    if (input != NULL) g_free (input);

    return;
}


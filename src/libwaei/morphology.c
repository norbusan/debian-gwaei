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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include "config.h"

static LwMorphology *lw_morphology_new ();
static void lw_morphology_free (LwMorphology *morphology);

static LwMorphologyEngine *_engine = NULL;


//!
//! @brief Frees an allocated LwMorphology object. 
//! @param morphology The object to free
//!
void 
lw_morphologylist_free (GList *list)
{
    if (list == NULL) return;
    g_list_free_full (list, (GDestroyNotify)lw_morphology_free);
}

//!
//! @brief Allocates a new empty LwMorphology object.
//!
static LwMorphology*
lw_morphology_new ()
{
    LwMorphology *morphology;
    morphology = g_new0 (LwMorphology, 1);
    return morphology;
}

//!
//! @brief Frees an allocated LwMorphology object.
//!
static void 
lw_morphology_free (LwMorphology *morphology)
{
    if (morphology->word != NULL)
      g_free (morphology->word); morphology->word = NULL;
    if (morphology->base_form != NULL)
      g_free (morphology->base_form); morphology->base_form = NULL;
    if (morphology->explanation != NULL)
      g_free (morphology->explanation); morphology->explanation = NULL;
    free(morphology);
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
      g_message ("You may not have any mecab dictionaries installed... (Try installing mecab-ipadic?)");
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
static gchar*
lw_morphologyengine_encode_to_mecab (LwMorphologyEngine *engine, const gchar *WORD, gint nbytes)
{
    const mecab_dictionary_info_t *info = mecab_dictionary_info (engine->mecab);
    gsize bytes_read, bytes_written;
    return g_convert (WORD, nbytes, info->charset, "UTF-8", &bytes_read, &bytes_written, NULL);
}

//!
//! @brief Convert string from Mecab's charset to UTF-8.
//!
static gchar*
lw_morphologyengine_decode_from_mecab (LwMorphologyEngine *engine, const gchar *word, gint nbytes)
{
    const mecab_dictionary_info_t *info = mecab_dictionary_info (engine->mecab);
    gsize bytes_read, bytes_written;
    return g_convert (word, nbytes, "UTF-8", info->charset, &bytes_read, &bytes_written, NULL);
}


//!
//! @brief Morphological analysis of input using Mecab
//!
GList*
lw_morphologyengine_analyze (LwMorphologyEngine *engine, const gchar *INPUT_RAW)
{
    if (engine == NULL) return NULL;
    g_return_val_if_fail (INPUT_RAW != NULL, NULL);

    const mecab_node_t *node;
    gchar **fields = NULL, *surface = NULL;
    gchar *temp;
    gchar *input = NULL;
    LwMorphology *morphology = NULL;
    GList *list = NULL;

    g_mutex_lock (&engine->mutex);

    input = lw_morphologyengine_encode_to_mecab (engine, INPUT_RAW, -1);
    if (!input)
      goto fail;
    node = mecab_sparse_tonode(engine->mecab, input);

#define FLUSH_ITEM                                                                            \
        do {                                                                                  \
            if (morphology->explanation && !g_str_has_prefix(morphology->explanation, morphology->base_form)) { \
                temp = g_strdup_printf("%s(%s)", morphology->explanation, morphology->base_form);         \
                g_free(morphology->explanation);                                                    \
                morphology->explanation = temp;                                                     \
            }                                                                                 \
            list = g_list_prepend(list, morphology);                              \
            morphology = NULL;                                                                      \
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
          if (morphology) {
            FLUSH_ITEM;
          }
        morphology = lw_morphology_new ();
        morphology->word = g_strdup (surface);
        morphology->base_form = g_strdup (base_form);
        morphology->explanation = NULL;
      }
      else {
        if (morphology) {
          temp = g_strconcat (morphology->word, surface, NULL);
          g_free (morphology->word);
          morphology->word = temp;
        }
      }

      // Construct explanation
      if (morphology) {
        if (morphology->explanation == NULL) {
          morphology->explanation = g_strdup (surface);
        }
        else {
          temp = g_strdup_printf ("%s-%s", morphology->explanation, surface);
          g_free (morphology->explanation);
          morphology->explanation = temp;
        }
      }

      g_strfreev (fields);
      g_free (surface);
      fields = NULL;
      surface = NULL;
    }

    if (morphology) {
      FLUSH_ITEM;
    }

    g_free(input);

    g_mutex_unlock (&engine->mutex);

    list = g_list_reverse(list);

    return list;

fail:
    g_mutex_unlock (&engine->mutex);

    if (morphology != NULL) lw_morphology_free (morphology);
    if (fields != NULL) g_strfreev (fields);
    if (surface != NULL) g_free (surface);
    if (input != NULL) g_free (input);

    return list;
}



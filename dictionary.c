#include "dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** Maximum value size for integers and doubles. */
#define MAXVALSZ    1024

/** Minimal allocated number of entries in a dictionary */
#define DICTMINSZ   128

/** Invalid key token */
#define DICT_INVALID_KEY   ((char*)-1)


static char * xstrdup(const char * s)
{
    char * t ;
    size_t len ;
    if (!s)
        return NULL;
    len = strlen(s) + 1;
    t = (char*) malloc(len);
    if (t)
        memcpy(t, s, len);
    return t;
}

static int DictionaryGrow(dictionary *d)
{
    char        ** new_val;
    char        ** new_key;
    unsigned     * new_hash;

    new_val  = (char**) calloc(d->size * 2, sizeof *d->val);
    new_key  = (char**) calloc(d->size * 2, sizeof *d->key);
    new_hash = (unsigned*) calloc(d->size * 2, sizeof *d->hash);
    if (!new_val || !new_key || !new_hash) 
    {
        /* An allocation failed, leave the dictionary unchanged */
        if (new_val)
            free(new_val);
        if (new_key)
            free(new_key);
        if (new_hash)
            free(new_hash);
        return -1 ;
    }
    /* Initialize the newly allocated space */
    memcpy(new_val, d->val, d->size * sizeof(char *));
    memcpy(new_key, d->key, d->size * sizeof(char *));
    memcpy(new_hash, d->hash, d->size * sizeof(unsigned));
    /* Delete previous data */
    free(d->val);
    free(d->key);
    free(d->hash);
    /* Actually update the dictionary */
    d->size *= 2 ;
    d->val = new_val;
    d->key = new_key;
    d->hash = new_hash;
    return 0 ;
}

/*---------------------------------------------------------------------------
                            Function codes
 ---------------------------------------------------------------------------*/
unsigned DictionaryHash(const char * key)
{
    size_t len;
    unsigned hash;
    size_t i;

    if (!key)
        return 0 ;

    len = strlen(key);
    for (hash = 0, i = 0; i < len; i++) {
        hash += (unsigned)key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


dictionary* DictionaryNew(size_t size)
{
    dictionary *d;

    if (size < DICTMINSZ)
        size = DICTMINSZ;
    d = (dictionary*)calloc(1, sizeof *d);
    if (d) 
    {
        d->size = size ;
        d->val = (char**)calloc(size, sizeof *d->val);
        d->key = (char**)calloc(size, sizeof *d->key);
        d->hash = (unsigned*)calloc(size, sizeof *d->hash);
    }
    return d;
}

void DictionaryDel(dictionary *d)
{
    ssize_t i;

    if (NULL == d) 
        return;
    for (i = 0; i < d->size; i++)
    {
        if (NULL != d->key[i])
            free(d->key[i]);
        if (NULL != d->val[i])
            free(d->val[i]);
    }
    free(d->val);
    free(d->key);
    free(d->hash);
    free(d);
    return;
}

const char* DictionaryGetValue(const dictionary *d, const char *key, const char *def)
{
    unsigned hash;
    ssize_t i;

    hash = DictionaryHash(key);
    for (i = 0; i < d->size; i++)
    {
        if (NULL == d->key[i])
            continue;
        if (hash == d->hash[i]) 
        {
            if (!strcmp(key, d->key[i]))
                return d->val[i];
        }
    }
    return def;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Set a value in a dictionary.
  @param    d       dictionary object to modify.
  @param    key     Key to modify or add.
  @param    val     Value to add.
  @return   int     0 if Ok, anything else otherwise
  If the given key is found in the dictionary, the associated value is
  replaced by the provided one. If the key cannot be found in the
  dictionary, it is added to it.
  It is Ok to provide a NULL value for val, but NULL values for the dictionary
  or the key are considered as errors: the function will return immediately
  in such a case.
  Notice that if you dictionary_set a variable to NULL, a call to
  dictionary_get will return a NULL value: the variable will be found, and
  its value (NULL) is returned. In other words, setting the variable
  content to NULL is equivalent to deleting the variable from the
  dictionary. It is not possible (in this implementation) to have a key in
  the dictionary without value.
  This function returns non-zero in case of failure.
 */
/*--------------------------------------------------------------------------*/
int DictionarySetValue(dictionary *d, const char *key, const char *val)
{
    ssize_t i;
    unsigned hash;

    if (NULL == d || NULL == key) 
        return -1;
    /* Compute hash for this key */
    hash = DictionaryHash(key) ;
    /* Find if value is already in dictionary */
    if (d->n > 0) 
    {
        for (i = 0; i < d->size; i++)
        {
            if (NULL == d->key[i])
                continue;
            if (hash == d->hash[i]) { /* Same hash value */
                if (!strcmp(key, d->key[i]))
                {   /* Same key */
                    /* Found a value: modify and return */
                    if (NULL != d->val[i])
                        free(d->val[i]);
                    d->val[i] = (val ? xstrdup(val) : NULL);
                    /* Value has been modified: return */
                    return 0;
                }
            }
        }
    }
    /* Add a new value */
    /* See if dictionary needs to grow */
    if (d->n == d->size) {
        /* Reached maximum size: reallocate dictionary */
        if (dictionary_grow(d) != 0)
            return -1;
    }

    /* Insert key in the first empty slot. Start at d->n and wrap at
       d->size. Because d->n < d->size this will necessarily
       terminate. */
    for (i=d->n ; d->key[i] ; ) {
        if(++i == d->size) i = 0;
    }
    /* Copy key */
    d->key[i]  = xstrdup(key);
    d->val[i]  = (val ? xstrdup(val) : NULL) ;
    d->hash[i] = hash;
    d->n ++ ;
    return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete a key in a dictionary
  @param    d       dictionary object to modify.
  @param    key     Key to remove.
  @return   void
  This function deletes a key in a dictionary. Nothing is done if the
  key cannot be found.
 */
/*--------------------------------------------------------------------------*/
void DictionaryDelKey(dictionary *d, const char *key)
{
    unsigned hash;
    ssize_t i;

    if (key == NULL || d == NULL)
        return;

    hash = dictionary_hash(key);
    for (i = 0; i < d->size; i++) {
        if (NULL == d->key[i])
            continue ;
        /* Compare hash */
        if (hash == d->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(key, d->key[i]))
                /* Found key */
                break;
        }
    }
    if (i >= d->size)
        /* Key not found */
        return;
    free(d->key[i]);
    d->key[i] = NULL;
    if (d->val[i]!=NULL)
    {
        free(d->val[i]);
        d->val[i] = NULL;
    }
    d->hash[i] = 0;
    d->n --;
    return;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a dictionary to an opened file pointer.
  @param    d   Dictionary to dump
  @param    f   Opened file pointer.
  @return   void
  Dumps a dictionary onto an opened file pointer. Key pairs are printed out
  as @c [Key]=[Value], one per line. It is Ok to provide stdout or stderr as
  output file pointers.
 */
/*--------------------------------------------------------------------------*/
void DictionaryDump(const dictionary *d, FILE *out)
{
    ssize_t i;

    if (NULL == d || NULL == out)
        return;
    if (d->n < 1)
    {
        fprintf(out, "empty dictionary\n");
        return;
    }
    for (i = 0; i < d->size; i++)
    {
        if(d->key[i])
            fprintf(out, "%20s\t[%s]\n", d->key[i],d->val[i] ? d->val[i] : "UNDEF");
    }
    return ;
}
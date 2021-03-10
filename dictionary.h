#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _dictionary_ {
    int n ;     /** Number of entries in dictionary */
    ssize_t size;  /** Storage size */
    char **val;   /** List of string values */
    char **key;   /** List of string keys */
    unsigned *hash;  /** List of hash values for keys */
} dictionary;


unsigned DictionaryHash(const char *key);

dictionary* DictionaryNew(size_t size);

void DictionaryDel(dictionary *vd);

const char* DictionaryGetValue(const dictionary *d, const char *key, const char *def);

int DictionarySetValue(dictionary *vd, const char *key, const char *val);

void DictionaryDelKey(dictionary *d, const char *key);

void DictionaryDump(const dictionary *d, FILE *out);

#ifdef __cplusplus
}
#endif

#endif
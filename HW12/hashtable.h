#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>
struct _element_hashtable {
    char *key;
    size_t value;
}; //__attribute__((packed))
typedef struct _element_hashtable element_hashtable;

struct _hashtable {
    element_hashtable *element;
    size_t size;
}; //__attribute__((packed))
typedef struct _hashtable hashtable;

hashtable *hashtable_create(size_t size);
hashtable *hashtable_insert(hashtable *table, const char *key, size_t value);
int hashtable_checkkey(hashtable *table, const char *key);
size_t hashtable_get(hashtable *table, const char *key);
int hashtable_modify(hashtable *table, const char *key, size_t new_value);
void filltable(hashtable **table, const char *key, size_t inc_val);
void hashtable_merge(hashtable **outtable, hashtable *inputtable);
void hashtable_free(hashtable *table);

#endif


#include "hashtable.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <bsd/string.h>

static long long hash(const char *key, size_t size) {
  if (key == NULL) {
    return 0;
  }
  const int p = 53;
  long long hash = 0, p_pow = 1;
  for (size_t i = 0; i < size; ++i) {
    hash += (key[i] - '!' + 1) * p_pow;
    p_pow *= p;
  }
  return hash;
}

hashtable *hashtable_create(size_t size) {
  hashtable *table = malloc(sizeof(hashtable));
  if (table == NULL) {
    return NULL;
  }
  table->size = size;
  table->element = calloc(sizeof(element_hashtable), table->size);
  if (table->element == NULL) {
    free(table);
    return NULL;
  }
  return table;
}

static hashtable *hashtable_expand(hashtable *table) {
  // The new table is twice as big
  hashtable *newt = hashtable_create(table->size * 2);
  // rehash all keys
  for (size_t i = 0; i < table->size; ++i) {
    if (table->element[i].key != NULL) {
      hashtable_insert(newt, table->element[i].key, table->element[i].value);
    }
  }
  hashtable_free(table);
  return newt;
}

hashtable *hashtable_insert(hashtable *table, const char *key, int value) {
  if (table == NULL) {
    printf("table is NULL!\n");
    return NULL;
  }
  if (key == NULL) {
    printf("key is NULL!\n");
    return table;
  }

  size_t indx = hash(key, strlen(key)) % table->size;
  // printf("start index for %s is %ld\n", key, indx);
  while (table->element[indx].key != NULL) {
    indx += 1;
    if (indx == table->size) {
      // expant hashtable and add new
      return (hashtable_insert(hashtable_expand(table), key, value));
    }
  }
  table->element[indx].key = calloc(strlen(key) + 1, sizeof(char));
  if (table->element[indx].key == NULL) {
    printf("Problem with calloc for key\n");
    hashtable_free(table);
    return NULL;
  }
  table->element[indx].value = value;
  size_t key_length = strlen(key);
  strlcpy(table->element[indx].key, key, key_length+1);
  return table;
}

// if key exist return 1 else return 0
int hashtable_checkkey(hashtable *table, const char *key) {
  if (table == NULL) {
    return 0;
  }
  if (key == NULL) {
    return 0;
  }
  for (size_t indx = hash(key, strlen(key)) % table->size; indx < table->size; ++indx) {
    if (table->element[indx].key == NULL) {
      return 0;
    }
    if (!strcmp(table->element[indx].key, key)) {
      return 1;
    }
  }
  return 0;
}

// get value by key
int hashtable_get(hashtable *table, const char *key) {
  if (!hashtable_checkkey(table, key)) {
    return 0;
  }
  for (size_t indx = hash(key, strlen(key)) % table->size; indx < table->size; ++indx) {
    if (!strcmp(table->element[indx].key, key)) {
      return table->element[indx].value;
    }
  }
  return 0;
}

// return 1 if modify else return 0
int hashtable_modify(hashtable *table, const char *key, int new_value) {
  if (!hashtable_checkkey(table, key)) {
    return 0;
  }
  for (size_t indx = hash(key, strlen(key)) % table->size; indx < table->size; ++indx) {
    if (!strcmp(table->element[indx].key, key)) {
      table->element[indx].value = new_value;
      return 1;
    }
  }
  return 0;
}

void hashtable_free(hashtable *table) {
  if (table == NULL) {
    return;
  }
  for (size_t indx = 0; indx < table->size; ++indx) {
    if (table->element[indx].key != NULL) {
      free(table->element[indx].key);
    }
  }
  free(table->element);
  free(table);
}
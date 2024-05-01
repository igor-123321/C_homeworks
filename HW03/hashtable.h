#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct _hashtable hashtable;

hashtable * hashtable_create(size_t size);
hashtable * hashtable_insert(hashtable *table,const char * key, int value);


#endif


#ifndef HASH_TABLE_H_INCLUDED
#define HASH_TABLE_H_INCLUDED

typedef struct _HashTable HashTable;
typedef struct _HashTableData HashTableData;

typedef unsigned int (*hash_fun)(void* key);
typedef void (*hash_free_fun)(void* data);
typedef int (*hash_key_equals_fun)(void* key1, void* key2);

struct _HashTableData
{
	void** keys;
	void** values;
	unsigned int size;
	unsigned int capacity;
	hash_free_fun free_key;
	hash_free_fun free_value;
};

struct _HashTable
{
	HashTableData keyValues;
	hash_fun hash;
	hash_key_equals_fun key_equals;
};

HashTable* hash_table_new(int n, hash_fun hash);
void hash_table_free(HashTable* self);
void hash_table_set_key_equal_fun(HashTable* self, hash_key_equals_fun hash_key_equals);
void hash_table_set_free_key_fn(HashTable* self, hash_free_fun free_key);
void hash_table_set_free_value_fn(HashTable* self, hash_free_fun free_value);
void hash_table_put(HashTable* self, void* key, void* value);
void* hash_table_get(HashTable* self, void* key);
void hash_table_delete(HashTable* self, void* key);
int hash_table_contains(HashTable* self, void* key);
unsigned int hash_table_size(HashTable* self);
unsigned int hash_table_capacity(HashTable* self);

#endif /*HASH_TABLE_H_INCLUDED*/

#include "string-hashtable.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>

static unsigned int
string_hash_fun(void *key)
{
	static const int R = 31;
	char *skey = (char *)key;
	unsigned i;
	unsigned hash = 0;
	size_t l = strlen(skey);

	for (i = 0; i < l; i++)
		hash = (R * hash + skey[i]) % UINT_MAX;

	return hash;
}

static int
_str_key_eq(void *k1, void *k2)
{
	char *s1, *s2;

	if (!k1 || !k2)
		return 1;

	s1 = (char *)k1;
	s2 = (char *)k2;

	return (strcmp(s1, s2) == 0);
}

static void
free_fn(void* data)
{
	free(data);
}

HashTable*
string_hash_table_new(unsigned int capacity)
{
	HashTable* ht = hash_table_new(capacity, string_hash_fun);
	hash_table_set_key_equal_fun(ht, _str_key_eq);

	hash_table_set_free_key_fn(ht, free_fn);
	hash_table_set_free_value_fn(ht, free_fn);

	return ht;
}

#include "hashtable.h"
#include <stdlib.h>
#include <assert.h>

static inline unsigned int
_size(HashTable* self)
{
	return self->keyValues.size;
}

static inline unsigned int
_capacity(HashTable* self)
{
	return self->keyValues.capacity;
}

static inline void**
_keys(HashTable* self)
{
	return self->keyValues.keys;
}

static inline void**
_values(HashTable* self)
{
	return self->keyValues.values;
}

static inline void
_inc_size(HashTable* self)
{
	++self->keyValues.size;
}

static inline void
_dec_size(HashTable* self)
{
	--self->keyValues.size;
}

static void
hash_table_data_init(HashTableData* self, unsigned n)
{
	self->keys = (void **) calloc(n, sizeof(void *));
	self->values = (void**) calloc(n, sizeof(void *));
	self->capacity = n;
	self->size = 0;
}

static void
_free_array(void** array, unsigned n, hash_free_fun free_fn)
{
	if (free_fn) {
		unsigned i;
		for (i = 0; i < n; i++) {
			if (array[i])
				free_fn(array[i]);
		}
	}
}

static void
hash_table_data_free(HashTable* self)
{
	_free_array(self->keyValues.keys, _capacity(self), self->keyValues.free_key);
	_free_array(self->keyValues.values, _capacity(self), self->keyValues.free_value);
	free(self->keyValues.keys);
	free(self->keyValues.values);
	self->keyValues.size = self->keyValues.capacity = 0;
	self->keyValues.free_key = self->keyValues.free_value = NULL;
}

static inline int
default_hash_key_equals(void* key1, void* key2)
{
	return key1 == key2;
}

HashTable*
hash_table_new(int n, hash_fun hash)
{
	HashTable* self = NULL;

	if((n <= 0) || !hash)
		return NULL;

	self = (HashTable *) calloc(1, sizeof(HashTable));
	hash_table_data_init(&self->keyValues, n);
	self->hash = hash;
	self->key_equals = default_hash_key_equals;

	return self;
}

void
hash_table_free(HashTable* self)
{
	hash_table_data_free(self);
	free(self);
}

void
hash_table_set_free_key_fn(HashTable* self, hash_free_fun free_key)
{
	self->keyValues.free_key = free_key;
}

void
hash_table_set_free_value_fn(HashTable* self, hash_free_fun free_value)
{
	self->keyValues.free_value = free_value;
}

void
hash_table_set_key_equal_fun(HashTable* self, hash_key_equals_fun key_equals)
{
	self->key_equals = key_equals;
}

static void
resize(HashTable* self, unsigned int n) //realloc?
{
	int i;
	HashTable* ht = hash_table_new(n, self->hash);
	hash_table_set_key_equal_fun(ht, self->key_equals);
	hash_table_set_free_key_fn(ht, self->keyValues.free_key);
	hash_table_set_free_value_fn(ht, self->keyValues.free_value);

	for (i = 0; i < _capacity(self); i++)
		if (_keys(self)[i] != NULL)
			hash_table_put(ht, _keys(self)[i], _values(self)[i]);

	free(self->keyValues.keys);
	free(self->keyValues.values);

	self->keyValues.keys = _keys(ht);
	self->keyValues.values = _values(ht);
	self->keyValues.size = _size(ht);
	self->keyValues.capacity = _capacity(ht);

	free(ht);
}

static inline int
_resize_needed_put(HashTable* self)
{
	return (_size(self)) >= (_capacity(self)/2);
}

static inline int
_resize_needed_del(HashTable* self)
{
	return (_size(self) > 0 && _size(self) == _capacity(self) / 8);
}

static inline unsigned int
_size_after_resize_put(HashTable* self)
{
	return _capacity(self) * 2;
}

static inline unsigned int
_size_after_resize_del(HashTable* self)
{
	return _capacity(self) / 2;
}

unsigned int
_hash(HashTable* self, void* key)
{
	return self->hash(key) % _capacity(self);
}

void
hash_table_put(HashTable* self, void* key, void* value)
{
	unsigned int i;

	assert(value != NULL);

	if (_resize_needed_put(self))
		resize(self, _size_after_resize_put(self));

	for (i = _hash(self, key); _keys(self)[i] != NULL; i = (i+1) % _capacity(self)) {
		if (self->key_equals(_keys(self)[i], key)) {
			_values(self)[i] = value;
			return;
		}
	}

	_keys(self)[i] = key;
	_values(self)[i] = value;
	_inc_size(self);
}

void*
hash_table_get(HashTable* self, void* key)
{
	unsigned int i;

	for (i = _hash(self, key); _keys(self)[i] != NULL; i = (i+1) % _capacity(self))
		if (self->key_equals(_keys(self)[i], key))
			return _values(self)[i];

	return NULL;
}

int
hash_table_contains(HashTable* self, void* key)
{
	void* value = hash_table_get(self, key);

	return (value == NULL) ? 0 : 1;
}

unsigned int
_idx_by_key(HashTable* self, void* key)
{
	unsigned int idx = _hash(self, key);

	while (!self->key_equals(_keys(self)[idx], key))
		idx = (idx + 1) % _capacity(self);

	return idx;
}

void
_free_entry_by_idx(HashTable* self, unsigned idx)
{
	if (self->keyValues.free_key)
		self->keyValues.free_key(_keys(self)[idx]);

	if (self->keyValues.free_value)
		self->keyValues.free_value(_values(self)[idx]);

	_keys(self)[idx] = NULL;
	_values(self)[idx] = NULL;
}

void
_redo_entries_from_idx(HashTable* self, unsigned idx)
{
	while (_keys(self)[idx] != NULL) {
		void* keyToRedo = _keys(self)[idx];
		void* valueToRedo = _values(self)[idx];
		_keys(self)[idx] = NULL;
		_values(self)[idx] = NULL;
		_dec_size(self);
		hash_table_put(self, keyToRedo, valueToRedo);
		idx = (idx + 1) % _capacity(self);
	}
}

void
_remove_entry(HashTable* self, void* key)
{
	unsigned int idx = _idx_by_key(self, key);

	_free_entry_by_idx(self, idx);
	idx = (idx + 1) % _capacity(self);
	_redo_entries_from_idx(self, idx);
	_dec_size(self);
}

void
hash_table_delete(HashTable* self, void* key)
{
	if (!hash_table_contains(self, key))
		return;

	_remove_entry(self, key);

	if (_resize_needed_del(self))
		resize(self, _size_after_resize_del(self));
}

unsigned int
hash_table_size(HashTable* self)
{
	return _size(self);
}

unsigned int
hash_table_capacity(HashTable* self)
{
	return _capacity(self);
}

#include "dubai_hashtable.h"

#include "linux/slab.h"

#include "dubai_utils.h"

struct dubai_dynamic_hashtable *dubai_alloc_dynamic_hashtable(uint32_t bits)
{
	size_t size, hash_size;
	struct dubai_dynamic_hashtable *htable = NULL;

	if (bits > 20) {
		dubai_err("Too big bits: %u", bits);
		return NULL;
	}
	htable = kzalloc(sizeof(struct dubai_dynamic_hashtable), GFP_ATOMIC);
	if (!htable)
		goto out;

	htable->bits = bits;
	hash_size = BIT(bits);
	size = sizeof(struct hlist_head) * hash_size;
	htable->head = kzalloc(size, GFP_ATOMIC);
	if (!htable->head)
		goto free_htable;

	__hash_init(htable->head, hash_size);

out:
	return htable;

free_htable:
	kfree(htable);
	htable = NULL;
	goto out;
}

void dubai_free_dynamic_hashtable(struct dubai_dynamic_hashtable *htable)
{
	if (!htable)
		return;

	if (htable->head) {
		dubai_err("free head");
		kfree(htable->head);
	}
	kfree(htable);
	dubai_err("free hashtable");
}

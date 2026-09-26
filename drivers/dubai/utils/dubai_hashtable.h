#ifndef DUBAI_HASHTABLE_H
#define DUBAI_HASHTABLE_H

#include <linux/types.h>
#include <linux/hashtable.h>

#define DECLARE_DUBAI_HASHTABLE(htable, bits)		DECLARE_HASHTABLE(htable, bits)
#define dubai_hash_init(htable)					hash_init(htable)
#define dubai_hash_add(htable, node, key)			hash_add(htable, node, key)
#define dubai_hash_for_each(htable, bkt, obj, member)		hash_for_each(htable, bkt, obj, member)
#define dubai_hash_for_each_safe(htable, bkt, tmp, obj, member)	hash_for_each_safe(htable, bkt, tmp, obj, member)
#define dubai_hash_for_each_possible(htable, obj, member, key)	hash_for_each_possible(htable, obj, member, key)

#define DUBAI_DYNAMIC_HASH_BITS(htable)		((htable)->bits)
#define DUBAI_DYNAMIC_HASH_SIZE(htable)		BIT(DUBAI_DYNAMIC_HASH_BITS(htable))
#define dubai_dynamic_hash_empty(htable)	__hash_empty((htable)->head, DUBAI_DYNAMIC_HASH_SIZE(htable))
#define dubai_dynamic_hash_add(htable, node, key) \
	hlist_add_head(node, &(htable)->head[hash_min(key, DUBAI_DYNAMIC_HASH_BITS(htable))])
#define dubai_dynamic_hash_for_each(htable, bkt, obj, member) \
	for ((bkt) = 0, (obj) = NULL; (obj) == NULL && (bkt) < DUBAI_DYNAMIC_HASH_SIZE(htable); (bkt)++) \
		hlist_for_each_entry((obj), &(htable)->head[bkt], member)
#define dubai_dynamic_hash_for_each_safe(htable, bkt, tmp, obj, member) \
	for ((bkt) = 0, (obj) = NULL; (obj) == NULL && (bkt) < DUBAI_DYNAMIC_HASH_SIZE(htable); (bkt)++) \
		hlist_for_each_entry_safe((obj), (tmp), &(htable)->head[bkt], member)
#define dubai_dynamic_hash_for_each_possible(htable, obj, member, key) \
	hlist_for_each_entry((obj), &(htable)->head[hash_min(key, DUBAI_DYNAMIC_HASH_BITS(htable))], member)

struct dubai_dynamic_hashtable {
	uint32_t bits;
	struct hlist_head *head;
};

struct dubai_dynamic_hashtable *dubai_alloc_dynamic_hashtable(uint32_t bits);
void dubai_free_dynamic_hashtable(struct dubai_dynamic_hashtable *htable);

#endif // DUBAI_HASHTABLE_H

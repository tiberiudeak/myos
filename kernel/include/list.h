/*
 * circular doubly linked list implementation
 */
#ifndef LIST_H
#define LIST_H 1

#include <stddef.h>

#ifndef offsetof
#define offsetof(type, member) \
	((size_t) &(((type *) 0)->member))
#endif

/**
 * get containing structure from a given (known) member
 *
 * @param ptr		pointer to the known member
 * @param type		type of the container (struct)
 * @param member	name of the member in the structure
 */
#define get_container(ptr, type, member) \
	((type *)((void *)(ptr) - offsetof(type, member)))

/*
 * get structure this link is embedded in
 *
 * @param ptr		ptr to the embedded_link
 * @param type		type of the struct
 * @param member	name of embedded_link within the struct
 */
#define list_get_entry(ptr, type, member) \
	get_container(ptr, type, member)

/*
 * iterate through a list
 *
 * @param cursor	pointer to an embedded_link struct
 * @param list_h	list to iterate through
 */
#define list_iterate(cursor, list_h) \
	for (cursor = (list_h)->next; cursor != (list_h); cursor = cursor->next)

/*
 * iterate through a list - safe to delete elements
 *
 * @param cursor		pointer to an embedded_link struct
 * @param tmp_storage	embdedded_link ptr as temporary storage
 * @param list_h		list to iterate through
 */
#define list_iterate_safe(cursor, tmp_storage, list_h) \
	for (cursor = (list_h)->next, tmp_storage = cursor->next; \
			cursor != (list_h); \
			cursor = tmp_storage, tmp_storage = cursor->next)

/*
 * node/sentinel of a linked list
 *
 * - embed this struct in the stucture you want to use
 *   as a node in the linked list
 * - create sentinel of a list using the list_init
 *   function
 */
struct embedded_link {
	struct embedded_link *next, *prev;
};

void list_init(struct embedded_link *);
void list_add_front(struct embedded_link *, struct embedded_link *);
void list_add_end(struct embedded_link *, struct embedded_link *);
void list_add_before(struct embedded_link *, struct embedded_link *,
		struct embedded_link *);
void list_delete(struct embedded_link *, struct embedded_link *);
int list_is_empty(struct embedded_link *);

#endif /* LIST_H */


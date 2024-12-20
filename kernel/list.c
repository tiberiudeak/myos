#include <list.h>

/*
 * initialize list - create sentinel
 *
 * @param l embedded_link representing the sentinel
 */
inline void list_init(struct embedded_link *l) {
	l->next = l;
	l->prev = l;
}

/*
 * add element in the list after sentinel
 *
 * @param list_h	head of list
 * @param new_node	node to be added
 */
inline void list_add_front(struct embedded_link *list_h,
				struct embedded_link *new_node) {
	new_node->next = list_h->next;
	new_node->prev = list_h;
	list_h->next->prev = new_node;
	list_h->next = new_node;
}

/*
 * add element in the list after last element
 *
 * @param list_h	head of list
 * @param new_node	node to be added
 */
inline void list_add_end(struct embedded_link *list_h,
				struct embedded_link *new_node) {
	new_node->next = list_h;
	new_node->prev = list_h->prev;
	list_h->prev->next = new_node;
	list_h->prev = new_node;
}

/*
 * delete given node from the list
 *
 * @param list_h	head of list
 * @param node		node to be deleted
 */
void list_delete(struct embedded_link *list_h,
				struct embedded_link *node) {

	// cannot delete list head
	if (list_h == node)
		return;

	node->prev->next = node->next;
	node->next->prev = node->prev;

	node->next = NULL;
	node->prev = NULL;
}


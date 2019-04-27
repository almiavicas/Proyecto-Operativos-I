#include <stdio.h>
#include <stdlib.h>
#include "ordered_list.h"
#include "error.h"

typedef struct ordered_list list_t;
typedef struct list_iterator list_it;


list_t * new_ordered_list() {
	list_t * l = malloc(sizeof(list_t));
	if (l == NULL) {
		fprintf(stderr, "%s\n", err_mem);
		perror("Error :");
		return NULL;
	}
	l->size = 0;
	return l;
};

void * list_get(list_t * l, int key) {
	if (list_is_empty(l)) {
		fprintf(stderr, "%s\n", err_emp);
		return NULL;
	}
	struct item * it = l->begin;
	while (it != NULL) { 
		if (it->key == key) {
			item_info(it);
			return it->value;
		}
		it = it->next;
	}
	fprintf(stderr, "%s: %d\n", err_knf, key);
	return NULL;
}

int list_insert(list_t * l, int key, void * value) {
	struct item * new_item = malloc(sizeof(struct item));
	if (new_item == NULL) {
		fprintf(stderr, "%s\n", err_mem);
		return 0;
	}
	new_item->key = key;
	new_item->value = value;
	if (list_is_empty(l)) {
		l->begin = new_item;
	}
	else {
		// Check if the element is to be inserted left to the beginning
		if (key < l->begin->key) {
			new_item->next = l->begin;
			l->begin = new_item;
		}
		// Element is going to be inserted at right of the iterator
		else {
			struct item * it = l->begin;
			while (it->next != NULL && it->next->key <= key) {
				if (it->key == key) {
					fprintf(stderr, "%s\n", err_rep);
					return 0;
				}
				it = it->next;
			}
			
			if (it->next != NULL) {
				new_item->next = it->next;
			}
			it->next = new_item;
		}
	}
	l->size++;
	return 1;
}

int list_remove(list_t * l, int key) {
	if (list_is_empty(l)) {
		fprintf(stderr, "%s\n", err_emp);
		return 0;
	}
	struct item * it = l->begin;
	struct item * prev;
	while (it != NULL || it->key <= key) {
		prev = it;
		it = it->next;
	}
	if (it == NULL || it->key != key) {
		fprintf(stderr, "%s\n", err_knf);
		return 0;
	}
	prev->next = it->next;
	free(it->value);
	free(it);
	l->size--;
	return 1;
}

int list_is_empty(list_t * l) {
	return l->size == 0;
}

int list_is_begin(struct ordered_list *l, int key) {
	return l->begin->key == key;
}

int list_is_last(struct ordered_list * l, int key) {
	list_it * it = iterator(l);
	struct item * i;
	while (it_has_next(it)) {
		i = it->next;
		it_next(it);
	}
	return i->key == key;
}


list_it * iterator(list_t * l) {
	list_it * it = malloc(sizeof(list_it));
	if (it == NULL) {
		fprintf(stderr, "%s\n", err_mem);
		return NULL;
	}
	it->next = l->begin;
	return it;
}

void * it_next(list_it * it) {
	struct item * next_item = it->next;
	if (next_item == NULL) {
		fprintf(stderr, "%s\n", err_nit);
	}
	it->next = next_item->next;
	return next_item->value;
}

int it_has_next(list_it * it) {
	return it->next != NULL;
}

void item_info(struct item * it) {
	printf("Item Info:\n\tpointer: %p\n\tkey: %d\n\tvalue: %p\n\tnext: %p\n", it, it->key, it->value, it->next);
}
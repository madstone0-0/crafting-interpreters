#include "linked_list.h"

#include <stdlib.h>

LinkedList* list_new() {
    auto new_list = (LinkedList*)calloc(1, sizeof(struct LinkedList));
    new_list->size = 0;
    new_list->head = NULL;
    new_list->tail = NULL;
    return new_list;
}

void list_free(LinkedList* ll) {
    auto tmp = ll->head;
    while (tmp != NULL) {
        auto next = tmp->next;
        free(tmp);
        tmp = next;
    }
    free(ll);
}

int list_size(LinkedList* ll) { return ll->size; }

void list_add(LinkedList* ll, int val) {
    auto new_node = (ListNode*)calloc(1, sizeof(ListNode));
    new_node->val = val;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (ll->head == NULL) {
        ll->head = new_node;
        ll->tail = ll->head;
        ++ll->size;
        return;
    }

    new_node->prev = ll->tail;

    ll->tail->next = new_node;
    ll->tail = new_node;
    ++ll->size;
}

void list_insert(LinkedList* ll, int idx, int val) {
    if (idx < 0 || idx > ll->size) {
        return;
    }

    if (idx == ll->size) return list_add(ll, val);

    auto new_node = (ListNode*)calloc(1, sizeof(ListNode));
    new_node->val = val;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (ll->size == 0) {
        if (idx == 0) {
            ll->head = new_node;
            ll->tail = ll->head;
            ++ll->size;
            return;
        } else {
            return;
            ;
        }
    }

    if (idx == 0) {
        new_node->next = ll->head;
        ll->head->prev = new_node;
        ll->head = new_node;
        ++ll->size;
        return;
    }

    auto prev = list_node_at(ll, idx - 1);
    auto next = prev->next;

    prev->next = new_node;
    prev->next->prev = prev;

    prev->next->next = next;
    next->prev = prev->next;
    ++ll->size;
}

ListNode* list_node_at(LinkedList* ll, int idx) {
    if (idx < 0 || idx >= ll->size) {
        return NULL;
    }

    int i = 0;
    ListNode* tmp = ll->head;
    while (tmp != NULL && i != idx) {
        tmp = tmp->next;
        i++;
    }
    return tmp;
}

int list_at(LinkedList* ll, int idx) {
    auto node = list_node_at(ll, idx);
    if (node == NULL) {
        return -1;
    }
    return node->val;
}

int list_remove(LinkedList* ll, int idx) {
    if (ll->size == 0) {
        return -1;
    }

    if (idx < 0 || idx >= ll->size) {
        return -1;
    }

    {
        auto res = ll->head->val;

        if (idx == 0 && ll->size == 1) {
            free(ll->head);
            ll->head = NULL;
            ll->tail = NULL;
            --ll->size;
            return res;
        }

        if (idx == 0) {
            ll->head = ll->head->next;
            if (ll->head != NULL) {
                free(ll->head->prev);
                ll->head->prev = NULL;
            }
            --ll->size;
            return res;
        }
    }

    if (idx == ll->size - 1) {
        auto res = ll->tail->val;
        ll->tail = ll->tail->prev;
        if (ll->tail != NULL) {
            free(ll->tail->next);
            ll->tail->next = NULL;
        }
        --ll->size;
        return res;
    }

    auto prev = list_node_at(ll, idx - 1);
    auto res = prev->next->val;
    auto tmp = prev->next;
    prev->next = prev->next->next;
    if (prev->next != NULL) prev->next->prev = prev;
    free(tmp);
    --ll->size;
    return res;
}

ListNode* list_node_find(LinkedList* ll, int val) {
    auto tmp = ll->head;
    while (tmp != NULL) {
        if (tmp->val == val) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

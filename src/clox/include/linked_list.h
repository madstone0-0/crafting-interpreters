
#ifndef __LINKED_LIST__
#define __LINKED_LIST__

typedef struct ListNode {
    int val;
    struct ListNode* next;
    struct ListNode* prev;
} ListNode;

typedef struct LinkedList {
    int size;
    ListNode* head;
    ListNode* tail;
} LinkedList;

LinkedList* list_new();

void list_free(LinkedList* ll);

int list_size(LinkedList* ll);

void list_add(LinkedList* ll, int val);

void list_insert(LinkedList* ll, int idx, int val);

int list_remove(LinkedList* ll, int idx);

ListNode* list_node_at(LinkedList* ll, int idx);

int list_at(LinkedList* ll, int idx);

ListNode* list_node_find(LinkedList* ll, int val);

#endif  // !LINKED_LIST

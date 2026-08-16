#ifdef TYPE
#ifndef TYPED
#define TYPED(THING) THING
#endif

typedef struct TYPED(LinkedList) TYPED(LinkedList);

typedef struct TYPED(ListNode) TYPED(ListNode);

void TYPED(list_new)(void);

void TYPED(list_free)(TYPED(LinkedList) * ll);

int TYPED(list_size)(TYPED(LinkedList) * ll);

void TYPED(list_add)(TYPED(LinkedList) * ll, TYPE val);

void TYPED(list_insert)(TYPED(LinkedList) * ll, int idx, TYPE val);

TYPE TYPED(list_remove)(TYPED(LinkedList) * ll, int idx);

void TYPED(list_node_at)(TYPED(LinkedList) * ll, int idx);

TYPE TYPED(list_at)(TYPED(LinkedList) * ll, int idx);

TYPED(ListNode) * TYPED(list_node_find)(TYPED(LinkedList) * ll, int val);

#endif

#include "linkedlist.h"
#include "test.h"

// Make sure all these weird ass macros at least produce
// valid code...
LL_BASICSTRUCT(intll, int);
LL_PROTOTYPE(intll);
LL_DEFINITION(intll);

int prefree_count = 0;
void linkedlist_prefree(intll * item) {
  item = item;
  prefree_count++;
}

// Fixes bug discovered only when world was being tested: some
// fields weren't set, even though the list still worked in our tests here.
static void ll_consistency(intll * ilist) {
  intll * next = ilist;
  intll * prev = NULL;
  int i = 0;
  while(next) {
    ASSERT(next->prev == prev, "llconsistency[%d] prev", i);
    if(prev) ASSERT(prev->next == next, "llconsistency[%d] next", i);
    prev = next;
    next = next->next;
    i++;
  }
}

void linkedlist_test() {
  intll * ilist;
  intll_init(&ilist);
  ASSERT(intll_count(ilist) == 0, "init makes empty list");
  ll_consistency(ilist);
  intll_free(&ilist, NULL);
  ASSERT(intll_count(ilist) == 0, "init free still leaves empty list");
  intll_init(&ilist);
  ASSERT(intll_count(ilist) == 0, "second init after free doesn't explode");

  // Insert one item at the start, it becomes the head
  intll * inserted;
  int result = intll_create_after(&ilist, NULL, &inserted);
  ASSERT(result == 0, "create_after 1 at beginning returns no error");
  ASSERT(intll_count(ilist) == 1, "create_after 1 count");
  ll_consistency(ilist);
  // We KNOW it's at the head. this is a funny test... won't work
  // if there's a sentinel node.
  inserted->data = 55;
  ASSERT(ilist->data == 55, "create_after inserted the node at the head");

  // Insert one item after head, doesn't change head
  result = intll_create_after(&ilist, ilist, &inserted);
  ASSERT(result == 0, "create_after 2 AFTER beginning returns no error");
  ASSERT(intll_count(ilist) == 2, "create_after 2 count");
  ll_consistency(ilist);
  inserted->data = 67;
  ASSERT(ilist->data == 55, "create_after 2 inserted did not overwrite head");
  ASSERT(ilist->next->data == 67, "create_after 2 data in 2nd slot");

  // Remove the head, now head should point to second item
  intll_remove(&ilist, ilist);
  ASSERT(intll_count(ilist) == 1, "remove 1 count");
  ll_consistency(ilist);
  ASSERT(ilist->data == 67, "remove 1 head moved over");
  ASSERT(ilist->next == NULL, "single count next null");
  ASSERT(ilist->prev == NULL, "single count prev null");

  // Add one after head, same as before
  result = intll_create_after(&ilist, ilist, &inserted);
  ASSERT(result == 0, "create_after 2 AFTER remove returns no error");
  ASSERT(intll_count(ilist) == 2, "create_after 2 after remove count");
  ll_consistency(ilist);
  inserted->data = 1;

  // Add one after head, which means an insert between two elements
  result = intll_create_after(&ilist, ilist, &inserted);
  ASSERT(result == 0, "create_after 3 AFTER remove returns no error");
  ASSERT(intll_count(ilist) == 3, "create_after 3 after remove count");
  ll_consistency(ilist);
  inserted->data = 2;

  // Now make sure all the items are in the order you expect
  ASSERT(ilist->data == 67, "createafter 3 head");
  ASSERT(ilist->next->data == 2, "createafter 3 next");
  ASSERT(ilist->next->next->data == 1, "createafter 3 nextnext");

  intll_free(&ilist, linkedlist_prefree);
  ASSERT(intll_count(ilist) == 0, "free 3 count 0");
  ASSERT(prefree_count == 3, "free 3 called free 3 times");

  // Now we test insert "before" null
  result = intll_create_before(&ilist, NULL, &inserted);
  ASSERT(result == 0, "create_before 1 no error");
  ASSERT(intll_count(ilist) == 1, "create_before 1");
  ll_consistency(ilist);
  inserted->data = 40;

  // And we test insert "before" head
  result = intll_create_before(&ilist, ilist, &inserted);
  ASSERT(result == 0, "create_before 2 no error");
  ASSERT(intll_count(ilist) == 2, "create_before 2");
  ll_consistency(ilist);
  inserted->data = 50;
  ASSERT(ilist->data == 50, "create_before 2 item 1");
  ASSERT(ilist->next->data == 40, "create_before 2 item 2");

  // And we test insert "before" something in the middle
  result = intll_create_before(&ilist, ilist->next, &inserted);
  ASSERT(result == 0, "create_before 3 no error");
  ASSERT(intll_count(ilist) == 3, "create_before 3");
  ll_consistency(ilist);
  inserted->data = 60;
  ASSERT(ilist->data == 50, "create_before 3 item 1");
  ASSERT(ilist->next->data == 60, "create_before 3 item 2");
  ASSERT(ilist->next->next->data == 40, "create_before 3 item 3");

  intll_free(&ilist, NULL);
  ASSERT(1, "intll_free");
}

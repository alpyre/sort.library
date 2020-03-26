/******************************************************************************
 * Example 02_List_Basics: Sorting lists with sort.library                     *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <exec/exec.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>

#include <sort/sort.h>

//Library base and interface
struct Library *SortBase;
#ifdef __amigaos4__
  struct SortIFace *ISort;
#endif

//Utility functions
///createListFromArray(array, size)
struct List *createListFromArray(char *arr, ULONG size)
{
  struct List *list;
  struct Node *item;
  ULONG i;

  list = (struct List*) AllocMem(sizeof(struct List), MEMF_ANY);
  if (list)
  {
    NewList(list);
    for (i = 0; i < size; i++)
    {
      item = AllocMem(sizeof(struct Node), MEMF_ANY);
      if (item)
      {
        item->ln_Pri = arr[i];
        AddTail(list, item);
      }
    }
  }

  return list;
}
///
///freeList(list)
VOID freeList(struct List* list)
{
  struct Node *item;
  struct Node *next;

  for (item = (struct Node*) list->lh_Head; item->ln_Succ; item = next)
  {
    next = item->ln_Succ;

    FreeMem(item, sizeof(struct Node));
  }

  FreeMem(list, sizeof(struct List));
}
///
///printList(list)
VOID printList(struct List *list)
{
  UBYTE buffer[8];
  UBYTE result[256] = {0};
  BOOL firstItem = TRUE;
  struct Node *i;

  for (i = list->lh_Head; i->ln_Succ; i = i->ln_Succ)
  {
    sprintf(buffer, "%ld", (LONG)i->ln_Pri);
    if (firstItem) firstItem = FALSE; else strcat(result, ", ");
    strcat(result, buffer);
  }

  puts(result);
}
///

int main(int argc, char **argv)
{
  int rc = 0;

  SortBase = OpenLibrary("sort.library", 0);
  if (SortBase)
  {
  #ifdef __amigaos4__
    ISort = (struct SortIFace *)GetInterface(SortBase, "main", 1, NULL);
    if (ISort)
    {
  #endif

      BYTE arr[10] = {6, 7, 3, 2, 9, 8, 0, 1, 5, 4};
      struct List *list = createListFromArray(arr, 10);

      puts("Initial List:");
      printList(list);

      Sort(list, SORT_List, TAG_END); //Sort defaults to comparing ln_Pri of list nodes.

      puts("Sorted List:");
      printList(list);

      freeList(list);

  #ifdef __amigaos4__
      DropInterface((struct Interface *)ISort);
    }
  #endif
  }
  else puts("Could not open sort.library!");

  return(rc);
}

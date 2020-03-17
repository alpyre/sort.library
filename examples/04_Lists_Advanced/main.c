/******************************************************************************
 * Example 04_Lists_Advanced: Advanced list sorting with sort.library         *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <exec/exec.h>
#include <proto/exec.h>

#include <sort/sort.h>

//Library base and interface
struct Library *SortBase;
#ifdef __amigaos4__
  struct SortIFace *ISort;
#endif

#define MALE   0
#define FEMALE 1

struct Data
{
  STRPTR firstName;
  STRPTR sirName;
  ULONG id;
  UBYTE sex;
  UBYTE age;
};

//Custom list item structure
struct Person
{
  struct MinNode node;
  STRPTR firstName;
  STRPTR sirName;
  ULONG id;
  UBYTE sex;
  UBYTE age;
};

//Custom compare functions
LONG idCmp(APTR p1, APTR p2)
{
  struct Person *s1 = (struct Person *)p1;
  struct Person *s2 = (struct Person *)p2;

  if      (s1->id > s2->id) return  1;
  else if (s1->id < s2->id) return -1;
  else                      return  0;
}

LONG nameCmp(APTR p1, APTR p2)
{
  STRPTR n1 = ((struct Person *)p1)->firstName;
  STRPTR n2 = ((struct Person *)p2)->firstName;
  LONG cmpVal = Stricmp(n1, n2);

  if      (cmpVal > 0) return  1;
  else if (cmpVal < 0) return -1;
  else                 return  0;
}

LONG sirNameCmp(APTR p1, APTR p2)
{
  STRPTR n1 = ((struct Person *)p1)->sirName;
  STRPTR n2 = ((struct Person *)p2)->sirName;
  LONG cmpVal = Stricmp(n1, n2);

  if      (cmpVal > 0) return  1;
  else if (cmpVal < 0) return -1;
  else                 return  0;
}

//Custom hash function for Counting Sort
ULONG idHash(APTR p, ULONG digit)
{
  return ((struct Person *)p)->id;
}

//Utility functions
///createListFromArray(array, size)
struct MinList *createListFromArray(struct Data *array, ULONG size)
{
  struct MinList *list;
  struct Person *item;
  struct Data *data;
  ULONG i;

  list = (struct MinList*)AllocMem(sizeof(struct MinList), MEMF_ANY);
  if (list)
  {
    NewList((struct List*)list);
    for (i = 0; i < size; i++)
    {
      data = &array[i];
      item = AllocMem(sizeof(struct Person), MEMF_ANY);
      if (item)
      {
        item->firstName = data->firstName;
        item->sirName   = data->sirName;
        item->id        = data->id;
        item->sex       = data->sex;
        item->age       = data->age;
        AddTail((struct List*)list, (struct Node*)&item->node);
      }
    }
  }

  return list;
}
///
///freeList(list)
VOID freeList(struct MinList *list)
{
  struct Person *item;
  struct Person *next;

  for (item = (struct Person*) list->mlh_Head; item->node.mln_Succ; item = next)
  {
    next = (struct Person*)item->node.mln_Succ;

    FreeMem(item, sizeof(struct Person));
  }

  FreeMem(list, sizeof(struct MinList));
}
///
///printList(list)
VOID printList(struct MinList *list)
{
  struct Person *person;

  for (person = (struct Person *)list->mlh_Head; person->node.mln_Succ; person = (struct Person *)person->node.mln_Succ)
  {
    printf("%lu, Name: %s %s, Sex: %s, Age: %ld\n", person->id,
                                                    person->firstName,
                                                    person->sirName,
                                                    person->sex ? "Female" : "Male",
                                                    person->age);
  }
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

      #define ARRAY_SIZE 9
      struct Data array[ARRAY_SIZE] = {{"James", "Hetfield", 2, MALE,   56},
                                       {"Lars",  "Ulrich",   9, MALE,   56},
                                       {"Kirk",  "Hammet",   7, MALE,   57},
                                       {"Cliff", "Burton",   1, MALE,   24},
                                       {"Jason", "Newsted",  8, MALE,   57},
                                       {"Dave",  "Mustaine", 3, MALE,   58},
                                       {"Marty", "Friedman", 4, MALE,   57},
                                       {"Dave",  "Ellefson", 5, MALE,   55},
                                       {"Nick",  "Menza",    6, MALE,   55},
                                      };

      struct MinList *people = createListFromArray(array, ARRAY_SIZE);

      puts("Initial List:");
      printList(people);

      //Sort the people list by id using Counting Sort...
      //...which is a very fast sort algorithm that makes use of hash sets...
      //...and that's because we have to create a range structure...
      //...and a custom hash function to use it.
      {
        struct Range range;//please examine the function idHash (defined above):
        range.min = 1;     //lowest hash value for id in the data is 1
        range.max = 9;     //highest hash value for id in the data is 9
                           //because idHash() just returns the value in id
        Sort(people, SORT_List, SORT_Algorithm, SORT_Counting,
                                SORT_ItemSize, sizeof(struct Person),
                                SORT_HashFnc, idHash,
                                SORT_Range, &range,
                                TAG_END);
      }

      puts("\nSorted List by id:");
      printList(people);

      //Let's sort the array by name first and sirname second
      //To do this we initially sort by sirname with any algorithm...
      Sort(people, SORT_List, SORT_Algorithm, SORT_Auto,
                              SORT_ItemSize, sizeof(struct Person),
                              SORT_CompareFnc, sirNameCmp,
                              TAG_END);

      //...then sort by firstname with a "stable" sort algorithm
      Sort(people, SORT_List, SORT_Algorithm, SORT_Merge,
                              SORT_ItemSize, sizeof(struct Person),
                              SORT_CompareFnc, nameCmp,
                              TAG_END);

      puts("\nSorted List by name & sirname:");
      printList(people);

  #ifdef __amigaos4__
      DropInterface((struct Interface *)ISort);
    }
  #endif
  }
  else puts("Could not open sort.library!");

  return(rc);
}

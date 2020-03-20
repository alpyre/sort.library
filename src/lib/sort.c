/******************************************************************************

  This file is part of Sort Library.
  Copyright (C) 2020 Ibrahim Alper Sönmez

  Sort Library is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Sort Library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sort Library. If not, see <https://www.gnu.org/licenses/>.

 ******************************************************************************/

///includes
#ifdef __MORPHOS__
  #include <emul/emulregs.h>
#endif
#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include <SDI_lib.h>
#include "SDI_macros.h"

#include <sort/sort.h>
#include "sortbase.h"
///
///definitions
#define allocMem(a) (memoryPool ? AllocPooled(memoryPool, a) : AllocMem(a, MEMF_ANY))
#define freeMem(a, b) memoryPool ? FreePooled(memoryPool, a, b) : FreeMem(a, b)

#define index(a, b) (APTR)(((APTR)a) + ((b) * itemSize))
#define MAX_ARR(a, b) (CompareFnc(index(arr, a), index(arr, b)) ==  1 ? a : b)
#define MIN_ARR(a, b) (CompareFnc(index(arr, a), index(arr, b)) == -1 ? a : b)

#if !defined __amigaos4__
  #define NewList(list) \
  ({									\
    struct List *_NewList_list = (list);					\
    _NewList_list->lh_TailPred = (struct Node *)_NewList_list;		\
    _NewList_list->lh_Head = (struct Node *)&_NewList_list->lh_Tail;	\
    _NewList_list->lh_Tail = 0;						\
  })
#endif
///
///prototypes
VOID quickSort(APTR arr, ULONG low, ULONG high);
VOID quickSortList(struct MinList *list, struct MinNode *low, struct MinNode *high);
VOID insertSort(APTR arr, ULONG size);
VOID insertSortList(struct MinList *list);
VOID mergeSort(APTR arr, ULONG l, ULONG r);
VOID selectSort(APTR arr, ULONG size);
VOID selectSortList(struct MinList *list);
VOID shellSort(APTR arr, ULONG size);
VOID countingSort(APTR arr, ULONG size, struct Range *range, ULONG digit);
VOID radixSort(APTR arr, ULONG size, struct Range *range, ULONG digits);
VOID heapSort(APTR arr, ULONG size);
VOID bubbleSort(LONG *arr, ULONG size);
VOID bubbleSortList(struct MinList *list);

ULONG listHashI(APTR, ULONG);
ULONG byteHash(APTR, ULONG);
ULONG wordHash(APTR, ULONG);
ULONG longHash(APTR, ULONG);
ULONG listPriHash(APTR, ULONG);
ULONG longRadixHash(APTR, ULONG);
ULONG listPriRadixHash(APTR, ULONG);

LONG listCmpI(APTR, APTR);
LONG byteCompare(APTR, APTR);
LONG wordCompare(APTR, APTR);
LONG longCompare(APTR, APTR);
LONG listPriCompare(APTR, APTR);

VOID swap(APTR, APTR);
VOID swapNodes(struct MinList*, struct MinNode*, struct MinNode*);

VOID clearMem(APTR addr, ULONG size);
///
///globals
#ifdef __amigaos4__
  extern struct Library *UtilityBase;
  extern struct UtilityIFace *IUtility;
#elif __MORPHOS__
  extern struct Library *UtilityBase;
#else
  extern struct UtilityBase *UtilityBase;
#endif

ULONG (*HashFnc)(APTR, ULONG);
ULONG (*realHashFnc)(APTR, ULONG);
LONG (*CompareFnc)(APTR, APTR);
LONG (*realCmpFnc)(APTR, APTR);
APTR swapBuffer = NULL; // buffer used when swapping array elements
ULONG itemSize = 0;     // size of a single array item in bytes
ULONG sortOrder = 1;    // setting this -1 makes the sort order to be in reverse
///
///structures
// Private structures
struct Partition
{
  struct MinNode *low;
  struct MinNode *high;
};
///

/*******************************************************************************
 * Library Functions                                                           *
 ******************************************************************************/
///SortA
/****** Sort/SortA *************************************************************
*                                                                              *
*   NAME                                                                       *
*      SortA -- Sort an array or list with the algorithm of your choice.       *
*                                                                              *
*   SYNOPSIS                                                                   *
*      result = SortA(array, size, taglist);                                   *
*      D0             A0     D0    A1                                          *
*                                                                              *
*      ULONG SortA(APTR, ULONG, struct TagItem *);                             *
*                                                                              *
*      result = Sort(array, size, tag1, ...);                                  *
*                                                                              *
*      ULONG SortA(APTR, ULONG, ULONG, ...);                                   *
*                                                                              *
*   FUNCTION                                                                   *
*      Sorts an array or a double linked list (a.k.a Exec List) with one of    *
*      the available sort algorithms. Any data structure can be sorted by      *
*      providing custom compare functions (for comparison based algorithms)    *
*      or custom hash functions (for hash set based algorithms) in tags.       *
*      For arrays: it will default to sort an array of LONG integers by value. *
*      For lists: it will default to sort a full exec list by the value in the *
*      ln_Pri member of it's nodes.                                            *
*                                                                              *
*   INPUTS                                                                     *
*      array - pointer to array or list                                        *
*      size - size of the array or SORT_List to inform that the pointer given  *
*             in the first argument is a pointer to a list (struct List*)      *
*      taglist - tags to configure sort for various data structures            *
*                                                                              *
*   TAGS                                                                       *
*      SORT_Algorithm  - One of the sort algorithms available. Will default    *
*                        to SORT_Auto when ommited:                            *
*                        SORT_Auto                                             *
*                        SORT_Selection                                        *
*                        SORT_Insertion                                        *
*                        SORT_Shell                                            *
*                        SORT_Bubble                                           *
*                        SORT_Quick                                            *
*                        SORT_Merge                                            *
*                        SORT_Counting                                         *
*                        SORT_Radix                                            *
*                        SORT_Heap                                             *
*                                                                              *
*                        For more information on these algorithms refer to     *
*                        doc/ALGORITHMS.                                       *
*      SORT_ListSize   - If the length of the list to be sorted is known       *
*                        passing it with this tag will prevent a count in some *
*                        algorithms that require it (Merge, Counting, Radix).  *
*      SORT_ItemSize   - Size of each array element in bytes (not required     *
*                        for lists). Will default to sizeof(LONG) when ommited.*
*      SORT_CompareFnc - A custom function to compare two array elements (or   *
*                        list items). It should be in the form below:          *
*                        LONG myCompare(APTR p1, APTR p2);                     *
*                        p1- a void pointer to the element/item to be compared *
*                        p2- a void pointer to the other element/item to be    *
*                            compared with the first (p1)                      *
*                                                                              *
*                        The function should cast the pointers to the          *
*                        regarding data structure, access the properties to do *
*                        the comparison, compare them as expected and return:  *
*                        -1 - if the first one is lesser then the second       *
*                         0 - if the first one is equal to the second          *
*                         1 - if the first one is bigger then the second       *
*                        See example code for details.                         *
*                        Will default to compare two LONG integers.            *
*      SORT_HashFnc    - A custom hash function to be used with Counting and   *
*                        Radix Sort. It should be in the form below:           *
*                        ULONG myHash(APTR p, ULONG digit);                    *
*                        p - a void pointer to array element (or list item)    *
*                        digit - UNUSED for Counting Sort hashes. For Radix    *
*                                Sort hashes your function should return this  *
*                                digit of the value.                           *
*                        See example code for details.                         *
*                        For arrays this will default to a function that       *
*                        hashes LONG integers. For lists it will default to    *
*                        hash the ln_Pri member. For Radix Sort it will        *
*                        default to use decimal base.                          *
*      SORT_Range      - A pointer to a struct Range which provides min and    *
*                        max values the hashes will be. It determines the size *
*                        of the hash set that will be used when sorting with   *
*                        Counting or Radix Sorts.                              *
*                        Cannot be ommited when using Counting Sort.           *
*                        It will default to min -9, max 9 when using Radix.    *
*      SORT_Digits     - The number of digits in the value that has the most   *
*                        digits in the array/list. For example for an array:   *
*                        LONG arr[] = {0, -125, 12, 5, 25};                    *
*                        SORT_Digits should be set to 3 (-125 has 3 digits -   *
*                        that is when sorting in decimal radix of course).     *
*                        Will default to 3 when sorting lists (because ln_Pri  *
*                        is a BYTE value so range is in between -128, 127).    *
*                        Cannot be ommited otherwise (returns an error if so ) *
*                        Can be manipulated to use different properties of an  *
*                        element/item other than a digit of an integer value.  *
*                        See example code for details.                         *
*      SORT_Reverse    - Will sort the array in reverse order                  *
*      SORT_MemPool    - Sort will use the provided memory pool to allocate    *
*                        the extra memory required by some sort algorithms.    *
*                                                                              *
*   RESULT                                                                     *
*      result - 0: if the sort was successful                                  *
*               SORT_ERR_MEMORY: if memory is insufficient for the algorithm   *
*               SORT_ERR_RANGE: if the range (struct Range *) required by      *
*               Counting Sort algorithm is not provided in tags.               *
*               SORT_ERR_DIGITS: if the digit count required by Radix Sort     *
*               algorithm is not provided in tags.                             *
*   EXAMPLE                                                                    *
*      LONG arr[10] = {6, 10, 8, 3, 5, 1, 9, 4, 2, 7};                         *
*      Sort(arr, 10, TAG_END); // sorts with SORT_Auto                         *
*      printArray(arr, 10);                                                    *
*                                                                              *
*     >1, 2, 3, 4, 5, 6, 7, 8, 9, 10                                           *
*                                                                              *
*      -- OR --                                                                *
*                                                                              *
*      LONG arr[12] = {0, -125, 12, 5, 25, 1015, -38, -7, 4, 5, 12, -3};       *
*      struct Range range;                                                     *
*      range.min = -125;                                                       *
*      range.max = 1015;                                                       *
*      Sort(arr, 12, SORT_Algorithm, SORT_Counting,                            *
*                    SORT_Range, &range,                                       *
*                    TAG_END);                                                 *
*      printArray(arr, 12);                                                    *
*                                                                              *
*     >-125, -38, -7, -3, 0, 4, 5, 5, 12, 12, 25, 1015                         *
*                                                                              *
*   NOTES                                                                      *
*      Passing invalid size and/or range values may result in trashed memory,  *
*      crashes which may cause invalidated disk partitions (and in worst case  *
*      can even fail file systems). Use with care.                             *
*                                                                              *
*   BUGS                                                                       *
*      No known bugs yet                                                       *
*                                                                              *
*   SEE ALSO                                                                   *
*      See documents and example source code for more detailed information.    *
*                                                                              *
********************************************************************************
*
*/

LIBPROTO(SortA, ULONG, REG(a6, UNUSED __BASE_OR_IFACE), REG(a0, APTR array), REG(d0, ULONG size), REG(a1, struct TagItem * tagList))
{
  ULONG rc = 0;
  APTR  memoryPool = NULL;
  ULONG inputType;      // data structure of arr (an array or an execList)
  ULONG algorithm;      // the algorithm to be used in sorting (i.e: SORT_Quick)
  ULONG digits;         // number of digits to be sorted with radixSort
  struct Range* range;  // range of hash values for items used in countingSort
  struct TagItem *tag_i = tagList;
  struct TagItem *tag;

  struct Range decimalRadixRange;
  struct Range byteRange;
  decimalRadixRange.min = -9;
  decimalRadixRange.max = 9;
  byteRange.min = -127;
  byteRange.max =  128;

  // return if nothing to sort
  if (!array) return rc;

  // return if already sorted
  if (size < 2) return rc;

  // initialize values
  if (size == SORT_List)
    inputType = SORT_List;
  else
    inputType = SORT_Array;

  algorithm  = SORT_Auto;
  itemSize   = 0;
  CompareFnc = NULL;
  HashFnc    = NULL;
  range      = NULL;
  digits     = 0;
  sortOrder  = 1;

  // set options passed by the tagList
  while ((tag = NextTagItem(&tag_i))) {
    switch (tag->ti_Tag) {
      case SORT_Algorithm:
        switch (tag->ti_Data) {
          case SORT_Selection:
            algorithm = SORT_Selection;
          break;
          case SORT_Insertion:
            algorithm = SORT_Insertion;
          break;
          case SORT_Shell:
            algorithm = SORT_Shell;
          break;
          case SORT_Bubble:
            algorithm = SORT_Bubble;
          break;
          case SORT_Merge:
            algorithm = SORT_Merge;
          break;
          case SORT_Quick:
            algorithm = SORT_Quick;
          break;
          case SORT_Counting:
            algorithm = SORT_Counting;
          break;
          case SORT_Radix:
            algorithm = SORT_Radix;
          break;
          case SORT_Heap:
            algorithm = SORT_Heap;
          break;
        }
      break;
      case SORT_ListSize:
        size = tag->ti_Data;
      break;
      case SORT_ItemSize:
        itemSize = tag->ti_Data;
      break;
      case SORT_CompareFnc:
        CompareFnc = (LONG (*)(APTR, APTR))tag->ti_Data;
      break;
      case SORT_HashFnc:
        HashFnc = (ULONG (*)(APTR, ULONG))tag->ti_Data;
      break;
      case SORT_Range:
        range = (struct Range*)tag->ti_Data;
      break;
      case SORT_Digits:
        digits = tag->ti_Data;
      break;
      case SORT_Reverse:
        if (tag->ti_Data) sortOrder = -1;
      break;
      case SORT_MemPool:
        memoryPool = (APTR)tag->ti_Data;
      break;
    }
  }

  // NOTE: make SORT_Auto select the most efficient algorithm regarding size
  if (algorithm == SORT_Auto) {
    algorithm = SORT_Quick;
  }

  /*****************
  * DO THE SORTING *
  ******************/
  switch (inputType) {
    case SORT_List:
    if (!CompareFnc) CompareFnc = listPriCompare;

    {
      struct MinList *list = (struct MinList*) array;
      switch (algorithm) {
        case SORT_Selection:
          selectSortList(list);
        break;
        case SORT_Insertion:
          insertSortList(list);
        break;
        case SORT_Quick:
          if (list->mlh_Head->mln_Succ) {
            quickSortList(list, list->mlh_Head, list->mlh_TailPred);
          }
        break;
        case SORT_Bubble:
          bubbleSortList(list);
        break;
        case SORT_Merge:
        {
          struct MinNode *item;
          ULONG bufSize, i;
          APTR *arr;
          itemSize = sizeof(APTR);
          realCmpFnc = CompareFnc;
          CompareFnc = listCmpI;

          // Count the size of the list if not given
          if (size == SORT_List) {
            size = 0;
            for (item = list->mlh_Head; item->mln_Succ; item = item->mln_Succ) size++;
          }

          if (size > 1) {
            bufSize = size * itemSize * 2;
            swapBuffer = allocMem(bufSize);
            if (swapBuffer) {
              arr = swapBuffer + (size * itemSize);

              // create an array of item pointers from the list
              item = list->mlh_Head;
              for (i = 0; i < size; i++) {
                arr[i] = (APTR)item;
                item = item->mln_Succ;
              }

              // do a regular array sort
              mergeSort(arr, 0, size - 1);

              // recreate the list from the sorted array of node pointers
              // create the list back from the array
              NewList((struct List*)list);
              for (i = 0; i < size; i++) {
                AddTail((struct List*)list, (struct Node*)arr[i]);
              }

              freeMem(swapBuffer, bufSize);

            }
            else rc = SORT_ERR_MEMORY;
          }
        }
        break;
        case SORT_Shell:
        case SORT_Heap:
        {
          struct MinNode *item;
          ULONG bufSize, i;
          APTR *arr;
          itemSize = sizeof(APTR);
          realCmpFnc = CompareFnc;
          CompareFnc = listCmpI;

          // Count the size of the list if not given
          if (size == SORT_List) {
            size = 0;
            for (item = list->mlh_Head; item->mln_Succ; item = item->mln_Succ) size++;
          }
          if (size > 1) {
            bufSize = (size + 1) * itemSize;
            swapBuffer = allocMem(bufSize);
            if (swapBuffer) {
              arr = swapBuffer + itemSize;

              // create an array of item pointers from the list
              item = list->mlh_Head;
              for (i = 0; i < size; i++) {
                arr[i] = (APTR)item;
                item = item->mln_Succ;
              }

              // do a regular array sort
              if (algorithm == SORT_Shell)
                shellSort(arr, size);
              else
                heapSort(arr, size);

              // recreate the list from the sorted array of node pointers
              NewList((struct List*)list);
              for (i = 0; i < size; i++) {
                AddTail((struct List*)list, (struct Node*)arr[i]);
              }

              freeMem(swapBuffer, bufSize);
            }
            else rc = SORT_ERR_MEMORY;
          }
        }
        break;
        case SORT_Radix:
          if (!range)   range   = &decimalRadixRange;
          if (!HashFnc) HashFnc = listPriRadixHash;
          if (!digits)  digits  = 3;
        case SORT_Counting:
          if (!range)   range   = &byteRange;
          if (!HashFnc) HashFnc = listPriHash;

          {
            struct MinNode *item;
            ULONG bufSize, i;
            APTR buffer;
            APTR *arr;

            // set default parameters
            itemSize = sizeof(APTR);
            realHashFnc = HashFnc;
            HashFnc     = listHashI;

            // Count the size of the list if not given
            if (size == SORT_List) {
              size = 0;
              for (item = list->mlh_Head; item->mln_Succ; item = item->mln_Succ) size++;
            }

            if (size > 1) {
              // buffer is used as: array from list | swapBuffer | hashSet
              bufSize = (size * 2 * itemSize) + ((range->max - range->min + 1) * sizeof(ULONG));
              buffer = allocMem(bufSize);
              if (buffer) {
                arr = (APTR*)buffer;
                swapBuffer = buffer + (size * itemSize);

                // create an array of item pointers from the list
                item = list->mlh_Head;
                for (i = 0; i < size; i++) {
                  arr[i] = (APTR)item;
                  item = item->mln_Succ;
                }

                // do a regular array sort
                if (algorithm == SORT_Counting)
                  countingSort(arr, size, range, 0);
                else
                  radixSort(arr, size, range, digits);

                // recreate the list from the sorted array of item pointers
                NewList((struct List*)list);
                for (i = 0; i < size; i++) {
                  AddTail((struct List*)list, (struct Node*)arr[i]);
                }

                freeMem(buffer, bufSize);
                swapBuffer = NULL;
              }
              else rc = SORT_ERR_MEMORY;
            }
          }
        break;
      }
    }
    break;
    case SORT_Array:
      if (!itemSize) itemSize = sizeof(LONG);
      if (!CompareFnc)
      {
        switch (itemSize) {
          case 1:
            CompareFnc = byteCompare;
          break;
          case 2:
          case 3:
            CompareFnc = wordCompare;
          break;
          default:
            CompareFnc = longCompare;
        }
      }
      switch (algorithm) {
        case SORT_Selection:
        case SORT_Insertion:
        case SORT_Shell:
        case SORT_Bubble:
        case SORT_Quick:
        case SORT_Heap:
          swapBuffer = allocMem(itemSize);
          if (swapBuffer) {
            switch (algorithm) {
              case SORT_Selection:
                selectSort(array, size);
              break;
              case SORT_Insertion:
                insertSort(array, size);
              break;
              case SORT_Shell:
                shellSort(array, size);
              break;
              case SORT_Bubble:
                bubbleSort(array, size);
              break;
              case SORT_Quick:
                quickSort(array, 0, size - 1);
              break;
              case SORT_Heap:
                heapSort(array, size);
              break;
            }
            freeMem(swapBuffer, itemSize);
            swapBuffer = NULL;
          }
          else rc = SORT_ERR_MEMORY;
        break;
        case SORT_Merge:
          swapBuffer = allocMem(size * itemSize);
          if (swapBuffer) {
            mergeSort(array, 0, size - 1);
            freeMem(swapBuffer, size * itemSize);
            swapBuffer = NULL;
          }
          else rc = SORT_ERR_MEMORY;
        break;
        case SORT_Counting:
          if (!HashFnc)
          {
            switch (itemSize) {
              case 1:
                HashFnc = byteHash;
                if (!range) range = &byteRange;
              break;
              case 2:
              case 3:
                HashFnc = wordHash;
              break;
              default:
                HashFnc = longHash;
            }
          }
          if (range)
          {
            // bufSize is :  temp buffer size + hashSet size
            ULONG bufSize = (size * itemSize) + ((range->max - range->min + 1) * sizeof(ULONG));
            swapBuffer = allocMem(bufSize);
            if (swapBuffer) {
              countingSort(array, size, range, 0);
              freeMem(swapBuffer, bufSize);
              swapBuffer = NULL;
            }
            else rc = SORT_ERR_MEMORY;
          }
          else rc = SORT_ERR_RANGE;
        break;
        case SORT_Radix:
          if (!HashFnc) HashFnc = longRadixHash;
          if (!range) range = &decimalRadixRange;
          if (digits)
          {
            // bufSize is :  temp buffer size + hashSet size
            ULONG bufSize = (size * itemSize) + ((range->max - range->min + 1) * sizeof(ULONG));
            swapBuffer = allocMem(bufSize);
            if (swapBuffer) {
              radixSort(array, size, range, digits);
              freeMem(swapBuffer, bufSize);
              swapBuffer = NULL;
            }
            else rc = SORT_ERR_MEMORY;
          }
          else rc = SORT_ERR_DIGITS;
        break;
      }
    break;
  }

  return rc;
}
///
///Sort
#ifdef __amigaos4__
LIBPROTOVA(Sort, ULONG, REG(a6, UNUSED __BASE_OR_IFACE), REG(a0, APTR array), REG(d0, ULONG size), REG(d1, ULONG tag1), ...)
{
  return SortA(array, size, (struct TagItem*)&tag1);
}
#endif
///

/***************************
 * Built-in Hash functions *
 ***************************/
/// listHashI
/******************************************************************************
 * Interface for hash functions when sorting a list using an array.           *
 ******************************************************************************/
ULONG listHashI(APTR p, ULONG digit)
{
  return realHashFnc(*((APTR*)p), digit);
}
///
/// byteHash
/******************************************************************************
 * Creates an hash value from the given pointer to an array item as BYTE.     *
 ******************************************************************************/
ULONG byteHash(APTR p, UNUSED ULONG digit)
{
  return (ULONG)(*((BYTE*)p));
}
///
/// wordHash
/******************************************************************************
 * Creates an hash value from the given pointer to an array item as WORD.     *
 ******************************************************************************/
ULONG wordHash(APTR p, UNUSED ULONG digit)
{
  return (ULONG)(*((WORD*)p));
}
///
/// longHash
/******************************************************************************
 * Creates an hash value from the given pointer to an array item.             *
 * Sort() will default to this for SORT_Counting.                             *
 ******************************************************************************/
ULONG longHash(APTR p, UNUSED ULONG digit)
{
  return (ULONG)(*((LONG*)p));
}
///
/// listPriHash
/******************************************************************************
 * Creates an hash from a list item using it's priority (ln_Pri) member.      *
 * This assumes that the node is a full node (struct Node). Sorting a MinNode *
 * with this compare method may result in an arbitrarily sorted list.         *
 * Since this is the default hash function for lists, developers should       *
 * provide their hash functions via SORT_HashFnc tag if they are using        *
 * MinNodes. This will also require a range between -127 and 128 when using   *
 * SORT_Counting (which will be set as default).                              *
 ******************************************************************************/
ULONG listPriHash(APTR p, UNUSED ULONG digit)
{
  struct Node *n = (struct Node*)p;
  LONG v = n->ln_Pri;
  return (ULONG)v;
}
///
/// longRadixHash
/******************************************************************************
 * Creates an hash value from the given pointer. Assumes the pointer given    *
 * points to a LONG value. Creates an hash value according to the given digit.*
 * Sort() will default to this for SORT_Radix.                                *
 ******************************************************************************/
ULONG longRadixHash(APTR p, ULONG digit)
{
   LONG v = *((LONG*)p);
   ULONG i;
   for (i = 0; i < digit; i++) v /= 10;
   return (ULONG)(v%10);
}
///
/// listPriRadixHash
/******************************************************************************
 * Creates an hash value from the given pointer to a list node. Assumes the   *
 * node to be a full node (struct List). Sorting a MinNode with this hash     *
 * method may result in an arbitrarily sorted list (if it doesn't crash).     *
 * Since this is the default hash function for sorting lists with radixSort,  *
 * developers should provide their own hash functions via SORT_HashFnc tag if *
 * they are using MinNodes and/or want to sort against another member other   *
 * than ln_Pri.                                                               *
 * Hash value will be the given digit of the ln_Pri member.                   *
 * Sort() will default to this for SORT_Radix.                                *
 ******************************************************************************/
ULONG listPriRadixHash(APTR p, ULONG digit)
{
  struct Node* n = (struct Node*)p;
  LONG v = n->ln_Pri;
  ULONG i;
  for (i = 0; i < digit; i++) v /= 10;
  return (ULONG)(v%10);
}
///

/******************************
 * Built-in Compare Functions *
 ******************************/
/// listCmpI
/******************************************************************************
 * Interface for compare function when sorting a list using an array from it. *
 ******************************************************************************/
LONG listCmpI(APTR p1, APTR p2)
{
  return realCmpFnc(*((APTR*)p1), *((APTR*)p2));
}
///
/// byteCompare()
/******************************************************************************
 * Compares the two given pointers to array items against the value they hold *
 * (as a BYTE value).                                                         *
 ******************************************************************************/
LONG byteCompare(APTR p1, APTR p2)
{
  BYTE val1 = *((BYTE*)p1);
  BYTE val2 = *((BYTE*)p2);

  if (val1 == val2) return 0;
  if (val1 <  val2) return -1;
  else return 1;
}
///
/// wordCompare()
/******************************************************************************
 * Compares the two given pointers to array items against the value they hold *
 * (as a WORD value).                                                         *
 ******************************************************************************/
LONG wordCompare(APTR p1, APTR p2)
{
  WORD val1 = *((WORD*)p1);
  WORD val2 = *((WORD*)p2);

  if (val1 == val2) return 0;
  if (val1 <  val2) return -1;
  else return 1;
}
///
/// longCompare()
/******************************************************************************
 * Compares the two given pointers to array items against the value they hold *
 * (as a LONG value).                                                         *
 ******************************************************************************/
LONG longCompare(APTR p1, APTR p2)
{
  LONG val1 = *((LONG*)p1);
  LONG val2 = *((LONG*)p2);

  if (val1 == val2) return 0;
  if (val1 <  val2) return -1;
  else return 1;
}
///
/// listPriCompare()
/******************************************************************************
 * Compares the two given list nodes against their priority (ln_Pri) members. *
 * This assumes that the node is a full node (struct Node). Sorting a MinNode *
 * with this compare method may result in an arbitrarily sorted list.         *
 * Since this is the default compare function for lists, developers should    *
 * provide their compare functions via SORT_CompareFnc tag if they are using  *
 * MinNodes.                                                                  *
 ******************************************************************************/
LONG listPriCompare(APTR p1, APTR p2)
{
  struct Node *n1 = (struct Node*) p1;
  struct Node *n2 = (struct Node*) p2;

  if (n1->ln_Pri == n2->ln_Pri) return  0;
  if (n1->ln_Pri <  n2->ln_Pri) return -1;
  else return 1;
}
///

/******************
 * Swap Functions *
 ******************/
///swap      (swap function for arrays)
VOID swap(APTR p1, APTR p2)
{
  if (p1 != p2)
  {
    CopyMem(p1, swapBuffer, itemSize);
    CopyMem(p2, p1, itemSize);
    CopyMem(swapBuffer, p2, itemSize);
  }
}
///
///swapNodes (swap function for lists)
/******************************************************************************
 * Swaps the placing of items in a list by changing link pointers             *
 * (which means without moving their the payloads in memory).                 *
 ******************************************************************************/
VOID swapNodes(struct MinList *header, struct MinNode *n1, struct MinNode *n2)
{
  if (n1 != n2)
  {
    struct MinNode t1;
    struct MinNode t2;
    struct MinNode *prev;
    struct MinNode *next;
    // memorize initial link pointers in both nodes
    t1.mln_Succ = n1->mln_Succ;
    t1.mln_Pred = n1->mln_Pred;
    t2.mln_Succ = n2->mln_Succ;
    t2.mln_Pred = n2->mln_Pred;

    // set the link pointers of the surrounding members
    prev = t1.mln_Pred;
    if ((APTR)prev != (APTR)header)
      prev->mln_Succ = n2;
    else
      header->mlh_Head = n2;

    next = t1.mln_Succ;
    if ((APTR)next != (APTR)header + sizeof(ULONG))
      next->mln_Pred = n2;
    else
      header->mlh_TailPred = n2;

    prev = t2.mln_Pred;
    if ((APTR)prev != (APTR)header)
      prev->mln_Succ = n1;
    else
      header->mlh_Head = n1;

    next = t2.mln_Succ;
    if ((APTR)next != (APTR)header + sizeof(ULONG))
      next->mln_Pred = n1;
    else
      header->mlh_TailPred = n1;

    if (t1.mln_Succ == n2) {
      n1->mln_Succ = t2.mln_Succ;
      n1->mln_Pred = n2;
      n2->mln_Succ = n1;
      n2->mln_Pred = t1.mln_Pred;
    }
    else if (t2.mln_Succ == n1) {
      n1->mln_Succ = n2;
      n1->mln_Pred = t2.mln_Pred;
      n2->mln_Succ = t1.mln_Succ;
      n2->mln_Pred = n1;
    }
    else {
      n1->mln_Succ = t2.mln_Succ;
      n1->mln_Pred = t2.mln_Pred;
      n2->mln_Succ = t1.mln_Succ;
      n2->mln_Pred = t1.mln_Pred;
    }
  }
}
///

/******************
 * Sort Functions *
 ******************/
///quickSort
LONG partition(APTR arr, ULONG low, ULONG high)
{
  ULONG i = low;
  ULONG l;
  APTR pivot;

  // choose a better pivot to optimize the algorithm
  {
    ULONG med = low + (high - low)/2;
    ULONG max = MAX_ARR(MAX_ARR(low, med), high);

    if (max == high)
      swap(index(arr, MAX_ARR(low, med)), index(arr, high));
    else if (max == med)
    {
      ULONG mid = MAX_ARR(low, high);
      if (mid != high)
        swap(index(arr, mid), index(arr, high));
    }
    else
    {
      ULONG mid = MAX_ARR(med, high);
      if (mid != high)
        swap(index(arr, mid), index(arr, high));
    }

    pivot = index(arr, high);
  }

  for (l = low; l < high; l++) {
    if (CompareFnc(pivot, index(arr, l)) == sortOrder) {
      swap(index(arr, i), index(arr, l));
      i++;
    }
  }
  swap(index(arr, i), index(arr, high));
  return i;
}

VOID quickSort(APTR arr, ULONG low, ULONG high)
{
  if (low < high)
  {
    ULONG pi = partition(arr, low, high); // pi = partitioning index
    if (pi != low)
      quickSort(arr, low, pi - 1);  // Sort left partition
    if (pi != high)
      quickSort(arr, pi + 1, high); // Sort right partition
  }
}
///
///quickSortList
struct MinNode *partitionList(struct MinList *list, struct Partition* part)
{
  struct MinNode *high = part->high;
  struct MinNode *low  = part->low;
  struct MinNode *i = low;
  struct MinNode *l;
  struct MinNode *ln;
  BOOL firstTime = TRUE;

  APTR pivot = (APTR)high;

  for (l = low; l != high; l = ln) {
    ln = l->mln_Succ;
    if (CompareFnc(pivot, (APTR)l) == sortOrder) {
      if (firstTime && i == low) {part->low = l; firstTime = FALSE;}
      swapNodes(list, i, l);        // l is now in the place of i so to
      i = l->mln_Succ;              // iterate i we take the next item of l
    }
  }
  if (firstTime) part->low = high;
  swapNodes(list, i, high);
  part->high = i;
  return high;
}

VOID quickSortList(struct MinList *list, struct MinNode *low, struct MinNode *high)
{
  struct Partition part;
  part.low = low;
  part.high = high;

  if (low != high)
  {
    struct MinNode *pi = partitionList(list, &part);  // pi = partitioning item
    if (part.low != pi) {
      quickSortList(list, part.low, pi->mln_Pred);    // Sort left partition
    }
    if (part.high != pi) {
      quickSortList(list, pi->mln_Succ, part.high);   // Sort right partition
    }
  }
}
///
///insertSort
VOID insertSort(APTR arr, ULONG size)
{
  APTR i;
  APTR l;

  for (i = arr + itemSize; i < (arr + (size * itemSize)); i += itemSize)
  {
    CopyMem(i, swapBuffer, itemSize);                 // v = arr[i];

    for (l = i - itemSize; l >= arr; l -= itemSize)
    {
      LONG cmpVal = CompareFnc(swapBuffer, l);
      if (cmpVal == 0 || cmpVal == sortOrder) break;  // if (arr[l] <= v)
      else CopyMem(l, l + itemSize, itemSize);        // arr[l+1] = arr[l];
    }
    CopyMem(swapBuffer, l + itemSize, itemSize);      // arr[l+1] = v;
  }
}

VOID insertSortList(struct MinList *list)
{
  struct MinNode *i, *in, *l, *lp;
  LONG cmpVal;

  // check for empty list
  if (!list->mlh_Head->mln_Succ) return;

  for (i = list->mlh_Head->mln_Succ; i->mln_Succ; i = in)
  {
    in = i->mln_Succ;

    for (l = i->mln_Pred; l->mln_Pred; l=lp)
    {
      lp = l->mln_Pred;
      cmpVal = CompareFnc(i, l);
      if (cmpVal == 0 || cmpVal == sortOrder) break;
      else swapNodes(list, l, i);
    }
  }
}
///
///mergeSort
VOID merge(APTR arr, ULONG l, ULONG m, ULONG r)
{
  APTR i, j, k;          // memory iterators
  APTR L, R;             // margins of Left and Right temporary arrays
  ULONG LSize = (m - l + 1) * itemSize; // Sizes of temporary arrays...
  ULONG RSize = (r - m) * itemSize;     // ...Left and Right

  /* create temp arrays */
  i = swapBuffer;
  j = i + LSize;
  k = index(arr, l); // Initial index of merged subarray
  L = i+LSize;  // margin of Left temporary array
  R = j+RSize;  // margin of Right temporary array

  /* Copy data to temp arrays L[] and R[] */
  CopyMem(k, i, LSize + RSize);

  /* Merge the temp arrays back into arr[l..r]*/
  while (i < L && j < R)
  {
    LONG cmpVal = CompareFnc(j, i);
    if (cmpVal == 0 || cmpVal == sortOrder) // if (L[i] <= R[j])
    {
      CopyMem(i, k, itemSize);              // arr[k] = L[i];
      i += itemSize;
    }
    else
    {
      CopyMem(j, k, itemSize);              // arr[k] = R[j];
      j += itemSize;
    }
    k += itemSize;
  }

  /* Copy the remaining elements of L[], if there are any */
  while (i < L)
  {
    CopyMem(i, k, itemSize);                // arr[k] = L[i];
    i += itemSize;
    k += itemSize;
  }

  /* Copy the remaining elements of R[], if there are any */
  while (j < R)
  {
    CopyMem(j, k, itemSize);                // arr[k] = R[j];
    j += itemSize;
    k += itemSize;
  }
}

VOID mergeSort(APTR arr, ULONG l, ULONG r)
{
  if (l < r)
  {
    // partition the array in two parts from the middle
    ULONG m = l+(r-l)/2; // Same as (l+r)/2, but avoids overflow for large l and r

    // partition all sub partitions recursively
    mergeSort(arr, l, m);
    mergeSort(arr, m+1, r);

    // then merge them all
    merge(arr, l, m, r);
  }
}
///
///selectSort
VOID selectSort(APTR arr, ULONG size)
{
  APTR i, l, smallest;
  APTR iMax = arr + (size * itemSize);

  for (i = arr; i < iMax; i+=itemSize) {
    smallest = i;
    for (l = i + itemSize; l < iMax; l+=itemSize) {
      if (CompareFnc(smallest, l) == sortOrder) smallest = l;
    }
    swap(i, smallest);
  }
}

VOID selectSortList(struct MinList *list)
{
  struct MinNode *i, *in, *l, *s;

  for (i = list->mlh_Head; i->mln_Succ; i = in) {
    in = i->mln_Succ;
    s = i; // node with the smallest value
    for (l = in; l->mln_Succ; l = l->mln_Succ) {
      if (CompareFnc((APTR)s, (APTR)l) == sortOrder) s = l;
    }
    if (i->mln_Succ == s) in = i;
    swapNodes(list, i, s);
  }
}
///
///shellSort
VOID shellSort(APTR arr, ULONG size)
{
  ULONG i, j;
  ULONG gap;

  for (gap = size/2; gap > 0; gap /= 2)
  {
    for (i = gap; i < size; i++)
    {
      CopyMem(index(arr, i), swapBuffer, itemSize);

      for (j = i; j >= gap && CompareFnc(index(arr, j-gap), swapBuffer) == sortOrder; j -= gap) {
        CopyMem(index(arr, j-gap), index(arr, j), itemSize);
      }

      CopyMem(swapBuffer, index(arr, j), itemSize);
    }
  }
}
///
///countingSort
VOID countingSort(APTR arr, ULONG size, struct Range *range, ULONG digit)
{
  ULONG *hashSet = (ULONG*)(swapBuffer + (size * itemSize));
  ULONG hashSetSize = range->max - range->min + 1;
  LONG offset = 0 - range->min;
  APTR i;
  ULONG l;

  // Initialize the hashSet to all zeroes
  for (l = 0; l < hashSetSize; l++) {
    hashSet[l] = 0;
  }

  // fill the hash table
  for (i = arr; i < (arr + (size * itemSize)); i+=itemSize) {
    hashSet[HashFnc(i, digit)+offset]++;
  }

  // mutate the hash table for counting sort
  for (l = 1; l < hashSetSize; l++) {
    hashSet[l] += hashSet[l-1];
  }

  if (sortOrder == 1)
  {
    // Values in the hashSet (-1) are the correct indices for items
    // (reverse traversal is to make the algorithm stable)
    for (i = (arr + ((size-1) * itemSize)); i >= arr; i-=itemSize) {
      ULONG index = --hashSet[HashFnc(i, digit)+offset];
      CopyMem(i, swapBuffer + (index * itemSize), itemSize);
    }
  }
  else // sort in reverse
  {
    for (i = (arr + ((size-1) * itemSize)); i >= arr; i-=itemSize) {
      ULONG index = --hashSet[HashFnc(i, digit)+offset];
      CopyMem(i, swapBuffer + ((size-index-1) * itemSize), itemSize);
    }
  }

  // copy the sorted array in temp buffer over the actual array
  CopyMem(swapBuffer, arr, itemSize * size);
}
///
///radixSort
VOID radixSort(APTR arr, ULONG size, struct Range *range, ULONG digits)
{
  ULONG d;

  for (d = 0; d < digits; d++)
  {
    countingSort(arr, size, range, d);
  }
}
///
///heapSort
VOID heapify(APTR arr, ULONG size, ULONG root)
{
  ULONG largest = root; // initialize largest as root
  ULONG l = 2*root + 1; // left child
  ULONG r = 2*root + 2; // right child

  // compare left child with root
  if (l < size && CompareFnc(index(arr, l), index(arr, largest)) == sortOrder)
    largest = l;

  // compare right child with root
  if (r < size && CompareFnc(index(arr, r), index(arr, largest)) == sortOrder)
    largest = r;

  // max heapify
  if (largest != root)
  {
    swap(index(arr, root), index(arr, largest));

    // recursively trickle down the sub-tree
    heapify(arr, size, largest);
  }
}

VOID heapSort(APTR arr, ULONG size)
{
  LONG i;
  // mutate the array for it to become a max heap
  for (i = size / 2 - 1; i >= 0; i--)
    heapify(arr, size, i);

  // remove elements from the heap one by one
  for (i=size-1; i>=0; i--)
  {
    // move current root to the end of the array
    swap(arr, index(arr, i));

    // re-heapify the reduced heap
    heapify(arr, i, 0);
  }
}
///
///bubbleSort
/******************************************************************************
 * NOTE: Optimize bubbleSort() to not use index() macros!                     *
 ******************************************************************************/
VOID bubbleSort(LONG *arr, ULONG size)
{
  ULONG i, j;

  for (i = 0; i < size-1; i++)
    for (j = 0; j < size-i-1; j++)
      if (arr[j] > arr[j+1])
        swap(index(arr, j), index(arr, j+1));
}

VOID bubbleSortList(struct MinList *list)
{
  struct MinNode *i;
  struct MinNode *stop;

  // check for empty list
  if (!list->mlh_Head->mln_Succ) return;

  for (stop = list->mlh_TailPred; stop != list->mlh_Head;){
    for (i = list->mlh_Head; TRUE;) {
      if (i->mln_Succ == stop) {
        if (CompareFnc(i, i->mln_Succ) == sortOrder) {
          swapNodes(list, i, i->mln_Succ);
          i = i->mln_Pred;
        }
        break;
      }
      else {
        if (CompareFnc(i, i->mln_Succ) == sortOrder)
          swapNodes(list, i, i->mln_Succ);
        else
          i = i->mln_Succ;
      }
    }
    stop = i;
  }
}
///

///MorphOS stubs
#ifdef __MORPHOS__

LIBSTUB(SortA, ULONG)
{
	struct SortBase *base = (struct SortBase*)REG_A6;
	return LIB_SortA(base, (APTR)REG_A0, (ULONG)REG_D0, (struct TagItem *)REG_A1);
}

#endif /* __MORPHOS__ */
///

/******************************************************************************
 * Example 03_Arrays_Advanced: Advanced array sorting with sort.library       *
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

//Custom array element structure
struct Person
{
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

//Utility functions
///printArray(arr, size)
VOID printArray(struct Person *arr, ULONG size)
{
  ULONG i;
  struct Person *person;

  for (i = 0, person = arr; i < size; i++, person++)
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
      struct Person people[ARRAY_SIZE] = {{"James", "Hetfield", 2, MALE,   56},
                                          {"Lars",  "Ulrich",   9, MALE,   56},
                                          {"Kirk",  "Hammet",   7, MALE,   57},
                                          {"Cliff", "Burton",   1, MALE,   24},
                                          {"Jason", "Newsted",  8, MALE,   57},
                                          {"Dave",  "Mustaine", 3, MALE,   58},
                                          {"Marty", "Friedman", 4, MALE,   57},
                                          {"Dave",  "Ellefson", 5, MALE,   55},
                                          {"Nick",  "Menza",    6, MALE,   55},
                                         };
      puts("Initial Array:");
      printArray(people, ARRAY_SIZE);

      //Sort the people array by id using Quick Sort
      Sort(people, ARRAY_SIZE, SORT_Algorithm, SORT_Quick,
                               SORT_ItemSize, sizeof(struct Person),
                               SORT_CompareFnc, idCmp,
                               TAG_END);

      puts("\nSorted Array by id:");
      printArray(people, ARRAY_SIZE);

      //Let's sort the array by name first and sirname second
      //To do this we initially sort by sirname with any algorithm...
      Sort(people, ARRAY_SIZE, SORT_Algorithm, SORT_Auto,
                               SORT_ItemSize, sizeof(struct Person),
                               SORT_CompareFnc, sirNameCmp,
                               TAG_END);

      //...then sort by firstname with a "stable" sort algorithm
      Sort(people, ARRAY_SIZE, SORT_Algorithm, SORT_Merge,
                               SORT_ItemSize, sizeof(struct Person),
                               SORT_CompareFnc, nameCmp,
                               TAG_END);

      puts("\nSorted Array by name & sirname:");
      printArray(people, ARRAY_SIZE);

  #ifdef __amigaos4__
      DropInterface((struct Interface *)ISort);
    }
  #endif
  }
  else puts("Could not open sort.library!");

  return(rc);
}

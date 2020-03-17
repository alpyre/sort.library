/******************************************************************************
 * Example 01_Array_Basics: Sorting arrays with sort.library                  *
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

//Utility functions
///printArray(arr, size)
VOID printArray(LONG *arr, ULONG size)
{
  UBYTE buffer[8];
  UBYTE result[256];
  LONG i;

  result[0] = 0;

  for (i = 0; i < size; i++)
  {
    sprintf(buffer, "%ld", arr[i]);
    if (i != 0) strcat(result, ", ");
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

      LONG arr[10] = {6, 7, 3, 2, 9, 8, 0, 1, 5, 4};
      puts("Initial Array:");
      printArray(arr, 10);

      Sort(arr, 10, TAG_END); //Sort defaults to array of LONG's. See documents.

      puts("Sorted Array:");
      printArray(arr, 10);

  #ifdef __amigaos4__
      DropInterface((struct Interface *)ISort);
    }
  #endif
  }
  else puts("Could not open sort.library!");

  return(rc);
}

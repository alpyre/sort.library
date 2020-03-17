/******************************************************************************
 * Example 05_DOS_Sort: AmigaDOS Sort replacement which uses sort.library     *
 ******************************************************************************/

///definitions
#define PROGRAMNAME     "Sort"
#define VERSION         0
#define REVISION        6
#define VERSIONSTRING   "0.6"

//define command line syntax and number of options
#define RDARGS_TEMPLATE "FROM/A, TO/A, CS=COLSTART/K, C=CASE/S, N=NUMERIC/S"
#define RDARGS_OPTIONS  5
///
///includes
#include <stdio.h>
#include <string.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <libraries/locale.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/locale.h>
#include <proto/utility.h>

#include <sort/sort.h>
///
///structures
struct Config
{
  struct RDArgs *RDArgs;
  LONG Options[RDARGS_OPTIONS];
};

struct Line
{
  struct MinNode node;
  STRPTR string;
  STRPTR cmpStr;
};
///
///globals
//Version String
#if defined(__SASC)
  const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " "  __AMIGADATE__ "\n\0";
#elif defined(_DCC)
  const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __COMMODORE_DATE__ ")\n\0";
#elif defined(__GNUC__)
  __attribute__((section(".text"))) volatile static const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __DATE__ ")\n\0";
#else
  const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __DATE__ ")\n\0";
#endif

//Library base and interface for sort.library
struct Library *SortBase;
#ifdef __amigaos4__
  struct SortIFace *ISort;
#endif

APTR memPool;
struct Locale *locale;
///
///entry
int main(int argc, char **argv)
{
  int rc = 20;

  memPool = CreatePool(MEMF_ANY, 8192, 8192);
  if (memPool)
  {
    if (argc)
    {
      struct Config *config = (struct Config*)AllocPooled(memPool, sizeof(struct Config));
      if (config)
      {
        memset(config, 0, sizeof(struct Config));
        if (config->RDArgs = ReadArgs(RDARGS_TEMPLATE, config->Options, NULL))
        {
          rc = Main(config);
          FreeArgs(config->RDArgs);
        }
        else
        PrintFault(IoErr(), PROGRAMNAME);

        FreePooled(memPool, config, sizeof(struct Config));
      }
    }
    DeletePool(memPool);
  }
  else
    puts("Out of memory");

  return(rc);
}
///

///addFirstLine(list, str)
/***********************************************************************
 * Pushes the address of the first line from file to the given list.   *
 ***********************************************************************/
VOID addFirstLine(struct MinList *list, STRPTR str)
{
  struct Line *line = AllocPooled(memPool, sizeof(struct Line));
  if (line)
  {
    line->string = str;
    AddTail((struct List*)list, (struct Node*)line);
  }
}
///
///addNewLine(list, str, prevSkipChars)
/***********************************************************************
 * Pushes the address of the next line from file to the given list and *
 * sets the compare string for the previous line.                      *
 ***********************************************************************/
VOID addNewLine(struct MinList *list, STRPTR str, ULONG prevSkipChars)
{
  struct Line *line = AllocPooled(memPool, sizeof(struct Line));
  struct Line *prev = (struct Line*)list->mlh_TailPred;
  prev->cmpStr = prev->string + prevSkipChars;

  if (line)
  {
    line->string = str;
    AddTail((struct List*)list, (struct Node*)line);
  }
}
///
///strNumericCmp(str1, str2)
/***********************************************************************
 * A strcmp() variant which ignores non numeric characters.            *
 ***********************************************************************/
LONG strNumericCmp(STRPTR s1, STRPTR s2)
{
  UBYTE c1, c2;
  do
    {
      c1 = *s1++;
      c2 = *s2++;
      if (c1 < '0' || c1 > '9') c1 = 0;
      if (c2 < '0' || c2 > '9') c2 = 0;
      if (c1 == 0)
        return c1 - c2;
    }
  while (c1 == c2);
  return c1 - c2;
}
///

/*********************
 * Compare functions *
 *********************/
///srtCmp(p1, p2)
/***********************************************************************
 * Compares the strings of two line items case sensitively.            *
 ***********************************************************************/
LONG strCmp(APTR p1, APTR p2)
{
  STRPTR s1 = ((struct Line *)p1)->cmpStr;
  STRPTR s2 = ((struct Line *)p2)->cmpStr;
  LONG cmpVal;

  cmpVal = StrnCmp(locale, s1, s2, -1, SC_ASCII);

  if      (cmpVal > 0) return  1;
  else if (cmpVal < 0) return -1;
  else                 return  0;
}
///
///striCmp(p1, p2)
/***********************************************************************
 * Compares the strings of two line items case insensitively.          *
 ***********************************************************************/
LONG striCmp(APTR p1, APTR p2)
{
  STRPTR s1 = ((struct Line *)p1)->cmpStr;
  STRPTR s2 = ((struct Line *)p2)->cmpStr;
  LONG cmpVal;

  cmpVal = Stricmp(s1, s2);

  if      (cmpVal > 0) return  1;
  else if (cmpVal < 0) return -1;
  else                 return  0;
}
///
///numCmp(p1, p2)
/***********************************************************************
 * Compares the strings of two line items numerically.                 *
 ***********************************************************************/
LONG numCmp(APTR p1, APTR p2)
{
  STRPTR s1 = ((struct Line *)p1)->cmpStr;
  STRPTR s2 = ((struct Line *)p2)->cmpStr;
  LONG cmpVal;

  cmpVal = strNumericCmp(s1, s2);

  if      (cmpVal > 0) return  1;
  else if (cmpVal < 0) return -1;
  else                 return  0;
}
///

///Main(config)
int Main(struct Config *config)
{
  int rc = 0;
  struct MinList lines;
  //get command line arguments to more readible identifiers
  STRPTR fn_From  = config->Options[0];
  STRPTR fn_To    = config->Options[1];
  ULONG  colStart;
  BOOL   caseSens = config->Options[3];
  BOOL   numeric  = config->Options[4];
  StrToLong(config->Options[2], &colStart);

  NewList((struct List *)&lines);

  SortBase = OpenLibrary("sort.library", 0);
  if (SortBase)
  {
  #ifdef __amigaos4__
    ISort = (struct SortIFace *)GetInterface(SortBase, "main", 1, NULL);
    if (ISort)
    {
  #endif

    BPTR fh = Open(fn_From, MODE_OLDFILE);
    if (caseSens) locale = OpenLocale(NULL);

    if (fh)
    {
    #ifdef __amigaos4__
      struct ExamineData *ed = ExamineObjectTags(EX_FileHandleInput, fh, TAG_END);
      if (ed)
      {
        ULONG bufferSize = ed->FileSize + 1;
    #else
      struct FileInfoBlock fib;
      if (ExamineFH(fh, &fib))
      {
        ULONG bufferSize = fib.fib_Size + 1;
    #endif

        UBYTE *buffer = (UBYTE*)AllocPooled(memPool, bufferSize);
        if (buffer)
        {
          ULONG numLines = 1;
          ULONG strLen = 0;
          LONG ch;
          UBYTE *c = buffer;
          *c = NULL;

          //Read/analize/store lines from file in one go
          addFirstLine(&lines, c);
          while ((ch = FGetC(fh)) != -1)
          {
            if (ch == '\n') {
              *c = NULL; // null terminate the buffer if we hit a newline
               c++;
               addNewLine(&lines, c, strLen > colStart ? colStart : strLen);
               numLines++;
               strLen = 0;
            }
            else {
              *c = (UBYTE) ch;
              c++;
              strLen++;
            }
          }
          *c = NULL;
          //Set the compare string for the final line
          ((struct Line*)lines.mlh_TailPred)->cmpStr = ((struct Line*)lines.mlh_TailPred)->string + (strLen > colStart ? colStart : strLen);

          //Sort the lines with a stable sort algorithm
          Sort(&lines, SORT_List, SORT_Algorithm,  SORT_Merge,
                                  SORT_ListSize,   numLines,
                                  SORT_CompareFnc, numeric ? numCmp : (caseSens ? strCmp : striCmp),
                                  SORT_MemPool,    memPool,
                                  TAG_END);
          //Write output file
          {
            BOOL err = FALSE;
            BPTR fh = Open(fn_To, MODE_NEWFILE);
            if (fh)
            {
              struct Line *line;
              for (line = (struct Line*) lines.mlh_Head; line->node.mln_Succ; line = (struct Line*)line->node.mln_Succ)
              {
                if (FPuts(fh, line->string)) { err = TRUE; break;}
                if (line != (struct Line*) lines.mlh_TailPred)
                  if (FPutC(fh, '\n') == EOF) { err = TRUE; break;}
              }
              if (err) puts("Error writing to output file");
              Close(fh);
            }
            else
            {
              rc = 20;
              printf("Can't open %s for output\n", fn_To);
            }
          }
        }
      #ifdef __amigaos4__
        FreeDosObject(DOS_EXAMINEDATA, ed);
      #endif
      }
      Close(fh);
    }
    else
    {
      rc = 20;
      printf("Can't open %s\n", fn_From);
    }

    if (caseSens) CloseLocale(locale);

  #ifdef __amigaos4__
      DropInterface((struct Interface *)ISort);
    }
  #endif
  }
  else puts("Could not open sort.library!");

  return(rc);
}
///

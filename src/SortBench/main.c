/******************************************************************************
 * SortBench                                                                  *
 ******************************************************************************/

///definitions
#define PROGRAMNAME     "SortBench"
#define VERSION         0
#define REVISION        1
#define VERSIONSTRING   "0.1"
#define AUTHOR          "İbrahim Alper Sönmez"
#define COPYRIGHT       "© 2020 " AUTHOR
#define CONTACT         "amithlondestek@gmail.com"
#define DESCRIPTION     "A benchmark program for sort.library"

//define command line syntax and number of options
#define RDARGS_TEMPLATE ""
#define RDARGS_OPTIONS  0

//#define or #undef GENERATEWBMAIN to enable workbench startup
#define GENERATEWBMAIN

//missing definitions in NDK & SDK
#if !defined (__MORPHOS__)
  #define MAX(a, b) (a > b ? a : b)
  #define MIN(a, b) (a > b ? b : a)
#endif

#define AddMinTail(a, b) AddTail((struct List*) a, (struct Node*) b)
#if defined(__SASC)
#define NewMinList(a)  NewList((struct List*) a)
#endif

// MUI Appication Return ID signals
#define MUIV_Application_ReturnID_DoubleStart 1
#define MUIV_Application_ReturnID_SortBench   2
///
///includes
//standard headers
#include <limits.h>

//Amiga headers
#include <exec/exec.h>
#include <workbench/startup.h>
#include <libraries/iffparse.h>

//Amiga devices
#include "timer.h"

//Amiga protos
#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <proto/utility.h>

/* MUI headers */
#include <libraries/mui.h>
#include <proto/muimaster.h>
#include <workbench/workbench.h>
#include <SDI_hook.h>

//Sort Library headers
#include <sort/sort.h>
///
///structures
/***********************************************
* Global configuration struct for this program *
************************************************/
struct Config
{
  struct RDArgs *RDArgs;

  //command line options
  #if RDARGS_OPTIONS
  LONG Options[RDARGS_OPTIONS];
  #endif
};

struct MyItem
{
  struct MinNode node;
  ULONG val;
};
///
///globals
/***********************************************
* Version string for this program              *
************************************************/
#if defined(__SASC)
const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " "  __AMIGADATE__ "\n\0";
#elif defined(_DCC)
const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __COMMODORE_DATE__ ")\n\0";
#elif defined(__GNUC__)
__attribute__((section(".text"))) volatile static const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __DATE__ ")\n\0";
#else
const UBYTE VersionTag[] = "$VER: " PROGRAMNAME " " VERSIONSTRING " (" __DATE__ ")\n\0";
#endif

#ifdef __amigaos4__
  struct Device *TimerBase = NULL;
#else
  struct Library *TimerBase = NULL;
#endif
struct timerequest *timeReq = NULL;
struct Library *SortBase = NULL;
#if defined(__amigaos4__)
  struct SortIFace *ISort;
#endif
///
///MUI globals
struct Library *MUIMasterBase;
#if defined(__amigaos4__)
  struct MUIMasterIFace *IMUIMaster;
#endif

#ifdef __GNUC__
/* Otherwise auto open will try version 37, and muimaster.library has version
   19.x for MUI 3.8 */
int __oslibversion = 0;
#endif

//sort algorithms
STRPTR algName[] = {"Selection",
                    "Insertion",
                    "Shell",
                    "Bubble",
                    "Quick",
                    "Merge",
                    "Counting",
                    "Radix",
                    "Heap"
                   };

enum
{
  _Select = 1,
  _Insert,
  _Shell,
  _Bubble,
  _Quick,
  _Merge,
  _Count,
  _Radix,
  _Heap,
  numAlgorithms
};

Object *App, *Win, *btn_Start, *str_Size, *str_Range, *str_ItemSize;
Object *dsp_State, *gau_Progress, *cyc_Data, *chk_Forbid;
Object *dsp[numAlgorithms];

STRPTR cyc_DSEntries[] = {"Array",
                          "List",
                           NULL
                         };
STRPTR str_NA = "N/A";
STRPTR dsp_AlgHelp = "Elapsed time during sort in seconds.\nLess is better. Best score will be displayed in bold.";
ULONG gauge_counter = 0;
ULONG gauge_steps = numAlgorithms * 2;
///
///prototypes
/***********************************************
* Function forward declarations                *
************************************************/
int            main    (int argc, char **argv);
int            wbmain  (struct WBStartup *wbs);
struct Config *Init    (void);
int            Main    (struct Config *config);
void           CleanUp (struct Config *config);

Object *buildGUI(void);

VOID sortBench(VOID);

VOID fillArrayRnd(APTR arr, ULONG size, ULONG range, ULONG itemSize, ULONG seed);
VOID fillListRnd(struct MinList *list, APTR mem, ULONG size, ULONG range, ULONG seed);
ULONG countDigits(ULONG val);
///
///init
/***********************************************
* Program initialization                       *
* - Allocates the config struct to store the   *
*   global configuration data.                 *
* - Do your other initial allocations here.    *
************************************************/
struct Config *Init()
{
  struct Config *config = (struct Config*)AllocMem(sizeof(struct Config), MEMF_CLEAR);

  if (config)
  {
    timeReq = OpenTimer();
    if (timeReq)
      TimerBase = timeReq->tr_node.io_Device;
    else
    {
      puts("Couldn't open timer.device");
      FreeMem(config, sizeof(struct Config));
      config = NULL;
    }
  }

  return(config);
}
///
///entry
/***********************************************
 * Ground level entry point                    *
 * - Branches regarding Shell/WB call.         *
 ***********************************************/
int main(int argc, char **argv)
{
  int rc = 20;

  //argc != 0 identifies call from shell
  if (argc)
  {
    struct Config *config = Init();

    if (config)
    {
      #if RDARGS_OPTIONS
        // parse command line arguments
        if (config->RDArgs = ReadArgs(RDARGS_TEMPLATE, config->Options, NULL))
          rc = Main(config);
        else
          PrintFault(IoErr(), PROGRAMNAME);
      #else
        rc = Main(config);
      #endif

      CleanUp(config);
    }
  }
  else
    rc = wbmain((struct WBStartup *)argv);

  return(rc);
}

/***********************************************
 * Workbench main                              *
 * - This executable was called from Workbench *
 ***********************************************/
int wbmain(struct WBStartup *wbs)
{
  int rc = 20;

  #ifdef GENERATEWBMAIN
    struct Config *config = Init();

    if (config)
    {
      //<SET Config->Options[] HERE>

      rc = Main(config);

      CleanUp(config);
    }
  #endif

  return(rc);
}
///
///buildGUI
/*******************************
 * Utility GUI object creators *
 *******************************/
Object *MUI_NewIntegerInputBox(LONG initVal, BOOL unSigned, STRPTR help, UBYTE ctrlChr)
{
  STRPTR acceptString = unSigned ? "0123456789" : "0123456789-";
  Object *obj = MUI_NewObject(MUIC_String,
    MUIA_Frame, MUIV_Frame_String,
    MUIA_String_MaxLen, 10,
    MUIA_String_Integer, initVal,
    MUIA_String_Accept, acceptString,
    MUIA_ShortHelp, help,
    MUIA_ControlChar, ctrlChr,
  TAG_END);

  return obj;
}

Object *MUI_NewKeyButton(STRPTR text, UBYTE key)
{
 Object *object;

 object = MUI_NewObject(MUIC_Text,
   MUIA_Frame, MUIV_Frame_Button,
   MUIA_Font, MUIV_Font_Button,
   MUIA_Text_Contents, text,
   MUIA_Text_PreParse, "\33c",
   MUIA_Text_HiChar  , key,
   MUIA_ControlChar  , key,
   MUIA_InputMode    , MUIV_InputMode_RelVerify,
   MUIA_Background   , MUII_ButtonBack,
 TAG_END);

return object;
}

Object *MUI_NewDisplayBox(STRPTR text, STRPTR help)
{
  Object *obj = MUI_NewObject(MUIC_Text,
    MUIA_Frame, MUIV_Frame_ReadList,
    MUIA_Background , MUII_ButtonBack,
    MUIA_Font, MUIV_Font_Button,
    MUIA_Text_Contents, (ULONG)text,
    MUIA_ShortHelp, help,
  TAG_END);

  return obj;
}

Object *MUI_NewCheckMark(BOOL state, STRPTR label, UBYTE key, STRPTR help)
{
 Object *object;

 object = MUI_NewObject(MUIC_Group,
   MUIA_Group_Horiz, TRUE,
   MUIA_Group_Child, MUI_NewObject(MUIC_Image,
     MUIA_Frame, MUIV_Frame_ImageButton,
     MUIA_InputMode, MUIV_InputMode_Toggle,
     MUIA_Image_Spec, MUII_CheckMark,
     MUIA_Image_FreeVert, TRUE,
     MUIA_Selected, state,
     MUIA_Background, MUII_ButtonBack,
     MUIA_ShowSelState, FALSE,
     MUIA_ControlChar, key,
   TAG_END),
   MUIA_Group_Child, MUI_NewObject(MUIC_Text,
     MUIA_Text_Contents, label,
     MUIA_Text_HiChar, key,
   TAG_END),
   MUIA_ShortHelp, help,
 TAG_END);

return object;
}

/***************************************
 * Advances a step at the progress bar *
 ***************************************/
VOID update_gauge(VOID)
{
  LONG gauge_val;
  gauge_counter++;

  gauge_val = gauge_counter * 100 / gauge_steps;
  DoMethod(gau_Progress, MUIM_Set, MUIA_Gauge_Current, gauge_val);
}

/***********************************************************
 * Returns the n'th child object from a group class object *
 ***********************************************************/
Object* getGroupChild(Object* group, LONG n)
{
  struct List *list;
  LONG i;
  Object* obj;
  struct Node* node;

  GetAttr(MUIA_Group_ChildList, group, &list);
  node = list->lh_Head;

  for (i = 0; i<n; i++)
  {
    obj = NextObject(&node);
  }

  return (obj);
}

/******************
 * Hook Functions *
 ******************/
// Range string hook
HOOKPROTO(str_Range_hookfnc, VOID, Object *dest, UNUSED STRPTR *triggerVal)
{
  ULONG val;
  ULONG itemSize;
  GetAttr(MUIA_String_Integer, str_Range, &val);
  GetAttr(MUIA_String_Integer, dest, &itemSize);

  if (val > USHRT_MAX && itemSize < 4) {
    DoMethod(dest, MUIM_Set, MUIA_String_Integer, 4);
    return;
  }

  if (val > UCHAR_MAX && itemSize < 2) {
    DoMethod(dest, MUIM_Set, MUIA_String_Integer, 2);
    return;
  }
}
MakeStaticHook(str_Range_hook, str_Range_hookfnc);

// ItemSize string hook
HOOKPROTO(str_ItemSize_hookfnc, VOID, Object *dest, UNUSED STRPTR *triggerVal)
{
  ULONG val;
  ULONG range;
  GetAttr(MUIA_String_Integer, str_ItemSize, &val);
  GetAttr(MUIA_String_Integer, dest, &range);

  if (val < 2 && range > UCHAR_MAX) {
    DoMethod(dest, MUIM_Set, MUIA_String_Integer, UCHAR_MAX);
    return;
  }

  if (val < 4 && range > USHRT_MAX) {
    DoMethod(dest, MUIM_Set, MUIA_String_Integer, USHRT_MAX);
    return;
  }
}
MakeStaticHook(str_ItemSize_hook, str_ItemSize_hookfnc);

// Item Size string acknowledge hook
HOOKPROTO(str_ItemSize_Acknowledge_hookfnc, VOID, Object *self, UNUSED STRPTR *triggerVal)
{
  ULONG val;
  GetAttr(MUIA_String_Integer, self, &val);

  if (val < 1)
    DoMethod(self, MUIM_Set, MUIA_String_Integer, 1);
}
MakeStaticHook(str_ItemSize_Acknowledge_hook, str_ItemSize_Acknowledge_hookfnc);

// Data structure cycle box update hook
HOOKPROTO(cyc_Data_hookfnc, VOID, Object *dest, ULONG *triggerVal)
{
  if (*triggerVal == 1)
    DoMethod(dest, MUIM_Set, MUIA_Disabled, TRUE);
  else
    DoMethod(dest, MUIM_Set, MUIA_Disabled, FALSE);
}
MakeStaticHook(cyc_Data_hook, cyc_Data_hookfnc);

/***********************************************
 * Program main window                         *
 * - Creates the MUI Application object.       *
 ***********************************************/
Object *buildGUI()
{
  App = MUI_NewObject(MUIC_Application,
    MUIA_Application_Author, (ULONG)AUTHOR,
    MUIA_Application_Base, (ULONG)PROGRAMNAME,
    MUIA_Application_Copyright, (ULONG)COPYRIGHT,
    MUIA_Application_Description, (ULONG)DESCRIPTION,
    MUIA_Application_Title, (ULONG)PROGRAMNAME,
    MUIA_Application_Version, (ULONG)VersionTag,
    MUIA_Application_SingleTask, TRUE,
    MUIA_Application_Window, (Win = MUI_NewObject(MUIC_Window,
      MUIA_Window_ID, MAKE_ID('S', 'L', 'S', 'B'),
      MUIA_Window_Title, (ULONG)PROGRAMNAME,
      MUIA_Window_Height, MIN(500, MUIV_Window_Height_Screen(80)),
      MUIA_Window_Width, 270,
      MUIA_Window_RootObject, MUI_NewObject(MUIC_Group,
        MUIA_Group_Child, MUI_NewObject(MUIC_Group,
          MUIA_Group_Child, MUI_NewObject(MUIC_Group,
            MUIA_Group_Columns, 2,
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Data Structure:",
              MUIA_Text_HiChar, 'd',
            TAG_END),
            MUIA_Group_Child, cyc_Data = MUI_NewObject(MUIC_Cycle,
              MUIA_Cycle_Entries, cyc_DSEntries,
              MUIA_ControlChar, 'd',
              MUIA_ShortHelp, "Type of data structure to sort.\nArray is adjacent items (of size given in Item Size) in memory, each\nholding random integer values themselves (or as their first members).\nList is a double linked list of items which carry 32bit random integer\nvalues as payload.",
            TAG_END),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Array/List Size:",
              MUIA_Text_HiChar, 'a',
            TAG_END),
            MUIA_Group_Child, str_Size = MUI_NewIntegerInputBox(1000, TRUE, "Length of the array/list to be sorted.", 'a'),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Value Range:",
              MUIA_Text_HiChar, 'v',
            TAG_END),
            MUIA_Group_Child, str_Range = MUI_NewIntegerInputBox(255, TRUE, "The range of the values stored in each element of\nthe array will be between 0 and this value.", 'v'),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Item Size:",
              MUIA_Text_HiChar, 'i',
            TAG_END),
            MUIA_Group_Child, str_ItemSize = MUI_NewIntegerInputBox(4, TRUE, "The size of each element of the array in bytes.\nTo store a 32bit (LONG/int32_t) value it must be at least 4.\nTo store a 16bit (WORD/int16_t) value it must be at least 2.", 'i'),
          TAG_END),
          MUIA_Group_Child, chk_Forbid = MUI_NewCheckMark(FALSE, "Turn multitasking off during sort", 't',
            "All sort operations will be done in Forbid() state.\nThis will cause GUI to freeze during benchmark\nbut the results will be more accurate."),
        TAG_END),
        MUIA_Group_Child, btn_Start = MUI_NewKeyButton("\033cStart Benchmark", 's'),
        MUIA_Group_Child, MUI_NewObject(MUIC_Group,
          MUIA_Group_Horiz, TRUE,
          MUIA_Group_Child, dsp_State =  MUI_NewObject(MUIC_Text,
            MUIA_Text_Contents, "Idle",
            MUIA_Frame, MUIV_Frame_ReadList,
          TAG_END),
          MUIA_Group_Child, (gau_Progress = MUI_NewObject(MUIC_Gauge,
            MUIA_Gauge_InfoText, "%ld%",
            MUIA_Gauge_Current, 0,
            MUIA_Gauge_Horiz, TRUE,
            MUIA_Frame, MUIV_Frame_Gauge,
          TAG_END)),
        TAG_END),
        MUIA_Group_Child, MUI_NewObject(MUIC_Group,
          MUIA_FrameTitle, "Results",
          MUIA_Frame, MUIV_Frame_Group,
          MUIA_Group_Child, MUI_NewObject(MUIC_Group,
            MUIA_Group_Columns, 2,
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Selection Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Select] = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Insertion Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Insert] = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Shell Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Shell]  = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Bubble Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Bubble] = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Quick Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Quick]  = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Merge Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Merge]  = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Counting Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Count]  = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Radix Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Radix]  = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
            MUIA_Group_Child, MUI_NewObject(MUIC_Text,
              MUIA_Text_Contents, "Heap Sort:",
            TAG_END),
            MUIA_Group_Child, dsp[_Heap]   = MUI_NewDisplayBox(str_NA, dsp_AlgHelp),
          TAG_END),
        TAG_END),
      TAG_END),
    TAG_END)),
  TAG_END);

  if (App)
  {
    //Set tab key gadget cycle chain
    DoMethod(Win, MUIM_Window_SetCycleChain, btn_Start,
                                             cyc_Data,
                                             str_Size,
                                             str_Range,
                                             str_ItemSize,
                                             getGroupChild(chk_Forbid, 1),
                                             NULL);
    DoMethod(Win, MUIM_Set, MUIA_Window_ActiveObject, btn_Start);

    //Notifications
    DoMethod(App, MUIM_Notify, MUIA_Application_DoubleStart, TRUE, App, 2,
      MUIM_Application_ReturnID, MUIV_Application_ReturnID_DoubleStart);

    DoMethod(Win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, App, 2,
      MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

    DoMethod(btn_Start, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
      MUIM_Application_ReturnID, MUIV_Application_ReturnID_SortBench);

    DoMethod(str_Range, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
             str_ItemSize, 3, MUIM_CallHook, &str_Range_hook, MUIV_TriggerValue);

    DoMethod(str_ItemSize, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime,
             str_Range, 3, MUIM_CallHook, &str_ItemSize_hook, MUIV_TriggerValue);

    DoMethod(str_ItemSize, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
             str_ItemSize, 3, MUIM_CallHook, &str_ItemSize_Acknowledge_hook, MUIV_TriggerValue);

    DoMethod(cyc_Data, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
             str_ItemSize, 3, MUIM_CallHook, &cyc_Data_hook, MUIV_TriggerValue);

//  DoMethod(btn_Quit, MUIM_Notify, MUIA_Pressed, FALSE, App, 2,
//    MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
  }

  return App;
}
///
///main
/***********************************************
 * Developer level main                        *
 * - Code your program here.                   *
 ***********************************************/
int Main(struct Config *config)
{
  int rc = 0;

  MUIMasterBase = OpenLibrary("muimaster.library", 0);
  if (MUIMasterBase)
  {
#if defined(__amigaos4__)
		if (IMUIMaster = (struct MUIMasterIFace *)GetInterface(MUIMasterBase, "main", 1, NULL))
    {
#endif
      SortBase = OpenLibrary("sort.library", 0);
      if (SortBase)
      {
#if defined(__amigaos4__)
        if (ISort = (struct SortIFace *)GetInterface(SortBase, "main", 1, NULL))
        {
#endif
          if (buildGUI())
          {
            ULONG signals = 0;
            BOOL running = TRUE;

            set(Win, MUIA_Window_Open, TRUE);

            while(running)
            {
              ULONG id = DoMethod (App, MUIM_Application_NewInput, &signals);
              switch(id)
              {
                case MUIV_Application_ReturnID_DoubleStart:
                  set(Win, MUIA_Window_Activate, TRUE);
                  DoMethod(Win, MUIM_Window_ToFront);
                break;
                case MUIV_Application_ReturnID_SortBench:
                  sortBench();
                break;
                case MUIV_Application_ReturnID_Quit:
                  running = FALSE;
                break;
              }
              if(running && signals) signals = Wait(signals | SIGBREAKF_CTRL_C);
              if (signals & SIGBREAKF_CTRL_C) break;
            }

            set(Win, MUIA_Window_Open, FALSE);

            MUI_DisposeObject(App);
          }
          else
          {
            rc = 20;
            puts("Could not create MUI application object!");
          }
#if defined(__amigaos4__)
          DropInterface((struct Interface *)ISort);
        }
        else rc = 20;
#endif
        CloseLibrary(SortBase);
      }
      else
      {
        rc = 20;
        puts("Could not open sort.library!");
      }
#if defined(__amigaos4__)
      DropInterface((struct Interface *)IMUIMaster);
    }
    else rc = 20;
#endif
    CloseLibrary(MUIMasterBase);
  }
  else
  {
    rc = 20;
    puts("Could not open muimaster.library!");
  }

  return(rc);
}
///
///cleanup
/***********************************************
 * Clean up before exit                        *
 * - Free allocated resources here.            *
 ***********************************************/
void CleanUp(struct Config *config)
{
  if (config)
  {
    CloseTimer(timeReq);

    // free command line arguments
    #if RDARGS_OPTIONS
      if (config->RDArgs)
        FreeArgs(config->RDArgs);
    #endif

    FreeMem(config, sizeof(struct Config));
  }
}
///

/******************
 * Hash Functions *
 ******************/
/// myByteHash(ptr)
ULONG myByteHash(APTR p, UNUSED ULONG digit)
{
  UBYTE val = *((UBYTE*)p);
  return (ULONG)val;
}
///
/// myByteRadixHash(ptr, digit)
ULONG myByteRadixHash(APTR p, ULONG digit)
{
  UBYTE val = *((UBYTE*)p);
  ULONG i;
  for (i = 0; i < digit; i++) val /= 10;
  return (ULONG)(val%10);
}
///
/// myWordHash(ptr)
ULONG myWordHash(APTR p, UNUSED ULONG digit)
{
  UWORD val = *((UWORD*)p);
  return (ULONG)val;
}
///
/// myWordRadixHash(ptr, digit)
ULONG myWordRadixHash(APTR p, ULONG digit)
{
  UWORD val = *((UWORD*)p);
  ULONG i;
  for (i = 0; i < digit; i++) val /= 10;
  return (ULONG)(val%10);
}
///
/// myListHash(ptr)
/*****************************************************
 * Hash function for list sorting with Counting Sort *
 *****************************************************/
ULONG myListHash(APTR p, UNUSED ULONG digit)
{
  struct MyItem *n = (struct MyItem*)p;
  LONG v = n->val;
  return (ULONG)v;
}
///
/// myListRadixHash(ptr, digit)
/**************************************************
 * Hash function for list sorting with Radix Sort *
 **************************************************/
ULONG myListRadixHash(APTR p, ULONG digit)
{
  struct MyItem* n = (struct MyItem*)p;
  LONG v = n->val;
  ULONG i;
  for (i = 0; i < digit; i++) v /= 10;
  return (ULONG)(v%10);
}
///

/*********************
 * Compare Functions *
 *********************/
 /// myByteCompare(ptr1, ptr2)
 LONG myByteCompare(APTR p1, APTR p2)
 {
   UBYTE val1 = *((UBYTE*)p1);
   UBYTE val2 = *((UBYTE*)p2);

   if (val1 == val2) return  0;
   if (val1 <  val2) return -1;
   else return 1;
 }
 ///
 /// myWordCompare(ptr1, ptr2)
 LONG myWordCompare(APTR p1, APTR p2)
 {
   UWORD val1 = *((UWORD*)p1);
   UWORD val2 = *((UWORD*)p2);

   if (val1 == val2) return  0;
   if (val1 <  val2) return -1;
   else return 1;
 }
 ///
 /// myListCompare(ptr1, ptr2)
/******************************************************************************
 * Compares the two given MyItem nodes against their priority val members.    *
 ******************************************************************************/
LONG myListCompare(APTR p1, APTR p2)
{
  struct MyItem *n1 = (struct MyItem*) p1;
  struct MyItem *n2 = (struct MyItem*) p2;

  if (n1->val == n2->val) return  0;
  if (n1->val <  n2->val) return -1;
  else return 1;
}
///

/// sortBench()
VOID sortBench()
{
  ULONG isList;
  ULONG size;
  ULONG itemSize;
  ULONG range;
  APTR array;
  ULONG forbid;
  ULONG best = _Select;
  STRPTR str;
  UBYTE strBuf[32]; // Buffer for display strings
  ULONG i;

  gauge_counter = 0;
  update_gauge();
  DoMethod(dsp_State, MUIM_Set, MUIA_Text_Contents, "Initializing benchmark");
  DoMethod(btn_Start, MUIM_Set, MUIA_Disabled, TRUE);

  //get values from GUI elements
  GetAttr(MUIA_Cycle_Active, cyc_Data, &isList);
  GetAttr(MUIA_String_Integer, str_Size, &size);
  if (isList)
    itemSize = sizeof(struct MyItem);
  else
    GetAttr(MUIA_String_Integer, str_ItemSize, &itemSize);
  GetAttr(MUIA_String_Integer, str_Range, &range);
  GetAttr(MUIA_Selected, chk_Forbid, &forbid);

  //clear GUI displays
  for (i = _Select; i < numAlgorithms; i++) {
    DoMethod(dsp[i], MUIM_Set, MUIA_Text_Contents, str_NA);
  }

  if (!itemSize) itemSize = 4;
  if (size)
  {
    ULONG arraySize = size * itemSize;

    array = AllocMem(arraySize, MEMF_ANY);
    if (array)
    {
      ULONG seed;
      struct timeval timeVal;
      struct timeval timeVal2;
      struct timeval shortest;
      struct MinList list;
      ULONG err;
      ULONG digits;
      struct Range rng;
      rng.min = 0;
      rng.max = range;
      shortest.tv_secs  = ULONG_MAX;
      shortest.tv_micro = ULONG_MAX;

      digits = countDigits(range);

      // create a randomization seed from system timer
      GetSysTime(&timeVal);
      seed = timeVal.tv_secs ^ timeVal.tv_micro;

      for (i = _Select; i < numAlgorithms; i++) {
        err = 0;
        DoMethod(dsp_State, MUIM_Set, MUIA_Text_Contents, isList ? "Creating list" : "Creating array");
        update_gauge();
        if (isList)
          fillListRnd(&list, array, size, range, seed);
        else
          fillArrayRnd(array, size, range, itemSize, seed);

        sprintf(strBuf, "Doing %s Sort", algName[i-1]);
        DoMethod(dsp_State, MUIM_Set, MUIA_Text_Contents, strBuf);
        update_gauge();

        // get elapsed times during sort
        if (isList) {
          if (forbid) Forbid();
          GetSysTime(&timeVal);
          err = Sort(&list, SORT_List, SORT_Algorithm, i,
            SORT_Range, i == _Count ? &rng : NULL,
            SORT_Digits, digits,
            SORT_HashFnc, i == _Count ? myListHash : myListRadixHash,
            SORT_CompareFnc, myListCompare,
            TAG_END);
          GetSysTime(&timeVal2);
          if (forbid) Permit();
        }
        else {
          if (forbid) Forbid();
          GetSysTime(&timeVal);
          err = Sort(array, size, SORT_Algorithm, i,
            SORT_Range, i == _Count ? &rng : NULL,
            SORT_Digits, digits,
            SORT_HashFnc, i == _Count ? (itemSize < 2 ? myByteHash : (itemSize < 4 ? myWordHash : NULL)) : (itemSize < 2 ? myByteRadixHash : (itemSize < 4 ? myWordRadixHash : NULL)),
            SORT_CompareFnc, itemSize < 2 ? myByteCompare : (itemSize < 4 ? myWordCompare : NULL),
            TAG_END);
          GetSysTime(&timeVal2);
          if (forbid) Permit();
        }

        if (err) {
          if (err == SORT_ERR_MEMORY)
            MUI_Request(App, Win, 0, "Error!", "*_OK", "Not enough memory to apply %s Sort.", algName[i-1]);
          continue;
        }

        SubTime(&timeVal2, &timeVal);
        // Memorize the algorithm with the shortest time
        if(CmpTime(&timeVal2, &shortest) == 1) {
          shortest.tv_secs  = timeVal2.tv_secs;
          shortest.tv_micro = timeVal2.tv_micro;
          best = i;
        }
        sprintf(strBuf, "%ld.%06lu secs", timeVal2.tv_secs, timeVal2.tv_micro);

        //update GUI display
        DoMethod(dsp[i], MUIM_Set, MUIA_Text_Contents, strBuf);
      }

      FreeMem(array, arraySize);
    }
    else
    {
      gauge_counter = -1;
      MUI_Request(App, Win, 0, "Error!", "*_OK", "Not enough memory to create %s.", isList ? "list" : "array");
    }
  }
  else gauge_counter = -1;
  update_gauge();
  // Mark the algorithm with best time with bold letters
  GetAttr(MUIA_Text_Contents, dsp[best], &str);
  strcpy(strBuf, "\33b");
  strcat(strBuf, str);
  DoMethod(dsp[best], MUIM_Set, MUIA_Text_Contents, strBuf);
  DoMethod(dsp_State, MUIM_Set, MUIA_Text_Contents, "Idle");
  DoMethod(btn_Start, MUIM_Set, MUIA_Disabled, FALSE);
}
///

/***********
 * Utility *
 ***********/
/// FastRand(seed)
#ifdef __amigaos4__
  #define FastRand(seed) ((seed << 1) ^ 0x1D872B41)
#endif
///
/// fillArrayRnd(arr, size, range, itemSize, seed)
/******************************************************************************
 * Fills the given memory as an array of random values. Values will be between*
 * 0 and range. It will create the exact same array if given the same seed.   *
 ******************************************************************************/
VOID fillArrayRnd(APTR arr, ULONG size, ULONG range, ULONG itemSize, ULONG seed)
{
  APTR a;
  ULONG val = seed;
  range++;

  if (size) {
    if (arr) {
      for (a = arr; a < arr + (size * itemSize); a+=itemSize) {
        val = FastRand(val);
        if (itemSize < 2)
          *((UBYTE*)a) = (UBYTE)(val & range);
        else if (itemSize < 4)
          *((UWORD*)a) = (UWORD)(val & range);
        else
          *((ULONG*)a) = val % range;
      }
    }
  }
}
///
/// fillListRnd(list, mem, size, range, seed)
/******************************************************************************
 * Fills the given memory as a double linked list of random values. Values    *
 * will be between 0 and range. It will create the exact same list if given   *
 * the same seed. The items will be adjacent in memory just like an array so  *
 * we will be able to allocate the total memory used by the list items        *
 * beforehand.                                                                *
 ******************************************************************************/
VOID fillListRnd(struct MinList *list, APTR mem, ULONG size, ULONG range, ULONG seed)
{
  APTR a;
  struct MyItem *item;
  ULONG val = seed;
  ULONG itemSize = sizeof(struct MyItem);
  range++;

  NewMinList(list);

  if (size) {
    if (mem) {
      for (item = (struct MyItem*)mem; item < mem + (size * itemSize); item++) {
        val = FastRand(val);
        item->val = val % range;
        AddMinTail(list, &item->node);
      }
    }
  }
}
///
/// countDigits(val)
/************************************************************
 * Returns the number of decimal digits the given value has *
 ************************************************************/
ULONG countDigits(ULONG val)
{
  ULONG result = 1;
  while (val/=10) result++;
  return result;
}
///

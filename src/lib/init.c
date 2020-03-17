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
  #include <emul/emulinterface.h>
  #include <emul/emulregs.h>
#endif

#include <exec/exec.h>
#include <proto/exec.h>
#include <dos/dos.h>

#include <SDI_lib.h>
#include "SDI_macros.h"

#include <proto/sort.h>

#include "revision.h"
#include "sortbase.h"
///
///definitions
#ifndef __amigaos4__
  #define DeleteLibrary(LIB) \
    FreeMem((STRPTR)(LIB)-(LIB)->lib_NegSize, (ULONG)((LIB)->lib_NegSize+(LIB)->lib_PosSize))
#endif
///
///globals
#ifdef __GNUC__
  volatile STATIC CONST UBYTE USED_VAR __attribute__((section(".text"))) VersionTag[] = VERSTAG;
  volatile STATIC CONST UBYTE USED_VAR __attribute__((section(".text"))) vstring[] = VSTRING;
  volatile STATIC CONST UBYTE USED_VAR __attribute__((section(".text"))) libname[] = LIBNAME;
#else
  CONST UBYTE USED_VAR VersionTag[] = VERSTAG;
  CONST UBYTE USED_VAR vstring[] = VSTRING;
  CONST UBYTE USED_VAR libname[] = LIBNAME;
#endif

#ifdef __MORPHOS__
/******************************************************************************
 * Inform the loader that this is an emulppc elf and not a ppc.library one.   *
 ******************************************************************************/
  const USED_VAR ULONG __amigappc__ = 1;
  const USED_VAR ULONG __abox__ = 1;
#endif

#ifdef __amigaos4__
struct ExecIFace *IExec;
#else
struct ExecBase *SysBase;
#endif

#ifdef __amigaos4__
  struct Library *UtilityBase = NULL;
  struct UtilityIFace *IUtility = NULL;
#elif __MORPHOS__
  struct Library *UtilityBase = NULL;
#else
  struct UtilityBase *UtilityBase = NULL;
#endif
///
///entry
// If a user tries to execute this binary it should return safely
#ifdef __amigaos4__
  int32 _start( void )
  {
	   return RETURN_FAIL;
  }
#else
  STATIC CONST UWORD __rts = 0x4E75;
#endif
///

/*******************************************************************************
 * Standard Library Functions                                                  *
 ******************************************************************************/
///libProtos
#ifdef __amigaos4__
  STATIC LIBFUNC struct SortBase *libInit(struct SortBase *, BPTR, struct ExecIFace *);
  STATIC LIBFUNC BPTR libExpunge(struct LibraryManagerInterface *);
  STATIC LIBFUNC struct SortBase *libOpen(struct LibraryManagerInterface *, ULONG);
  STATIC LIBFUNC BPTR libClose(struct LibraryManagerInterface *);
  STATIC LIBFUNC ULONG libObtain(struct LibraryManagerInterface *);
  STATIC LIBFUNC ULONG libRelease(struct LibraryManagerInterface *);
#elif __MORPHOS__
  STATIC LIBFUNC struct SortBase *libInit(struct SortBase *, BPTR, struct ExecBase *);
  STATIC LIBFUNC BPTR libExpunge(void);
  STATIC LIBFUNC struct SortBase *libOpen(void);
  STATIC LIBFUNC BPTR libClose(void);
#else
  STATIC LIBFUNC struct SortBase *libInit(REG(a0, BPTR), REG(a6, struct ExecBase *));
  STATIC LIBFUNC BPTR libExpunge(REG(a6, struct SortBase *));
  STATIC LIBFUNC struct SortBase *libOpen(REG(a6, struct SortBase *));
  STATIC LIBFUNC BPTR libClose(REG(a6, struct SortBase *));
#endif
///
///libVectors
#include "vectors.h"
#ifndef libvector
#error libvector is not defined in vectors.h
#endif

#ifndef __amigaos4__
STATIC LONG LIBFUNC libNull(void)
{
  return(0);
}
#endif

static const APTR libVectors[] =
{
  #ifdef __amigaos4__
  (APTR)libObtain,
  (APTR)libRelease,
  (APTR)NULL,
  (APTR)NULL,
  #else
  #ifdef __MORPHOS__
  (APTR)FUNCARRAY_32BIT_NATIVE,
  #endif
  (APTR)libOpen,
  (APTR)libClose,
  (APTR)libExpunge,
  (APTR)libNull,
  #endif
  libvector,
  (APTR)-1
};
///
///libInit
#ifdef __amigaos4__
  STATIC LIBFUNC struct SortBase *libInit(struct SortBase *base, BPTR seglist, struct ExecIFace *exec)
  {
    IExec = exec;
#elif __MORPHOS__
  STATIC LIBFUNC struct SortBase *libInit(struct SortBase *base, BPTR seglist, struct ExecBase *sysbase)
  {
    SysBase = sysbase;
#else
  STATIC LIBFUNC struct SortBase *libInit(REG(a0, BPTR seglist), REG(a6, struct ExecBase *sysbase))
  {
    struct SortBase *base;
    SysBase = sysbase;
    if((base = (struct SortBase *)MakeLibrary((APTR)libVectors, NULL, NULL, sizeof(struct SortBase), NULL)))
    {
#endif
      base->libNode.lib_Node.ln_Type = NT_LIBRARY;
      base->libNode.lib_Node.ln_Pri  = 0;
      base->libNode.lib_Node.ln_Name = libname;
      base->libNode.lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
      base->libNode.lib_Version      = VERSION;
      base->libNode.lib_Revision     = REVISION;
      base->libNode.lib_IdString     = vstring;

      base->segList = seglist;

      #ifdef USE_SEMAPHORE
        InitSemaphore(&base->libSemaphore);
      #endif

      // Add your additional init code here
      UtilityBase = (struct UtilityBase*)OpenLibrary("utility.library", 0);
#ifdef __amigaos4__
      IUtility = (struct UtilityIFace *)GetInterface(UtilityBase, "main", 1, NULL);
#endif

#if !defined(__amigaos4__) && !defined(__MORPHOS__)
      AddLibrary((struct Library *)base);
    }
#endif

    return base;
  }
///
///libExpunge
#ifdef __amigaos4__
  STATIC LIBFUNC BPTR libExpunge(struct LibraryManagerInterface *Self)
  {
    struct SortBase *base = (struct SortBase *)Self->Data.LibBase;
#elif __MORPHOS__
  STATIC LIBFUNC BPTR libExpunge(void)
  {
    struct SortBase *base = (void *)REG_A6;
#else
  STATIC LIBFUNC BPTR libExpunge(REG(a6, struct SortBase *base))
  {
#endif
    BPTR result;

    if (base->libNode.lib_OpenCnt == 0)
    {
      result = base->segList;

      // Undo what your additional init code did
#ifdef __amigaos4__
      if (IUtility) DropInterface((struct Interface *)IUtility);
#endif
      if (UtilityBase) CloseLibrary((struct Library *)UtilityBase);

      Remove((struct Node *)base);
      DeleteLibrary((struct Library *)base);
    }
    else
    {
      result = (BPTR)0;
      base->libNode.lib_Flags |= LIBF_DELEXP;
    }

    return(result);
  }
///
///libOpen
#ifdef __amigaos4__
  STATIC LIBFUNC struct SortBase *libOpen(struct LibraryManagerInterface *Self, ULONG version)
  {
    struct SortBase *base = (struct SortBase *)Self->Data.LibBase;
    if (version > VERSION) return NULL;
#elif __MORPHOS__
  STATIC LIBFUNC struct SortBase *libOpen(void)
  {
    struct SortBase *base = (void *)REG_A6;
#else
  STATIC LIBFUNC struct SortBase *libOpen(REG(a6, struct SortBase *base))
  {
#endif

    // Cancel a possible "delayed expunge"
    base->libNode.lib_Flags &= ~LIBF_DELEXP;

    #ifdef USE_SEMAPHORE
      ObtainSemaphore(&base->libSemaphore);
    #endif

    /* Add any specific open code here
       Return NULL before incrementing OpenCnt to fail opening */

    // Increment the open count
    base->libNode.lib_OpenCnt++;

    #ifdef USE_SEMAPHORE
      ReleaseSemaphore(&base->libSemaphore);
    #endif

    return (struct SortBase *)base;
  }
///
///libClose
#ifdef __amigaos4__
  STATIC LIBFUNC BPTR libClose(struct LibraryManagerInterface *Self)
  {
    struct SortBase *base = (struct SortBase *)Self->Data.LibBase;
#elif __MORPHOS__
  STATIC LIBFUNC BPTR libClose(void)
  {
    struct SortBase *base = (struct SortBase *)REG_A6;
#else
  STATIC LIBFUNC BPTR libClose(REG(a6, struct SortBase *base))
  {
#endif
    BPTR result = 0;

    #ifdef USE_SEMAPHORE
      ObtainSemaphore(&base->libSemaphore);
    #endif

    // Decrement the open count
    base->libNode.lib_OpenCnt--;

    #ifdef USE_SEMAPHORE
      ReleaseSemaphore(&base->libSemaphore);
    #endif

    if(base->libNode.lib_OpenCnt == 0 &&
       base->libNode.lib_Flags & LIBF_DELEXP)
    {
      #ifdef __amigaos4__
        result = libExpunge(Self);
      #elif __MORPHOS__
        result = libExpunge();
      #else
        result = libExpunge(base);
      #endif
    }

    return(result);
  }
///
///libManager
#ifdef __amigaos4__
  STATIC LIBFUNC ULONG libObtain(struct LibraryManagerInterface *Self)
  {
     return(Self->Data.RefCount++);
  }

  STATIC LIBFUNC ULONG libRelease(struct LibraryManagerInterface *Self)
  {
     return(Self->Data.RefCount--);
  }

  STATIC CONST APTR libManagerVectors[] =
  {
  	libObtain,
  	libRelease,
  	NULL,
  	NULL,
  	libOpen,
  	libClose,
  	libExpunge,
  	NULL,
  	(APTR)-1
  };

  STATIC CONST struct TagItem libManagerTags[] =
  {
  	{ MIT_Name,			    (Tag)"__library"       },
  	{ MIT_VectorTable,	(Tag)libManagerVectors },
  	{ MIT_Version,		  1                      },
  	{ TAG_DONE,			    0                      }
  };
#endif
///
///libInterfaces
#ifdef __amigaos4__
  STATIC CONST struct TagItem mainTags[] =
  {
     {MIT_Name,        (ULONG)"main"},
     {MIT_VectorTable, (ULONG)libVectors},
     {MIT_Version,     1},
     {TAG_DONE,        0}
  };

  STATIC CONST ULONG libInterfaces[] =
  {
     (ULONG)libManagerTags,
     (ULONG)mainTags,
     (ULONG)0
  };

  #ifndef NO_VECTABLE68K
  extern CONST APTR VecTable68K[];
  #endif

  STATIC CONST struct TagItem libCreateTags[] =
  {
  	{ CLT_DataSize,		sizeof(struct SortBase)},
  	{ CLT_InitFunc,		(Tag)libInit			      },
  	{ CLT_Interfaces,	(Tag)libInterfaces		  },
    #ifndef NO_VECTABLE68K
  	{ CLT_Vector68K, (Tag)VecTable68K },
    #endif
  	{TAG_DONE,		 0 }
  };
#endif
///
///ROMTAG
STATIC CONST USED_VAR struct Resident ROMTag =
{
  RTC_MATCHWORD,
  (struct Resident *)&ROMTag,
  ((APTR)&ROMTag) + sizeof(struct Resident),            // EndCode
  #if defined(__amigaos4__)
  RTF_AUTOINIT|RTF_NATIVE,      // Add RTF_COLDSTART to be reset resident
  #elif defined(__MORPHOS__)
  RTF_PPC,
  #else
  0,
  #endif
  VERSION,
  NT_LIBRARY,
  0,                             // PRIORITY not needed unless reset resident
  (char *)libname,
  (char *)vstring,
  #if defined(__amigaos4__)
  (APTR)libCreateTags,
  #else
  (APTR)libInit,
  #endif
  #if defined(__MORPHOS__)
  REVISION,
  0
  #endif
};
///

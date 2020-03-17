// This file is a part of Sort Library

#ifdef __USE_INLINE__
#undef __USE_INLINE__
#endif
#ifndef __NOGLOBALIFACE__
#define __NOGLOBALIFACE__
#endif

#include <exec/interfaces.h>
#include <exec/libraries.h>
#include <exec/emulation.h>
#include <interfaces/exec.h>
#include <interfaces/sort.h>
#include <proto/sort.h>

STATIC struct Library * stub_OpenPPC(ULONG *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((ULONG)Base + Base->lib_PosSize);
	struct LibraryManagerInterface *Self = (struct LibraryManagerInterface *) ExtLib->ILibrary;

	return Self->Open(0);
}
STATIC CONST struct EmuTrap stub_Open = { TRAPINST, TRAPTYPE, (ULONG (*)(ULONG *))stub_OpenPPC };

STATIC APTR stub_ClosePPC(ULONG *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((ULONG)Base + Base->lib_PosSize);
	struct LibraryManagerInterface *Self = (struct LibraryManagerInterface *) ExtLib->ILibrary;

	return Self->Close();
}
STATIC CONST struct EmuTrap stub_Close = { TRAPINST, TRAPTYPE, (ULONG (*)(ULONG *))stub_ClosePPC };

STATIC APTR stub_ExpungePPC(ULONG *regarray __attribute__((unused)))
{
	return NULL;
}
STATIC CONST struct EmuTrap stub_Expunge = { TRAPINST, TRAPTYPE, (ULONG (*)(ULONG *))stub_ExpungePPC };

STATIC ULONG stub_ReservedPPC(ULONG *regarray __attribute__((unused)))
{
	return 0UL;
}
STATIC CONST struct EmuTrap stub_Reserved = { TRAPINST, TRAPTYPE, stub_ReservedPPC };

STATIC ULONG stub_SortAPPC(ULONG *regarray)
{
	struct Library *Base = (struct Library *) regarray[REG68K_A6/4];
	struct ExtendedLibrary *ExtLib = (struct ExtendedLibrary *) ((ULONG)Base + Base->lib_PosSize);
	struct SortIFace *Self = (struct SortIFace *) ExtLib->MainIFace;

	return Self->SortA(
		(APTR)regarray[REG68K_A0/4],
		(ULONG)regarray[REG68K_D0/4],
		(struct TagItem *)regarray[REG68K_A1/4]
	);
}
STATIC CONST struct EmuTrap stub_SortA = { TRAPINST, TRAPTYPE, (ULONG (*)(ULONG *))stub_SortAPPC };

CONST CONST_APTR VecTable68K[] =
{	&stub_Open,
	&stub_Close,
	&stub_Expunge,
	&stub_Reserved,
	&stub_SortA,
	(CONST_APTR)-1
};

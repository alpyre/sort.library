#ifndef PRAGMAS_SORT_PRAGMAS_H
#define PRAGMAS_SORT_PRAGMAS_H

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

#if defined(LATTICE) || defined(__SASC) || defined(_DCC)
  #ifndef __CLIB_PRAGMA_LIBCALL
    #define __CLIB_PRAGMA_LIBCALL
  #endif
#else /* __MAXON__, __STORM__ or AZTEC_C */
  #ifndef __CLIB_PRAGMA_AMICALL
    #define __CLIB_PRAGMA_AMICALL
  #endif
#endif

#if defined(__SASC_60) || defined(__STORM__)
  #ifndef __CLIB_PRAGMA_TAGCALL
    #define __CLIB_PRAGMA_TAGCALL
  #endif
#endif

#ifdef __CLIB_PRAGMA_LIBCALL
	#pragma libcall SortBase SortA 1e 90803
/*#ifdef __CLIB_PRAGMA_TAGCALL
		#pragma tagcall SortBase Sort 24 90803
	#endif */
#endif /* __CLIB_PRAGMA_LIBCALL */

#ifdef __CLIB_PRAGMA_AMICALL
	#pragma amicall(SortBase, 0x1e, SortA(a0,d0,a1))
/*#ifdef __CLIB_PRAGMA_TAGCALL
		#pragma tagcall(SortBase, 0x24, Sort(a0,d0,a1))
	#endif */
#endif /* __CLIB_PRAGMA_AMICALL */

#endif /* PRAGMAS_SORT_PRAGMAS_H */

#ifndef PROTO_SORT_H
#define PROTO_SORT_H

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

#include <exec/types.h>
#include <dos/dos.h>

/****************************************************************************/

#ifndef __NOLIBBASE__
 extern struct Library * SortBase;
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/sort.h>
 #ifdef __USE_INLINE__
  #include <inline4/sort.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_SORT_PROTOS_H
  #define CLIB_SORT_PROTOS_H 1
 #endif /* CLIB_SORT_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct SortIFace *ISort;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_SORT_PROTOS_H
  #include <clib/sort_protos.h>
 #endif /* CLIB_SORT_PROTOS_H */
 #if defined(__GNUC__)
  #ifdef __AROS__
   #include <defines/sort.h>
  #else
   #ifndef __PPC__
    #include <inline/sort.h>
   #else /* __PPC__ */
    #include <ppcinline/sort.h>
   #endif /* __PPC__ */
  #endif /* __AROS__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/sort_protos.h>
  #endif /* __PPC__ */
 #else /* __GNUC__ */
  #include <pragmas/sort_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_SORT_H */

#ifndef _INLINE4_SORT_H
#define _INLINE4_SORT_H

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

#ifndef _SFDC_VARARG_DEFINED
	#define _SFDC_VARARG_DEFINED
	#ifdef __HAVE_IPTR_ATTR__
		typedef APTR _sfdc_vararg __attribute__((iptr));
	#else
		typedef ULONG _sfdc_vararg;
	#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/interfaces.h>
#include <interfaces/sort.h>

#define SortA(array, size, tagList) ISort->SortA((array), (size), (tagList))
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || (__GNUC__ >= 3)
#define Sort(array, size, ...) ({_sfdc_vararg args[] = {__VA_ARGS__}; SortA((array), (size), args);})
#elif (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define Sort(array, size, ...) ({_sfdc_vararg args[] = {## vargs}; SortA((array), (size), args);})
#endif

#endif /* _INLINE4_SORT_H */

#ifndef _INLINE_SORT_H
#define _INLINE_SORT_H

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

#include <inline/macros.h>

#ifndef SORT_BASE_NAME
#define SORT_BASE_NAME SortBase
#endif

#define SortA(array, size, tagList) \
	LP3(0x1e, ULONG, SortA, APTR, array, a0, ULONG, size, d0, struct TagItem *, tagList, a1, \
	, SORT_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define Sort(array, size, ...) \
	({_sfdc_vararg _args[] = {__VA_ARGS__}; SortA((array), (size), (const APTR) _args); })
#endif /* NO_INLINE_STDARG */

#endif /* _INLINE_SORT_H */

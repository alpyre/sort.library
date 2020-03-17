#ifndef CLIB_SORT_PROTOS_H
#define CLIB_SORT_PROTOS_H

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

ULONG SortA(APTR array, ULONG size, struct TagItem * tagList);
// ULONG Sort(APTR array, ULONG size, ULONG tag1, ...);

#endif /* CLIB_SORT_PROTOS_H */

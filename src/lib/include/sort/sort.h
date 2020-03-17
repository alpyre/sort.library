#ifndef SORT_SORT_H
#define SORT_SORT_H

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

#include <proto/sort.h>
#include <limits.h>

//Public structures
struct Range
{
  LONG min;
  LONG max;
};

//Sort library identifiers
#define SORT_List       ULONG_MAX
#define SORT_Array      TAG_USER + 1

//Sort library tag values
#define SORT_Algorithm  TAG_USER + 2
#define SORT_ListSize   TAG_USER + 3
#define SORT_ItemSize   TAG_USER + 4
#define SORT_CompareFnc TAG_USER + 5
#define SORT_HashFnc    TAG_USER + 6
#define SORT_Range      TAG_USER + 7
#define SORT_Digits     TAG_USER + 8
#define SORT_Reverse    TAG_USER + 9
#define SORT_MemPool    TAG_USER + 10

//Sort algorithms
#define SORT_Auto       0
#define SORT_Selection  1
#define SORT_Insertion  2
#define SORT_Shell      3
#define SORT_Bubble     4
#define SORT_Quick      5
#define SORT_Merge      6
#define SORT_Counting   7
#define SORT_Radix      8
#define SORT_Heap       9

//Error codes
#define SORT_ERR_RANGE  10
#define SORT_ERR_DIGITS 11
#define SORT_ERR_MEMORY 20

#endif /* SORT_SORT_H */

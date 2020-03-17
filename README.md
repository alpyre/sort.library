# sort.library
An open source AmigaOS library for various sorting algorithms.

### Implemented sort algorithms:
- Selection Sort
- Insertion Sort
- Shell Sort
- Bubble Sort
- Quick Sort
- Merge Sort
- Counting Sort
- Radix Sort
- Heap Sort Sort

### Supported data structures:
- Arrays
- Lists (double linked, aka. exec lists)

### SortBench
An opensource benchmark tool to compare algorithm performances is also available*

![sortbench](https://s5.gifyu.com/images/SortBench.gif)
<br>(*)requires MUI 3.8</br>

### Example Code
```c
LONG arr[10] = {6, 7, 3, 2, 9, 8, 0, 1, 5, 4};
Sort(arr, 10, TAG_END);
// array is now sorted as {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
```
See doc and examples folder for various working examples

### Legal Information
```
This file is part of Sort Library.
Copyright (C) 2020 İbrahim Alper Sönmez

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
```

### Author
Sort Library is being developed by İbrahim Alper Sönmez

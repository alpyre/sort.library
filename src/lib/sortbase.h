#ifndef _SORT_BASE_H
#define _SORT_BASE_H

// This file is a part of Sort Library

struct SortBase
{
    struct Library libNode;
    BPTR segList;
  #ifdef USE_SEMAPHORE
    struct SignalSemaphore libSemaphore;
  #endif
    // Add your additional data fields here
};

#endif /* _SORT_BASE_H */

/******************************************************************************
 * Open/Close timer device in a more convenient way                           *
 ******************************************************************************/

#ifdef __amigaos4__
 #define __USE_OLD_TIMEVAL__
 #include <proto/timer.h>
#else
  #include <clib/timer_protos.h>
#endif
#include <clib/alib_protos.h>
#include <devices/timer.h>

struct timerequest* OpenTimer(VOID);
VOID CloseTimer(struct timerequest*);

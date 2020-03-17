/******************************************************************************
 * Open/Close timer device in a more convenient way                           *
 ******************************************************************************/

#include <exec/exec.h>
#include <proto/exec.h>
#include "timer.h"

static BYTE error;

/// CloseTimer()
VOID CloseTimer(struct timerequest* timerReq)
{
  struct MsgPort* timerMP = NULL;

  if (!error)
    CloseDevice((struct IORequest*) timerReq);

  if (timerReq) {
    timerMP = timerReq->tr_node.io_Message.mn_ReplyPort;
    DeleteExtIO((struct IORequest*) timerReq);
  }

  if (timerMP)
    DeletePort(timerMP);
}
///
/// OpenTimer()
/*******************************************************************************
 * Creates a timerrequest to get a message from timer.device and sets an alarm *
 * for 2 seconds.                                                              *
 *******************************************************************************/
struct timerequest* OpenTimer(VOID)
{
  struct timerequest* timerReq = NULL;
  struct MsgPort*     timerMP  = NULL;

  // Open the messageport for timer device messages
  timerMP = CreatePort(NULL, 0);
  if (!timerMP) return NULL;

  // Create the message
  timerReq = (struct timerequest*) CreateExtIO(timerMP, sizeof(struct timerequest));
  if (!timerReq)
  {
    DeletePort(timerMP);
    return NULL;
  }

  // Open timer device
  error = OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest*) timerReq, NULL);
  if (error)
  {
    CloseTimer(timerReq);
    return NULL;
  }

  /* Set request for a 2 seconds alarm */
  timerReq->tr_node.io_Command = TR_ADDREQUEST;
  timerReq->tr_time.tv_secs   = 2;
  timerReq->tr_time.tv_micro  = 0;

  return timerReq;
}
///

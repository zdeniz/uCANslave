
#define TimerAlarm        OCR3B
#define TimerCounter      TCNT3
#define min_val(a,b) ((a<b)?a:b)

#include "const.h"
#include "timers.h"

TIMER_HANDLE activeSize = 0;
/*  ---------  The timer table --------- */
s_timer_entry timers[MAX_NB_TIMER] = {
    {TIMER_FREE, NULL, 0, 0},};


static TIMER_HANDLE lastActTimer = 0xFF;


UNS16 systemTick;
//----------------------------------------------------------------------
/**
 * Returns the current value of the millisecond counter (max 32768ms)
 * @return counter value in milliseconds
 */

unsigned int getTime(void) {
    unsigned int time;
    di(); // disable interrupt
    time = systemTick;
    ei(); // Enable interrupt
    return time;
}
//----------------------------------------------------------------------

/**
 * Checks if the time elapsed before an expected event
 * @param waitTime - time of event occurrence in milliseconds (<32768ms)
 * @return 1 - the event has occurred, 0 - the event has not occurred
 */

unsigned char expTime(unsigned int waitTime) {
    unsigned int currentTime;
    di(); // disable interrupt
    currentTime = systemTick;
    ei(); // Enable interrupt
 //   waitTime++;       // ????? why?????
    if (currentTime > waitTime) {
        if (currentTime - waitTime < 0x8000) {
            return 1;
        } else {
            return 0;
        }
    } else {
        if (waitTime - currentTime > 0x8000) {
            return 1;
        } else {
            return 0;
        }
    }
}
//----------------------------------------------------------------------

TIMER_HANDLE DelAlarm(TIMER_HANDLE handle) {
    /* Quick and dirty. system timer will continue to be trigged, but no action will be preformed. */
    //	MSG_WAR(0x3320, "DelAlarm. handle = ", handle);
    if (handle != TIMER_NONE) {
        if (handle == lastActTimer)
            lastActTimer--;
        timers[handle].state = TIMER_FREE;
    }
    return TIMER_NONE;
}
//----------------------------------------------------------------------

TIMER_HANDLE SetAlarm(TimerCallback_t callback, TIMEVAL value, TIMEVAL period) {
    TIMER_HANDLE cnt;
    s_timer_entry *ptr;
    if (callback) /* if something to store */ {

        /* in order to decide new timer setting we have to run over all timer rows */
        for (cnt = 0, ptr = timers; cnt < MAX_NB_TIMER; cnt++, ptr++) {
            if (ptr->state == TIMER_FREE) /* and empty row */ { /* just store */
                UNS8 tmp = (lastActTimer + 1);
                if (cnt == tmp) lastActTimer++;
                ptr->callback = callback;
                ptr->value = value + getTime();
                ptr->interval = period;
                ptr->state = TIMER_ARMED;
                return cnt;
            }
        }
    }
    return TIMER_NONE;
}
int tdcount = 0;
//----------------------------------------------------------------------

void TimeDispatch(void) {
    TIMER_HANDLE i;
    s_timer_entry *ptr;
    TIMEVAL currentTime = getTime();

    for (i = 0, ptr = timers; i <= lastActTimer; i++, ptr++) {

        if (ptr->state & TIMER_ARMED) /* if ptr is active */ {

            if (expTime(ptr->value)) {
                if (ptr->callback)
                    (*ptr->callback)();     // call callback
                if (ptr->interval) /* if simply outdated */ {
                    ptr->value = currentTime + ptr->interval;
                }else
                {
                    // somehow end the timer
                }
            }
        }
    }
}
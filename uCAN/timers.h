 
#ifndef TIMERS_H
#define	TIMERS_H

#include "data.h"

#define MAX_NB_TIMER    8       //количество таймеров в системе
#define TIMEVAL_MAX 0xFFFF

#define TIMER_FREE 0            
#define TIMER_ARMED 1
#define TIMER_TRIG 2
#define TIMER_TRIG_PERIOD 3

#define TIMER_NONE 0xFF



typedef void (*TimerCallback_t)();

struct struct_s_timer_entry {
	UNS8 state;
	TimerCallback_t callback; /* The callback func. */
	TIMEVAL value;
	TIMEVAL interval; /* Periodicity */
};

typedef struct struct_s_timer_entry s_timer_entry;
unsigned int getTime(void);
unsigned char expTime(unsigned int waitTime);
TIMER_HANDLE DelAlarm(TIMER_HANDLE handle);
TIMER_HANDLE SetAlarm(TimerCallback_t callback, TIMEVAL value, TIMEVAL period);
void TimeDispatch(void);


#endif	/* TIMERS_H */


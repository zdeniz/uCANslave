#ifndef PDO_H
#define	PDO_H

#include <xc.h> // include processor files - each processor file is guarded.  

#include "appcfg.h"
#include "const.h"

typedef struct struct_s_PDO_status s_PDO_status;

/* Handler for RxPDO event timers : empty function that user can overload */
void _RxPDO_EventTimers_Handler(UNS32 pdoNum);

/* Status of the TPDO : */
#define PDO_INHIBITED 0x01
#define PDO_RTR_SYNC_READY 0x01

/** The PDO structure */
/*struct struct_s_PDO_status {
  UNS8 transmit_type_parameter;
  TIMER_HANDLE event_timer;
  TIMER_HANDLE inhibit_timer;
  MESSAGE last_message;
};

#define s_PDO_status_Initializer {0, TIMER_NONE, TIMER_NONE, MESSAGE_Initializer}
*/
/** definitions of the different types of PDOs' transmission
 * 
 * SYNCHRO(n) means that the PDO will be transmited every n SYNC signal.
 */
#define TRANS_EVERY_N_SYNC(n) (n) /*n = 1 to 240 */
#define TRANS_SYNC_ACYCLIC    0    /* Trans after reception of n SYNC. n = 1 to 240 */
#define TRANS_SYNC_MIN        1    /* Trans after reception of n SYNC. n = 1 to 240 */
#define TRANS_SYNC_MAX        240  /* Trans after reception of n SYNC. n = 1 to 240 */
#define TRANS_RTR_SYNC        252  /* Transmission on request */
#define TRANS_RTR             253  /* Transmission on request */
#define TRANS_EVENT_SPECIFIC  254  /* Transmission on event */
#define TRANS_EVENT_PROFILE   255  /* Transmission on event */

void PDOInit(void);

void PDOStop(void);

void proceedPDO(void);



#endif	/* PDO_H */


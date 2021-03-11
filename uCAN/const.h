#ifndef CANCONST_H
#define	CANCONST_H

#include <xc.h> // include processor files - each processor file is guarded.

#define _READY   1
#define _BUSY   0

#define _ERROR  1
#define _OK     0

/*   GENERAL   */
#define CAN_SPEED 2
#define CAN_NODE 0x0F


/** definitions used for object dictionary access. ie SDO Abort codes . (See DS 301 v.4.02 p.48)
 */
#define OD_SUCCESSFUL 	             0x00000000
#define OD_READ_NOT_ALLOWED          0x06010001
#define OD_WRITE_NOT_ALLOWED         0x06010002
#define OD_NO_SUCH_OBJECT            0x06020000
#define OD_NOT_MAPPABLE              0x06040041
#define OD_LENGTH_DATA_INVALID       0x06070010
#define OD_NO_SUCH_SUBINDEX 	     0x06090011
#define OD_VALUE_RANGE_EXCEEDED      0x06090030 /* Value range test result */
#define OD_VALUE_TOO_LOW             0x06090031 /* Value range test result */
#define OD_VALUE_TOO_HIGH            0x06090032 /* Value range test result */
/* Others SDO abort codes 
 */
#define SDOABT_TOGGLE_NOT_ALTERNED   0x05030000
#define SDOABT_TIMED_OUT             0x05040000
#define SDOABT_COOMAD_UNKNOWN        0x05040001   
#define SDOABT_OUT_OF_MEMORY         0x05040005 /* Size data exceed SDO_MAX_LENGTH_TRANSFER */
#define SDOABT_GENERAL_ERROR         0x08000000 /* Error size of SDO message */
#define SDOABT_LOCAL_CTRL_ERROR      0x08000021 


#define EMCY_MAX_ERRORS 4       //size error table Object_1003h
#define MAX_EXPEDIT_SIZE 4      //size pack expedited transfer SDO

/*FUNCTIONAL TYPE*/
#define TIMEVAL UNS16       // time
#define TIMER_HANDLE UNS8   //number handle of timers


/*  CONSTANT    */

/* Should not be modified */
#define REQUEST 1
#define NOT_A_REQUEST 0

/*  Function Codes 
   ---------------
  defined in the canopen DS301 
*/
#define _NMT	   0x0
#define _SYNC       0x1
#define _TIME_STAMP 0x2
#define _PDO1tx     0x3
#define _PDO1rx     0x4
#define _PDO2tx     0x5
#define _PDO2rx     0x6
#define _PDO3tx     0x7
#define _PDO3rx     0x8
#define _PDO4tx     0x9
#define _PDO4rx     0xA
#define _SDOtx      0xB
#define _SDOrx      0xC
#define _HEARTBEAT  0xE         //0xE
//#define _LSS 	   0xF

/* NMT Command Specifier, sent by master to change a slave state */
/* ------------------------------------------------------------- */
/* Should not be modified */
#define NMT_Start_Node              0x01
#define NMT_Stop_Node               0x02
#define NMT_Enter_PreOperational    0x80
#define NMT_Reset_Node              0x81
#define NMT_Reset_Comunication      0x82


/* constantes used in the different state machines */
/* ----------------------------------------------- */
/* Must not be modified */
#define state1  0x01
#define state2  0x02
#define state3  0x03
#define state4  0x04
#define state5  0x05
#define state6  0x06
#define state7  0x07
#define state8  0x08
#define state9  0x09
#define state10 0x0A
#define state11 0x0B

/************************* CONSTANTES **********************************/
/** this are static defined datatypes taken fCODE the canopen standard. They
 *  are located at index 0x0001 to 0x001B. As described in the standard, they
 *  are in the object dictionary for definition purpose only. a device does not
 *  to support all of this datatypes.
 */
#define boolean         0x01
#define int8            0x02
#define int16           0x03
#define int32           0x04
#define uint8           0x05
#define uint16          0x06
#define uint32          0x07
#define real32          0x08
#define visible_string  0x09
#define octet_string    0x0A
#define unicode_string  0x0B
#define time_of_day     0x0C
#define time_difference 0x0D

#define domain          0x0F
#define int24           0x10
#define real64          0x11
#define int40           0x12
#define int48           0x13
#define int56           0x14
#define int64           0x15
#define uint24          0x16

#define uint40          0x18
#define uint48          0x19
#define uint56          0x1A
#define uint64          0x1B

#define pdo_communication_parameter 0x20
#define pdo_mapping                 0x21
#define sdo_parameter               0x22
#define identity                    0x23

/* CanFestival is using 0x24 to 0xFF to define some types containing a 
 value range (See how it works in objdict.c)
 */


/** Each entry of the object dictionary can be READONLY (RO), READ/WRITE (RW),
 *  WRITE-ONLY (WO)
 */
#define RW     0x00  
#define WO     0x01
#define RO     0x02

#define TO_BE_SAVE  0x04
#define DCF_TO_SEND 0x08
#endif	/* CANCONST_H */


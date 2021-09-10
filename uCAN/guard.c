#include "guard.h"
#include "leds.h"
#include "objacces.h"
#include "ucan.h"
#include "data.h"
#include "timers.h"

extern MESSAGE uMessage;
extern CO_Data uData;
extern UNS16 obj1017;

//void ProducerHeartbeatAlarm(void);
//UNS32 OnHeartbeatProducerUpdate(void);
//void heartbeatInit(void);
//void heartbeatStop(void);
//void _heartbeatError(UNS8 heartbeatID);
//void proceedNODE_GUARD(void);
//UNS8 sendHeartBeat(void);
//----------------------------------------------------------------------

UNS8 sendHeartBeat(void) {
    NOP();
    NOP();
    
    uMessage.cmd = _HEARTBEAT;
    uMessage.node = uData.nodeId;
    uMessage.len = 0x01;
    uMessage.rtr = NOT_A_REQUEST;
    uMessage.mdata[0] = uData.nodeState;
    ledBeat();
    return canSend(&uMessage);
};
       
//----------------------------------------------------------------------
void ProducerHeartbeatAlarm(void) {
    if (obj1017) {
        /* Time expired, the heartbeat must be sent immediately
         ** generate the correct node-id: this is done by the offset 1792
         ** (decimal) and additionaly
         ** the node-id of this device.
         */
        sendHeartBeat();
    } else {
        uData.ProducerHeartBeatTimer = DelAlarm(uData.ProducerHeartBeatTimer);
    }
};
// see if the parameters of the index and subindex of the callback () function are needed
// const indextable * unused_indextable, UNS8 unused_bSubindex


//----------------------------------------------------------------------
UNS32 OnHeartbeatProducerUpdate(void);
void heartbeatInit(void) {
    UNS32 errorCode = RegisterSetODentryCallBack(0x1017, 0x00, &OnHeartbeatProducerUpdate);
    if (obj1017) {
        uData.ProducerHeartBeatTimer = SetAlarm(&ProducerHeartbeatAlarm, obj1017, obj1017);
    }
};
//----------------------------------------------------------------------

void heartbeatStop(void) {
    uData.ProducerHeartBeatTimer = DelAlarm(uData.ProducerHeartBeatTimer);
};
//----------------------------------------------------------------------
UNS32 OnHeartbeatProducerUpdate(void) {
    heartbeatStop();
    heartbeatInit();
    return 0;
};
//----------------------------------------------------------------------
void _heartbeatError(UNS8 heartbeatID) {
};

//----------------------------------------------------------------------
void proceedNODE_GUARD(void) {
};


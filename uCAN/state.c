//------------------------------------------------------------------------------

#include "appcfg.h"
#include "const.h"
#include "data.h"
#include "objdict.h"
#include "states.h"
#include "sync.h"
#include "emcy.h"
#include "sdo.h"
#include "pdo.h"
#include "ucan.h"
#include "nmt.h"
#include "leds.h"

void heartbeatInit(void);
void heartbeatStop(void);

void proceedNODE_GUARD(void);

extern CO_Data uData;
extern UNS32 obj1014;
extern MESSAGE uMessage;
void ledNMT(void);
void canDispatch(void) {
    switch (uMessage.cmd) {
        case _SYNC: /* can be a SYNC or a EMCY message */
            if (uMessage.node == 0) /* SYNC */ {
                if (uData.CurrentCommunicationState.csSYNC)
                    proceedSYNC();
            } else /* EMCY */
                if (uData.CurrentCommunicationState.csEmergency)
                proceedEMCY();
            break;
            /* case TIME_STAMP: */
        case _PDO1tx:
        case _PDO1rx:
        case _PDO2tx:
        case _PDO2rx:
        case _PDO3tx:
        case _PDO3rx:
        case _PDO4tx:
        case _PDO4rx:
            if (uData.CurrentCommunicationState.csPDO)
                proceedPDO();
            break;
        case _SDOtx:
        case _SDOrx:
            if (uData.CurrentCommunicationState.csSDO)
                if (proceedSDO()) canSend(&uMessage);
            break;
        case _HEARTBEAT:
            if (uData.CurrentCommunicationState.csLifeGuard)
                proceedNODE_GUARD();
            break;
        case _NMT:
            if (uData.canFlags.iam_a_slave) {
                proceedNMTstateChange();
            }
            break;
    }
}
//------------------------------------------------------------------------------

void switchCommunicationState(s_state_communication newCommunicationState) {
    //      StartOrStop(csSDO, NOP(), resetSDO(d))
    if (newCommunicationState.csSDO && uData.CurrentCommunicationState.csSDO == 0) {
        uData.CurrentCommunicationState.csSDO = 1;
        NOP();
    } else if (!newCommunicationState.csSDO && uData.CurrentCommunicationState.csSDO == 1) {
        uData.CurrentCommunicationState.csSDO = 0;
        resetSDO();
    }
    //      StartOrStop(csSYNC, startSYNC(d), stopSYNC(d))
    if (newCommunicationState.csSYNC && uData.CurrentCommunicationState.csSYNC == 0) {
        uData.CurrentCommunicationState.csSYNC = 1;
        startSYNC();
    } else if (!newCommunicationState.csSYNC && uData.CurrentCommunicationState.csSYNC == 1) {
        uData.CurrentCommunicationState.csSYNC = 0;
        stopSYNC();
    }
    //      StartOrStop(csLifeGuard, lifeGuardInit(d), lifeGuardStop(d))
    if (newCommunicationState.csLifeGuard && uData.CurrentCommunicationState.csLifeGuard == 0) {
        uData.CurrentCommunicationState.csLifeGuard = 1;
        heartbeatInit();
    } else if (!newCommunicationState.csLifeGuard && uData.CurrentCommunicationState.csLifeGuard == 1) {
        uData.CurrentCommunicationState.csLifeGuard = 0;
        heartbeatStop();
    }
    //      StartOrStop(csEmergency, emergencyInit(d), emergencyStop(d))    
    if (newCommunicationState.csEmergency && uData.CurrentCommunicationState.csEmergency == 0) {
        uData.CurrentCommunicationState.csEmergency = 1;
        emergencyInit();
    } else if (!newCommunicationState.csEmergency && uData.CurrentCommunicationState.csEmergency == 1) {
        uData.CurrentCommunicationState.csEmergency = 0;
        emergencyStop();
    }
    //      StartOrStop(csPDO, PDOInit(d), PDOStop(d))
    if (newCommunicationState.csPDO && uData.CurrentCommunicationState.csPDO == 0) {
        uData.CurrentCommunicationState.csPDO = 1;
        PDOInit();\
	                                                                                                                                                } else if (!newCommunicationState.csPDO && uData.CurrentCommunicationState.csPDO == 1) {
        uData.CurrentCommunicationState.csPDO = 0;
        PDOStop();
    }
    //             StartOrStop(csBoot_Up, NOP(), slaveSendBootUp(d))
    if (newCommunicationState.csBoot_Up && uData.CurrentCommunicationState.csBoot_Up == 0) {
        uData.CurrentCommunicationState.csBoot_Up = 1;
        NOP();
    } else if (!newCommunicationState.csBoot_Up && uData.CurrentCommunicationState.csBoot_Up == 1) {
        uData.CurrentCommunicationState.csBoot_Up = 0;
        sendBootUp();
    }
}

//------------------------------------------------------------------------------

UNS8 setState(e_nodeState newState) {
    s_state_communication newCommunicationState;
    if (newState != uData.nodeState) {
        switch (newState) {
            case Initialisation:
            {
                //s_state_communication newCommunicationState = {1, 0, 0, 0, 0, 0, 0};
                newCommunicationState.cServices = 0x01;
                uData.nodeState = Initialisation;
                switchCommunicationState(newCommunicationState);
                /* call user app init callback now. */
                /* d->initialisation MUST NOT CALL SetState */
                initialisation();
            }

                /* Automatic transition - No break statement ! */
                /* Transition from Initialisation to Pre_operational */
                /* is automatic as defined in DS301. */
                /* App don't have to call SetState(d, Pre_operational) */

            case Pre_operational:
            {

                //s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 0, 1};
                newCommunicationState.cServices = 0x5E;
                uData.nodeState = Pre_operational;
                switchCommunicationState(newCommunicationState);
                preOperational();
            }
                if (!uData.canFlags.iam_autostart) break;

            case Operational:
                if (uData.nodeState == Initialisation) return 0xFF;
            {
                //s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1, 0};
                newCommunicationState.cServices = 0x3E;
                uData.nodeState = Operational;
                newState = Operational;
                switchCommunicationState(newCommunicationState);
                operational();
            }
                break;

            case Stopped:
                if (uData.nodeState == Initialisation) return 0xFF;
            {
                //s_state_communication newCommunicationState = {0, 0, 0, 0, 1, 0, 1};
                newCommunicationState.cServices = 0x50;
                uData.nodeState = Stopped;
                newState = Stopped;
                switchCommunicationState(newCommunicationState);
                stopped();
            }
                break;
            default:
                return 0xFF;

        }/* end switch case */
    }
    /* d->nodeState contains the final state */
    /* may not be the requested state */
    return uData.nodeState;
}
//------------------------------------------------------------------------------

UNS8 getNodeId(void) {
    return uData.nodeId;
}
//------------------------------------------------------------------------------

void setNodeId(UNS8 nodeId) {
    extern const indextable objdict[];
    extern const quick_index firstIndex;
    extern const quick_index lastIndex;

    if (!(nodeId > 0 && nodeId <= 127)) {
        //	  MSG_WAR(0x2D01, "Invalid NodeID",nodeId);
        return;
    }

    UNS8 ptrIndex = firstIndex.SDO_SVR;
    if (ptrIndex) {
        /* Adjust COB-ID Client->Server (rx) only id already set to default value or id not valid (id==0xFF)*/
        *(UNS32*) objdict[ptrIndex].pSubindex[1].pObject = (UNS32) (0x600 + nodeId);
        /* Adjust COB-ID Server -> Client (tx) only id already set to default value or id not valid (id==0xFF)*/
        *(UNS32*) objdict[ptrIndex].pSubindex[1].pObject = (UNS32) (0x580 + nodeId);
    }

    {
        UNS8 i = 0;
        UNS8 ptrIndex = firstIndex.PDO_RCV;
        UNS8 ptrLast = lastIndex.PDO_RCV;
        const UNS16 cobID[] = {0x200, 0x300, 0x400, 0x500};
        if (ptrIndex) {
            while ((ptrIndex <= ptrLast) && (i < 4)) {
                *(UNS32*) objdict[ptrIndex].pSubindex[1].pObject = (UNS32) (cobID[i] + nodeId);
                i++;
                ptrIndex++;
            }
        }
    }
    /* ** Initialize the transmit PDO communication parameters. Only for 0x1800 to 0x1803 */
    {
        UNS8 i = 0;
        UNS8 ptrIndex = firstIndex.PDO_TRS;
        UNS8 ptrLast = lastIndex.PDO_TRS;
        const UNS16 cobID[] = {0x180, 0x280, 0x380, 0x480};
        if (ptrIndex) {
            while ((ptrIndex <= ptrLast) && (i < 4)) {
                *(UNS32*) objdict[ptrIndex].pSubindex[1].pObject = (UNS32) (cobID[i] + nodeId);
                i++;
                ptrIndex++;
            }
        }
    }
    //-----------------------------------------------------------------------
    /* Update EMCY COB-ID if already set to default*/
    obj1014 = (UNS32) (nodeId + 0x80);

    /* bDeviceNodeId is defined in the object dictionary. */
    uData.nodeId = nodeId;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

void initialisation(void) {
    stateLED(Initialisation);
}
//------------------------------------------------------------------------------

void preOperational(void) {
    if (!(uData.canFlags.iam_a_slave)) {
        //       masterSendNMTstateChange(d, 0, NMT_Reset_Node);
    }
    stateLED(Pre_operational);
}
//------------------------------------------------------------------------------

void operational(void) {
    stateLED(Operational);
}
//------------------------------------------------------------------------------

void stopped(void) {
    stateLED(Stopped);
}
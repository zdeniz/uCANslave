#include "nmt.h"
#include "appcfg.h"
#include "const.h"
#include "data.h"
#include "states.h"
#include "uUART.h"

extern MESSAGE uMessage;
extern CO_Data uData;

void proceedNMTstateChange(void) {
    if (uData.nodeState == Pre_operational ||
            uData.nodeState == Operational ||
            uData.nodeState == Stopped) {

        //    MSG_WAR(0x3400, "NMT received. for node :  ", (*m).data[1]);

        /* Check if this NMT-message is for this node */
        /* byte 1 = 0 : all the nodes are concerned (broadcast) */

        if ((uMessage.mdata[1] == 0) || (uMessage.mdata[1] == uData.nodeId)) {

            switch (uMessage.mdata[0]) { /* command specifier (cs) */
                case NMT_Start_Node:
                    if ((uData.nodeState == Pre_operational) || (uData.nodeState == Stopped))
                        setState(Operational);
                    break;

                case NMT_Stop_Node:
                    if (uData.nodeState == Pre_operational ||
                            uData.nodeState == Operational)
                        setState(Stopped);
                    break;

                case NMT_Enter_PreOperational:
                    if (uData.nodeState == Operational ||
                            uData.nodeState == Stopped)
                        setState(Pre_operational);
                    break;

                case NMT_Reset_Node:
                    NMT_Slave_Node_Reset_Callback();
                    setState(Initialisation);
                    break;

                case NMT_Reset_Comunication:
                {
                    UNS8 currentNodeId = getNodeId();
                    NMT_Slave_Communications_Reset_Callback();
                    // clear old NodeId to make SetNodeId reinitializing
                    // SDO, EMCY and other COB Ids
                    uData.nodeId = 0xFF;
                    setNodeId(currentNodeId);
                }
                    setState(Initialisation);
                    break;
            }/* end switch */
        }/* end if( ( (*m).data[1] == 0 ) || ( (*m).data[1] ==
        bDeviceNodeId ) ) */
    }
}
//------------------------------------------------------------------------------

UNS8 sendBootUp(void) {
    NOP();
    NOP();
    // MSG_WAR(0x3407, "Send a Boot-Up msg ", 0);
    uMessage.cmd = _HEARTBEAT; /* message configuration */
    uMessage.node = uData.nodeId;
    uMessage.rtr = NOT_A_REQUEST;
    uMessage.len = 0x01;
    uMessage.mdata[0] = 0;
    return canSend(&uMessage);
}
//------------------------------------------------------------------------------

void NMT_Slave_Node_Reset_Callback(void) {
};

//------------------------------------------------------------------------------

void NMT_Slave_Communications_Reset_Callback(void) {
    //    resetProcessImg();      //????????????? ?????????? ?????????
    //    initEngineCAN();         //????????????? ?????????? ???? CAN

    //    initTPDO();
    //    initRPDO();
};
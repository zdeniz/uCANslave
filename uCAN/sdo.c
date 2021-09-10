#include "appcfg.h"
#include "data.h"
#include "objdict.h"
#include "guard.h"
#include "objacces.h"

extern MESSAGE uMessage;
extern const indextable objdict[];
extern const quick_index firstIndex;
extern const quick_index lastIndex;

void resetSDO(void) {
};

UNS8 proceedSDO(void) {

    // Does objDic have at least one SDO 1200 + x service?
    /* Am-I a server ? */
    UNS8 nServer;
    UNS8 ptrIndex = firstIndex.SDO_SVR;
    UNS8 ptrLast = lastIndex.SDO_SVR;
    UNS32 *pCobIdStC = NULL;
    UNS32 *pCobIdCtS = NULL;
    UNS16 tCobId;
    UNS8 j = 0;

    if (ptrIndex) {
        while (ptrIndex <= ptrLast) {
            if (objdict[ptrIndex].bSubCount <= 1) {
                //			MSG_ERR(0x1A61, "Subindex 1  not found at index ", 0x1200 + j);
                // No SDO server defined. Ignore the SDO package.
                return _BUSY;
            }

            // What COBIDs are reported for it?
            /* Looking for the cobid received. */

            tCobId = uMessage.extCOBID>>1;

            pCobIdStC = (UNS32*) objdict[ptrIndex].pSubindex[1].pObject;
            pCobIdCtS = (UNS32*) objdict[ptrIndex].pSubindex[2].pObject;

            if (*pCobIdStC == tCobId) {
                //			whoami = SDO_SERVER;
                //			MSG_WAR(0x3A62, "proceedSDO. I am server. index : ", 0x1200 + j);
                /* Defining Server number = index minus 0x1200 where the cobid received is defined. */
                nServer = j; // This is the COBID of the server number J
                tCobId = (UNS16)*pCobIdCtS;  // COBID answer for CLIENT
                break;
            }
            j++;
            ptrIndex++;
        }
        //
        return _BUSY;
        // error - NO SDO server
    }

    // proceed  receive SDDO
    {
        // Make head responce SDO

        uMessage.extCOBID = tCobId<<1;
        uMessage.rtr = NOT_A_REQUEST;
        uMessage.len = 8;
        // Make boad OK responce SDO
        uMessage.sdoMes.sdoCmd = 0x80;
        uMessage.sdoMes.errorSDO = 0;

        /* Test if the size of the SDO is ok */
        if (uMessage.len != 8) {
            //		MSG_ERR(0x1A67, "Error size SDO", 0);
            uMessage.sdoMes.errorSDO = SDOABT_GENERAL_ERROR;
            return _READY;
        }
    }

    /* Testing the command specifier */
    /* Allowed : cs = 1 and 2 */
    /* cs = other : 0, 3, 4, 5, 6 Not allowed -> abort. */


    if (uMessage.sdoMes.sdoCmd == 0x40)
        //Upload expedited protocol
    {
        getEntryForSendSDO();
    } else
        //Download expedited protocol
    {
        setEntryFromReceiveSDO();
    }
    return _READY;
};

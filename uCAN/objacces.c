#include "objacces.h"
#include "objdict.h"
//------------------------------------------------------------------------------

UNS32 RegisterSetODentryCallBack(UNS16 wIndex, UNS8 bSubindex, ODCallback_t Callback) {
    UNS32 errorCode;
    ODCallback_t *CallbackList;
    const indextable *odentry;

    odentry = scanIndexOD(wIndex, &errorCode, &CallbackList);
    if (errorCode == OD_SUCCESSFUL && CallbackList && bSubindex < odentry->bSubCount)
        CallbackList[bSubindex] = Callback;
    return errorCode;
}
//------------------------------------------------------------------------------

void _storeODSubIndex(UNS16 wIndex, UNS8 bSubindex) {
};
//------------------------------------------------------------------------------
extern MESSAGE uMessage;

void getEntryForSendSDO(void) {

    UNS16 wIndex = uMessage.sdoMes.sdoIndex;
    UNS8 bSubindex = uMessage.sdoMes.sdoSubindex;

    /* DO NOT USE MSG_ERR because the macro may send a PDO -> infinite
    loop if it fails. */

    const indextable *ptrTable;

    {
        ODCallback_t *Callback;
        UNS32 errorCode;
        ptrTable = scanIndexOD(wIndex, &errorCode, &Callback); // 

        if (errorCode != OD_SUCCESSFUL) //Index is present? YES or NO
        {
            uMessage.sdoMes.errorSDO = errorCode;
            return;
        }
    }

    if (ptrTable->bSubCount <= bSubindex) { //Subindex is present? YES or NO
        /* Subindex not found */
        //    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_NO_SUCH_SUBINDEX);
        {
           uMessage.sdoMes.errorSDO = OD_NO_SUCH_SUBINDEX;
            return;
        }
    }

    if (ptrTable->pSubindex[bSubindex].bAccessType & WO) {
        //    MSG_WAR(0x2B30, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
        //    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_READ_NOT_ALLOWED);
        {
            uMessage.sdoMes.errorSDO = OD_READ_NOT_ALLOWED;
            return;
        }
    }
    {
        UNS8 entrySize;
        entrySize = ptrTable->pSubindex[bSubindex].size; //size in byte
        if (entrySize > MAX_EXPEDIT_SIZE) {
            /* Requested variable is too large to fit into a transfer line, inform    *
             * the caller about the real size of the requested variable.
             *               */
            {
                uMessage.sdoMes.errorSDO = SDOABT_OUT_OF_MEMORY;
                return;
            }
        }
        {
            UNS8 tmp;
            // *pDataType = ptrTable->pSubindex[bSubindex].bDataType;  //ODentry type
            UNS8 *p = &uMessage.sdoMes.sdoData08[entrySize - 1];
            UNS8 *q = ptrTable->pSubindex[bSubindex].pObject;
            switch (entrySize) {
                case 4:
                    *(p--) = *(q++);
                case 3:
                    *(p--) = *(q++);
                case 2:
                    *(p--) = *(q++);
                case 1:
                    *(p--) = *(q++);
                    switch (entrySize) {
                        case 4:
                            tmp = 0x43;
                            break;
                        case 3:
                            tmp = 0x47;
                            break;
                        case 2:
                            tmp = 0x4B;
                            break;
                        case 1:
                            tmp = 0x4F;
                            break;
                    }
                    break;
                case 0:
                    tmp = 0x80;
                    uMessage.sdoMes.errorSDO = SDOABT_GENERAL_ERROR;
                    break;
            }
            uMessage.sdoMes.sdoCmd = tmp;
            return;
        }
    }
}
//----------------------------------------------------------------------

void setEntryFromReceiveSDO(void) {
    const indextable *ptrTable;
 
    UNS16 wIndex = uMessage.sdoMes.sdoIndex;
    UNS8 bSubindex = uMessage.sdoMes.sdoSubindex;

    {
        UNS32 errorCode;
        ODCallback_t *Callback;
        ptrTable = scanIndexOD(wIndex, &errorCode, &Callback);
        if (errorCode != OD_SUCCESSFUL) {
          uMessage.sdoMes.errorSDO = errorCode;
            return;
        }
    }
    if (ptrTable->bSubCount <= bSubindex) {
        /* Subindex not found */
        //    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_NO_SUCH_SUBINDEX);
        uMessage.sdoMes.errorSDO = OD_NO_SUCH_SUBINDEX;
        return;
    }
    if (ptrTable->pSubindex[bSubindex].bAccessType == RO) {
        //        MSG_WAR(0x2B25, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
        //        accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_WRITE_NOT_ALLOWED);
        uMessage.sdoMes.errorSDO = OD_WRITE_NOT_ALLOWED;
        return;
    }
    {
        UNS8 entrySize;
        switch (uMessage.sdoMes.sdoCmd) {
            case 0x23:
                entrySize = 4;
                break;
            case 0x27:
                entrySize = 4;
                break;
            case 0x2B:
                entrySize = 4;
                break;
            case 0x2F:
                entrySize = 4;
                break;
            default:
                uMessage.sdoMes.errorSDO = SDOABT_OUT_OF_MEMORY;
                return;

        }
        if (entrySize != ptrTable->pSubindex[bSubindex].size) { //size in byte
            {
                uMessage.sdoMes.errorSDO = SDOABT_OUT_OF_MEMORY;
                return;
            }
        }
        {
            // *pDataType = ptrTable->pSubindex[bSubindex].bDataType;  //ODentry type
            UNS8 *p = &uMessage.sdoMes.sdoData08[entrySize - 1];
            UNS8 *q = ptrTable->pSubindex[bSubindex].pObject;
            switch (entrySize) {
                case 4:
                    *(q++) = *(p--);
                case 3:
                    *(q++) = *(p--);
                case 2:
                    *(q++) = *(p--);
                case 1:
                    *(q++) = *(p--);
                    uMessage.sdoMes.errorSDO = 0;
                    uMessage.sdoMes.sdoCmd = 0x60;
                    return;
            }
        }
    }

}

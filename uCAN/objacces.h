#ifndef OBJACCES_H
#define	OBJACCES_H

#include <xc.h> // include processor files - each processor file is guarded.
#include "appcfg.h"
#include "data.h"
#include "objdict.h"

UNS32 RegisterSetODentryCallBack(UNS16 wIndex, UNS8 bSubindex, ODCallback_t Callback);
const indextable * scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks);
void _storeODSubIndex (UNS16 wIndex, UNS8 bSubindex);
void getEntryForSendSDO(void);
void setEntryFromReceiveSDO(void);

#endif	/* OBJACCES_H */


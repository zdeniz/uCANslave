  
#ifndef EMCY_H
#define	EMCY_H
#include <xc.h> // include processor files - each processor file is guarded.
//typedef void (*post_emcy_t)(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg);
//void _post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg);
  

void emergencyInit(void);

void emergencyStop(void);

void proceedEMCY(void);

#endif	/* EMCY_H */


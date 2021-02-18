#ifndef NMT_H
#define	NMT_H
#include <xc.h> // include processor files - each processor file is guarded.  
void proceedNMTstateChange(void);
unsigned char sendBootUp(void);
void NMT_Slave_Node_Reset_Callback(void);
void NMT_Slave_Communications_Reset_Callback(void);
#endif	/* NMT_H */


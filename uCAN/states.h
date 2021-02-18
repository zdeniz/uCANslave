
#ifndef STATE_H
#define	STATE_H
#include <xc.h> // include processor files - each processor file is guarded.  

void canDispatch(void);
void switchCommunicationState(s_state_communication newCommunicationState);
UNS8 setState(e_nodeState newState);
UNS8 getNodeId(void);
void setNodeId(UNS8 nodeId);

void initialisation(void);
void preOperational(void);
void operational(void);
void stopped(void);

#endif	/* STATE_H */


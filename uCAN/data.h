/* declaration of CO_Data type let us include all necessary headers
 struct struct_CO_Data can then be defined later
 */

#ifndef DATA_H
#define	DATA_H

#include <xc.h> // include processor files - each processor file is guarded.

#include "appcfg.h"
#include "const.h"
#include "objdict.h"

typedef union {
    int timerFlags;

    struct {
        unsigned sysTickFlag : 1;
        unsigned appTickFlag : 1;
        unsigned iam_a_slave : 1;
        unsigned iam_autostart : 1;

    };
} CANFLAGS;
typedef union {

    struct {

        union {
            UNS32 sdoHead;

            struct {
                UNS8 sdoCmd;
                UNS16 sdoIndex;
                UNS8 sdoSubindex;
            };
        };

        union {
            UNS32 errorSDO;
            UNS8 sdoData08[4];
        };
    };
} SDOPACK;
typedef struct {

    union {
        UNS16 extCOBID;

        struct {
            UNS8 xNode;
            UNS8 bCmd;
        };

        struct {
            unsigned rtr : 1;
            unsigned node : 7;
            unsigned cmd : 4;
            unsigned : 4;
        };
    };
    UNS8 len; //< message's length (0 to 8)

    union {
        UNS8 mdata[8];
        SDOPACK sdoMes;
    };
} MESSAGE;


#define MESSAGE_Initializer {0,0,0,0,{0,0,0,0,0,0,0,0}}



typedef union {

    struct {

        union {
            unsigned int canHead;

            struct {
                unsigned char hiHead;
                unsigned char loHead;
            };

            struct {
                unsigned hiNode : 4;
                unsigned cmd : 4;

                unsigned len : 4;
                unsigned rtr : 1;
                unsigned loNode : 3;
            };
        };

        struct {

            union {
                unsigned char data[8];
                SDOPACK sdoMes;
            };
            unsigned char crc;
        };
    };


    unsigned char pack[11];
} CANPACK;

/* Should not be modified */
enum enum_nodeState {
    Initialisation = 0x00,
    Disconnected = 0x01,
    Connecting = 0x02,
    Preparing = 0x02,
    Stopped = 0x04,
    Operational = 0x05,
    Pre_operational = 0x7F,
    Unknown_state = 0x0F
};

typedef enum enum_nodeState e_nodeState;

typedef union {
    unsigned char cServices;

    struct {
        unsigned csBoot_Up : 1;
        unsigned csSDO : 1;
        unsigned csEmergency : 1;
        unsigned csSYNC : 1;
        unsigned csLifeGuard : 1;
        unsigned csPDO : 1;
        unsigned csLSS : 1;

    };
} s_state_communication;

/*typedef struct struct_CO_Data CO_Data; */
//const indextable *, UNS8 bSubindex);

//typedef UNS32(*ODCallback_t)(void);
//typedef const indextable * (*scanIndexOD_t)(UNS16 wIndex, UNS32 * errorCode, ODCallback_t **Callback);

typedef struct struct_CO_Data CO_Data;

//s_state_communication newCommunicationState;

/* The error states 
 * ----------------- */
typedef enum enum_errorState {
    Error_free = 0x00,
    Error_occurred = 0x01
} e_errorState;

typedef struct {
    UNS16 errCode;
    UNS8 errRegMask;
    UNS8 active;
} s_errors;
//----------------------------------------------------------------------

/* Master node data struct */
struct struct_CO_Data {
    /* General */

    UNS8 nodeId;
    CANFLAGS canFlags;

    /* State machine */
    e_nodeState nodeState;
    s_state_communication CurrentCommunicationState;

    /* EMCY */
    e_errorState error_state;
    UNS8 error_history_size;
    UNS8* error_number;
    UNS32* error_first_element;
    UNS8* error_register;
    UNS32* error_cobid;
    s_errors error_data[EMCY_MAX_ERRORS];

    /* NMT-heartbeat */
    TIMER_HANDLE ProducerHeartBeatTimer; //number current heartbeattimer 

};


/* CO_Data structure */
#define CANOPEN_NODE_DATA_INITIALIZER() {\
	/* Object dictionary*/\
	& bDeviceNodeId,     /* bDeviceNodeId */\
	objdict,             /* objdict  */\
	PDO_status,          /* PDO_status */\
	NULL,                                /* RxPDO_EventTimers */\
	_RxPDO_EventTimers_Handler,          /* RxPDO_EventTimers_Handler */\
	& firstIndex,        /* firstIndex */\
	& lastIndex,         /* lastIndex */\
	& ObjdictSize,       /* ObjdictSize */\
	& iam_a_slave,       /* iam_a_slave */\
	valueRangeTest,      /* valueRangeTest */\
	\
	/* SDO, structure s_transfer */\
	{\
          REPEAT_SDO_MAX_SIMULTANEOUS_TRANSFERS_TIMES(s_transfer_Initializer)\
	},\
	\
	/* State machine*/\
	Unknown_state,      /* nodeState */\
	/* structure s_state_communication */\
	{\
		0,          /* csBoot_Up */\
		0,          /* csSDO */\
		0,          /* csEmergency */\
		0,          /* csSYNC */\
		0,          /* csHeartbeat */\
		0,           /* csPDO */\
		0           /* csLSS */\
	},\
//	_initialisation,     /* initialisation */\
//	_preOperational,     /* preOperational */\
//	_operational,        /* operational */\
//	_stopped,            /* stopped */\
//	NULL,                /* NMT node reset callback */\
//	NULL,                /* NMT communications reset callback */\
	\
	/* NMT-heartbeat */\
	& highestSubIndex_obj1016, /* ConsumerHeartbeatCount */\
	obj1016,                   /* ConsumerHeartbeatEntries */\
	heartBeatTimers,           /* ConsumerHeartBeatTimers  */\
	& obj1017,                 /* ProducerHeartBeatTime */\
	TIMER_NONE,                                /* ProducerHeartBeatTimer */\
	_heartbeatError,           /* heartbeatError */\
	\
	{REPEAT_NMT_MAX_NODE_ID_TIMES(NMTable_Initializer)},\
                                                   /* is  well initialized at "Unknown_state". Is it ok ? (FD)*/\
	\
	/* NMT-nodeguarding */\
	TIMER_NONE,                                /* GuardTimeTimer */\
	TIMER_NONE,                                /* LifeTimeTimer */\
	_nodeguardError,           /* nodeguardError */\
	& obj100C,                 /* GuardTime */\
	& obj100D,                 /* LifeTimeFactor */\
	{REPEAT_NMT_MAX_NODE_ID_TIMES(nodeGuardStatus_Initializer)},\
	\
	/* SYNC */\
	TIMER_NONE,                                /* syncTimer */\
	& obj1005,                 /* COB_ID_Sync */\
	& obj1006,                 /* Sync_Cycle_Period */\
	/*& obj1007, */            /* Sync_window_length */\
	_post_sync,                 /* post_sync */\
	_post_TPDO,                 /* post_TPDO */\
	_post_SlaveBootup,			/* post_SlaveBootup */\
  _post_SlaveStateChange,			/* post_SlaveStateChange */\
	\
	/* General */\
	0,                                         /* toggle */\
	NULL,                   /* canSend */\
	scanIndexOD,                /* scanIndexOD */\
	_storeODSubIndex,                /* storeODSubIndex */\
    /* DCF concise */\
    NULL,       /*dcf_odentry*/\
	NULL,		/*dcf_cursor*/\
	1,		/*dcf_entries_count*/\
	0,		/* dcf_status*/\
    0,      /* dcf_size */\
    NULL,   /* dcf_data */\
	\
	/* EMCY */\
	Error_free,                      /* error_state */\
	sizeof(obj1003) / sizeof(obj1003[0]),      /* error_history_size */\
	& highestSubIndex_obj1003,    /* error_number */\
	& obj1003[0],    /* error_first_element */\
	& obj1001,       /* error_register */\
    & obj1014,       /* error_cobid */\
	/* error_data: structure s_errors */\
	{\
	REPEAT_EMCY_MAX_ERRORS_TIMES(ERROR_DATA_INITIALIZER)\
	},\
	_post_emcy,              /* post_emcy */\
	/* LSS */\
	lss_Initializer\
}


#endif	/* DATA_H */

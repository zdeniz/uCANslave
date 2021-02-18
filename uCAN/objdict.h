

#ifndef OBJDICT_H
#define OBJDICT_H

#include <xc.h> // include processor files - each processor file is guarded. 
#include "appcfg.h"
#include "data.h"
/************************ STRUCTURES ****************************/
/** This are some structs which are neccessary for creating the entries
 *  of the object dictionary.
 */
typedef struct td_subindex
{
    UNS8                    bAccessType;
    UNS8                    bDataType; /* Defines of what datatype the entry is */
    UNS8                   size;      /* The size (in Byte) of the variable */
    void*                   pObject;   /* This is the pointer of the Variable */
} subindex;

/** Struct for creating entries in the communictaion profile
 */
typedef struct td_indextable
{
    subindex*   pSubindex;   /* Pointer to the subindex */
    UNS8   bSubCount;   /* the count of valid entries for this subindex
                         * This count here defines how many memory has been
                         * allocated. this memory does not have to be used.
                         */
    UNS16   index;
} indextable;

typedef struct s_quick_index{
	UNS8 SDO_SVR;
	UNS8 SDO_CLT;
	UNS8 PDO_RCV;
	UNS8 PDO_RCV_MAP;
	UNS8 PDO_TRS;
	UNS8 PDO_TRS_MAP;
}quick_index;



typedef UNS32 (*ODCallback_t)(void);
typedef const indextable * (*scanIndexOD_t)(UNS16 wIndex, UNS32 * errorCode, ODCallback_t **Callback);

/* Prototypes of function provided by object dictionnary */
UNS32 valueRangeTest (UNS8 typeValue, void * value);
const indextable * scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks);

#endif /* OBJDICT_H */

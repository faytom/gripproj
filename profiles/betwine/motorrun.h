#ifndef MOTORRUN_H
#define MOTORRUN_H

#define MR_VALUE                0
#define MR_STAMP                1

#define MR_SERV_UUID            0xAA30

#define MR_VALUE_UUID           0xAA31
#define MR_STAMP_UUID           0xAA32

#define MR_SERVICE              0x00000001

#define MR_STAMP_LEN            4

typedef NULL_OK void (*mrProfileChange_t)( uint8 param );

typedef struct
{
    mrProfileChange_t         pfnMrProfileChange;
} mrProfileCBs_t;

extern bStatus_t MR_AddService( uint32 services );

extern bStatus_t MR_RegisterAppCBs( mrProfileCBs_t *appCallbacks );

extern bStatus_t MR_SetParameter( uint8 param, uint8 len, void *pValue );

extern bStatus_t MR_GetParameter( uint8 param, void *pValue );

#endif

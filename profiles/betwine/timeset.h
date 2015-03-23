#ifndef TIMESET_H
#define TIMESET_H

#define TS_VALUE                0

#define TS_SERV_UUID            0xAA50

#define TS_VALUE_UUID           0xAA51

#define TS_SERVICE              0x00000001

#define TIME_LEN                6

typedef NULL_OK void (*tsProfileChange_t)(uint8 param[6]);

typedef struct
{
    tsProfileChange_t     pfnTsProfileChange;
} tsProfileCBs_t;

extern bStatus_t TS_AddService( uint32 services );

extern bStatus_t TS_RegisterAppCBs( tsProfileCBs_t *appCallbacks );

extern bStatus_t TS_SetParameter( uint8 param, uint8 len, void *pValue );

extern bStatus_t TS_GetParameter( uint8 param, void *pValue );

#endif

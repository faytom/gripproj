#ifndef SYSTEMTEST_H
#define SYSTEMTEST_H

#define ST_TEST1                0
#define ST_TEST2                1

#define ST_SERV_UUID            0xAA40

#define ST_TEST1_UUID           0xAA41
#define ST_TEST2_UUID           0xAA42

#define ST_SERVICE              0x00000001

#define ST_TEST_LEN             20

typedef NULL_OK void (*stProfileChange_t)( uint8 param[ST_TEST_LEN] );

typedef struct
{
    stProfileChange_t           pfnStProfileChange;
} stProfileCBs_t;

extern bStatus_t ST_AddService( uint32 services );
extern bStatus_t ST_RegisterAppCBs( stProfileCBs_t *appCallbacks );
extern bStatus_t ST_SetParameter( uint8 param, uint8 len, void *pValue );
extern bStatus_t ST_GetParameter( uint8 param, void *pValue );

#endif
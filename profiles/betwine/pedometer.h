#ifndef PEDOMETER_H
#define PEDOMETER_H

#define PM_STATE                0
#define PM_VALUE                1

#define PM_SERV_UUID            0xAA20

#define PM_STATE_UUID           0xAA21
#define PM_VALUE_UUID           0xAA22

#define PM_SERVICE              0x00000001

#define STEP_LEN             3

extern bStatus_t PM_AddService( uint32 services );

extern bStatus_t PM_SetParameter( uint8 param, uint8 len, void *pValue );

extern bStatus_t PM_GetParameter( uint8 param, void *pValue );

#endif

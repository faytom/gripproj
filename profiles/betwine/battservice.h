#ifndef BATTSERVICE_H
#define BATTSERVICE_H

#define BT_VALUE                0

#define BT_SERV_UUID            0xAA60

#define BT_VALUE_UUID           0xAA61

#define BT_SERVICE              0x00000001

extern bStatus_t BT_AddService( uint32 services );

extern bStatus_t BT_SetParameter( uint8 param, uint8 len, void *pValue );

extern bStatus_t BT_GetParameter( uint8 param, void *pValue );

#endif

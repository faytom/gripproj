#ifndef HITPOINT_H
#define HITPOINT_H

#define HP_VALUE                0

#define HP_SERV_UUID            0xAA10

#define HP_VALUE_UUID           0xAA11

#define HP_SERVICE              0x00000001

extern bStatus_t HP_AddService( uint32 services );

extern bStatus_t HP_SetParameter( uint8 param, uint8 len, void *pValue );

extern bStatus_t HP_GetParameter( uint8 param, void *pValue );

#endif

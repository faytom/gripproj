#ifndef HISTORYSTEP_H
#define HISTORYSTEP_H

#define HS_DAY                  0

#define HS_SERV_UUID            0xAA70

#define HS_STEPS_UUID           0xAA71

#define HS_SERVICE              0x00000001

#define STEPS_LEN               21

extern bStatus_t HS_AddService( uint32 services );

extern bStatus_t HS_SetParameter( uint8 param, uint8 len, void *pValue );

extern bStatus_t HS_GetParameter( uint8 param, void *pValue );

#endif

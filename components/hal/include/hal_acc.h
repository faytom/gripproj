#ifndef HAL_ACC_H
#define HAL_ACC_H

#include "MMA8652FC.h"
#include "hal_i2c.h"
#include "hal_types.h"
#include "hal_defs.h"

void HalAccInit( void );
void HalAccReset( void );
void HalAccActive( void );
void HalAccStandby( void );
void HalAccReadFifoData( uint8 pData[ACC_F_WMRK_COUNT] );
void HalAccClearTapIntr( void );

#endif
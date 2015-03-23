#include "hal_acc.h"

void HalAccInit( void )
{
    uint8 reg;
    
    HalI2CInit(HAL_MMA8652FC_I2C_ADDRESS, i2cClock_197KHZ);
    
    // Accel Standby
    HalI2CRead(ACC_REG_ADDR_CTRL_REG1, 1, &reg);
    reg &= ACC_STANDBY_MODE;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG1, 1, &reg);

    // Register Init
    HalI2CRead(ACC_REG_ADDR_F_SETUP, 1, &reg);
    reg &= ~ACC_F_MODE_MASK;
    HalI2CWrite(ACC_REG_ADDR_F_SETUP, 1, &reg);
    
    HalI2CRead(ACC_REG_ADDR_CTRL_REG1, 1, &reg);
    reg &= ~ACC_DATA_RATE_MASK;
    reg |= ACC_DATA_RATE_50;
    reg |= ACC_FAST_MODE;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG1, 1, &reg);
    
    HalI2CRead(ACC_REG_ADDR_F_SETUP, 1, &reg);
    reg &= ~ACC_F_MODE_MASK;
    reg |= ACC_F_MODE_CIRCULAR;
    reg &= ~ACC_F_WMRK_MASK;
    reg |= ACC_F_WMRK_25;
    HalI2CWrite(ACC_REG_ADDR_F_SETUP, 1, &reg);
    
    HalI2CRead(ACC_REG_ADDR_XYZ_DATA_CFG, 1, &reg);
    reg &= ~ACC_FS_MASK;
    reg |= ACC_FS_2G;
    reg &= ~ACC_HPF_OUT_ENABLE;
    HalI2CWrite(ACC_REG_ADDR_XYZ_DATA_CFG, 1, &reg);
  
    HalI2CRead(ACC_REG_ADDR_CTRL_REG2, 1, &reg);
    reg &= ~ACC_MODS_MASK;
    reg |= ACC_MODS_LOW_NOISE;
    reg &= ~ACC_SMODS_MASK;
    reg |= ACC_SMODS_LOW_NOISE;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG2, 1, &reg);
    
    HalI2CRead(ACC_REG_ADDR_CTRL_REG3, 1, &reg);
    reg |= ACC_INT_WAKE_PULSE;
    reg &= ~ACC_INT_ACTIVE_HIGH;
    reg |= ACC_INT_OPEN_DRAIN;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG3, 1, &reg);
    
    HalI2CRead(ACC_REG_ADDR_PULSE_CFG, 1, &reg);
    reg &= ~ACC_PULSE_CFG_DPA;
    reg |= ACC_PULSE_CFG_ELE;
    reg |= ACC_PULSE_CFG_ZDPEFE;
    reg &= ~ACC_PULSE_CFG_ZSPEFE;
//    reg |= ACC_PULSE_CFG_ZSPEFE;
    HalI2CWrite(ACC_REG_ADDR_PULSE_CFG, 1, &reg);
    
    reg = ACC_PULSE_TMLT;
    HalI2CWrite(ACC_REG_ADDR_PULSE_TMLT, 1, &reg);
    
    reg = ACC_PULSE_LTCY;
    HalI2CWrite(ACC_REG_ADDR_PULSE_LTCY, 1, &reg);
    
    reg = ACC_PULSE_WIND;
    HalI2CWrite(ACC_REG_ADDR_PULSE_WIND, 1, &reg);
    
    HalI2CRead(ACC_REG_ADDR_CTRL_REG4, 1, &reg);
    reg |= ACC_INT_EN_FIFO;
    reg |= ACC_INT_EN_PULSE;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG4, 1, &reg);
 
    HalI2CRead(ACC_REG_ADDR_CTRL_REG5, 1, &reg);
    reg |= ACC_INT1_CFG_FIFO;
    reg &= ~ACC_INT1_CFG_PULSE;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG5, 1, &reg);
    
    HalI2CDisable();
}

void HalAccReset( void )
{
    uint8 reg;
    
    HalI2CInit(HAL_MMA8652FC_I2C_ADDRESS, i2cClock_197KHZ);
    
    HalI2CRead(ACC_REG_ADDR_CTRL_REG2, 1, &reg);
    reg |= ACC_RESET;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG2, 1, &reg);
    
    HalI2CDisable();
}

void HalAccActive( void )
{
    uint8 reg;
    
    HalI2CInit(HAL_MMA8652FC_I2C_ADDRESS, i2cClock_197KHZ);
    
    HalI2CRead(ACC_REG_ADDR_CTRL_REG1, 1, &reg);
    reg |= ACC_ACTIVE_MODE;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG1, 1, &reg);
    
    HalI2CDisable();
}

void HalAccStandby( void )
{
    uint8 reg;
    
    HalI2CInit(HAL_MMA8652FC_I2C_ADDRESS, i2cClock_197KHZ);
    
    HalI2CRead(ACC_REG_ADDR_CTRL_REG1, 1, &reg);
    reg &= ACC_STANDBY_MODE;
    HalI2CWrite(ACC_REG_ADDR_CTRL_REG1, 1, &reg);
    
    HalI2CDisable();
}

void HalAccClearTapIntr( void )
{
    uint8 reg;
    
    HalI2CInit(HAL_MMA8652FC_I2C_ADDRESS, i2cClock_197KHZ);
    
    while (1)
    {
        HalI2CRead(ACC_REG_ADDR_INT_SOURCE, 1, &reg);
        
        if (reg & ACC_SRC_PULSE)
        {
            HalI2CRead(ACC_REG_ADDR_PULSE_SRC, 1, &reg);
            
            reg = ACC_PULSE_THSZ;
            HalI2CWrite(ACC_REG_ADDR_PULSE_THSZ, 1, &reg);
            
            break;
        }
    }
    
    HalI2CDisable();
}

void HalAccReadFifoData( uint8 pData[ACC_F_WMRK_COUNT * 3] )
{
    uint8 reg;
    
    HalI2CInit(HAL_MMA8652FC_I2C_ADDRESS, i2cClock_197KHZ);
    
    while (1)
    {
        HalI2CRead(ACC_REG_ADDR_INT_SOURCE, 1, &reg);
        
        if (reg & ACC_SRC_FIFO)
        {
            HalI2CRead(ACC_REG_ADDR_OUT_X_MSB, ACC_F_WMRK_COUNT * 3, pData);
            
            HalI2CRead(ACC_REG_ADDR_STATUS, 1, &reg);
            
            break;
        }
    }
    
    HalI2CDisable();
}

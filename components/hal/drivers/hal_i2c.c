#include "hal_i2c.h"

static uint8 i2cAddr;

void HalI2CInit(uint8 address, i2cClock_t clockRate)
{
    i2cAddr = address << 1;
    
    I2C_WRAPPER_DISABLE();//Wrapper Control Set Pin2 &3 to I2C Functionality
    I2CADDR = 0;//ic2 slave own address
    I2C_CLOCK_RATE(clockRate);
    I2C_ENABLE();
}

uint8 HalI2CRead(uint8 regAdd, uint8 len, uint8 *data)
{
    uint8 success = 0;
    
    I2C_STRT();
    
    if (I2CSTAT == mstStarted)
    {
        I2C_WRITE(i2cAddr);//write slave address vias i2c, write bit on lsb set to 0
        
        if (I2CSTAT == mstAddrAckW)
        {
            I2C_WRITE(regAdd);//write slave register address
            
            if (I2CSTAT == mstDataAckW)
            {
                I2C_STRT();
                
                if (I2CSTAT == mstRepStart)
                {
                    I2C_WRITE(i2cAddr | I2C_MST_RD_BIT);
                    
                    if (I2CSTAT == mstAddrAckR)
                    {
                        if (len == 1)
                        {
                            I2C_READ(*data);
                            if (I2CSTAT == mstDataNackR)
                            {
                                I2C_STOP();
                                return success;
                            }
                            else
                            {
                                success = 6;
                            }    
                        }
                        else
                        {
                            for (uint8 i = len; i > 0; i--)
                            {
                                if (i > 1)
                                {
                                    I2C_SET_ACK();
                                }
                                else
                                {
                                    I2C_SET_NACK();
                                }
                                
                                I2C_READ(*data);
                                
                                if (I2CSTAT == mstDataNackR)
                                {
                                    I2C_STOP();
                                    return success;
                                }
                                else if (I2CSTAT == mstDataAckR)
                                {
                                    data++;
                                }
                                else
                                {
                                    success = 6;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        success = 5;
                    }    
                }
                else
                {
                    success = 4;
                }
            }
            else
            {
                success = 3;
            }    
        }
        else
        {
            success = 2;
        }    
        
        I2C_STOP();
    }
    else
    {
        success = 1;
    }
    
    return success;
}

uint8 HalI2CWrite(uint8 regAdd, uint8 len, uint8 *data)
{
    uint8 success = 0;
    
    I2C_STRT();
    
    if (I2CSTAT == mstStarted)
    {
        I2C_WRITE(i2cAddr);
        
        if (I2CSTAT == mstAddrAckW)
        {
            I2C_WRITE(regAdd);
            
            if (I2CSTAT == mstDataAckW)
            {
                I2C_WRITE(*data);
                
                if (I2CSTAT == mstDataAckW)
                {
                    I2C_STOP();
                    return success;
                }
                else
                {
                    success = 4;
                }    
            }
            else
            {
                success = 3;
            }    
        }
        else
        {
            success = 2;
        }    
        
        I2C_STOP();
    }
    else
    {
        success = 1;
    }
    
    return success;
}

void HalI2CDisable(void)
{
    I2C_DISABLE();
}

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include "hal_board.h"
#include "hal_types.h"
#include "hal_defs.h"

#define HAL_I2C_SLAVE_ADDR_DEF      0x41

#define I2C_ENS1                    BV(6)
#define I2C_STA                     BV(5)
#define I2C_STO                     BV(4)
#define I2C_SI                      BV(3)
#define I2C_AA                      BV(2)

#define I2C_CLOCK_MASK              0x83

typedef enum
{
    i2cClock_123KHZ = 0x00,
    i2cClock_144KHZ = 0x01,
    i2cClock_165KHZ = 0x02,
    i2cClock_197KHZ = 0x03,
    i2cClock_33KHZ  = 0x80,
    i2cClock_267KHZ = 0x81,
    i2cClock_533KHZ = 0x82
} i2cClock_t;

typedef enum
{
    mstStarted      = 0x08,
    mstRepStart     = 0x10,
    mstAddrAckW     = 0x18,
    mstAddrNackW    = 0x20,
    mstDataAckW     = 0x28,
    mstDataNackW    = 0x30,
    mstLostArb      = 0x38,
    mstAddrAckR     = 0x40,
    mstAddrNackR    = 0x48,
    mstDataAckR     = 0x50,
    mstDataNackR    = 0x58
} i2cStatus_t;

#define I2C_MST_RD_BIT              0x01

#define I2C_WRAPPER_DISABLE()       st( I2CWC = 0x00; )

#define I2C_CLOCK_RATE(x)           st( I2CCFG &= ~I2C_CLOCK_MASK;       \
                                        I2CCFG |= x; )

#define I2C_SET_NACK()              st( I2CCFG &= ~I2C_AA; )
#define I2C_SET_ACK()               st( I2CCFG |=  I2C_AA; )

#define I2C_ENABLE()                st( I2CCFG |=  I2C_ENS1; )
#define I2C_DISABLE()               st( I2CCFG &= ~I2C_ENS1; )

#define I2C_STRT()                  st( I2CCFG &= ~I2C_SI;              \
                                        I2CCFG |= I2C_STA;              \
                                        while ((I2CCFG & I2C_SI) == 0); \
                                        I2CCFG &= ~I2C_STA; )

#define I2C_STOP()                  st( I2CCFG |= I2C_STO;              \
                                        I2CCFG &= ~I2C_SI;              \
                                        while ((I2CCFG & I2C_STO) != 0); )

#define I2C_READ(_X_)               st( I2CCFG &= ~I2C_SI;              \
                                        while ((I2CCFG & I2C_SI) == 0); \
                                        (_X_) = I2CDATA; )

#define I2C_WRITE(_X_)              st( I2CDATA = (_X_);                \
                                        I2CCFG &= ~I2C_SI;              \
                                        while ((I2CCFG & I2C_SI) == 0); )

void HalI2CInit(uint8 address, i2cClock_t clockRate);
uint8 HalI2CRead(uint8 reg, uint8 len, uint8 *pBuf);
uint8 HalI2CWrite(uint8 reg, uint8 len, uint8 *pBut);
void HalI2CDisable(void);

#endif

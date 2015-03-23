#ifndef HAL_KEY_H
#define HAL_KEY_H

#include "hal_board.h"

#define HAL_KEY_INTERRUPT_DISABLE           0x00
#define HAL_KEY_INTERRUPT_ENABLE            0x01

#define HAL_KEY_STATE_NORMAL                0x00
#define HAL_KEY_STATE_SHIFT                 0x01

#define HAL_ACC_INT1                        0x01
#define HAL_ACC_INT2                        0x02
#define HAL_CHR_INT                         0x04

#define HAL_KEY_RISING_EDGE                 0
#define HAL_KEY_FALLING_EDGE                1

#define HAL_KEY_DEBOUNCE_VALUE              25

#define HAL_KEY_CPU_PORT_0_IF               P0IF
#define HAL_KEY_CPU_PORT_1_IF               P1IF
#define HAL_P0_INPUT_EDGEBIT                BV(0)
#define HAL_P1_INPUT_EDGEBIT                BV(2)

#define HAL_ACC_INT1_PORT                   P1
#define HAL_ACC_INT1_BIT                    BV(6)
#define HAL_ACC_INT1_SEL                    P1SEL
#define HAL_ACC_INT1_DIR                    P1DIR

#define HAL_ACC_INT1_IEN                    IEN2
#define HAL_ACC_INT1_IENBIT                 BV(4)
#define HAL_ACC_INT1_ICTL                   P1IEN
#define HAL_ACC_INT1_ICTLBIT                BV(6)
#define HAL_ACC_INT1_PXIFG                  P1IFG
#define HAL_ACC_INT1_IFGBIT                 0xBF

#define HAL_ACC_INT2_PORT                   P1
#define HAL_ACC_INT2_BIT                    BV(7)
#define HAL_ACC_INT2_SEL                    P1SEL
#define HAL_ACC_INT2_DIR                    P1DIR

#define HAL_ACC_INT2_IEN                    IEN2
#define HAL_ACC_INT2_IENBIT                 BV(4)
#define HAL_ACC_INT2_ICTL                   P1IEN
#define HAL_ACC_INT2_ICTLBIT                BV(7)
#define HAL_ACC_INT2_PXIFG                  P1IFG
#define HAL_ACC_INT2_IFGBIT                 0x7F

#define HAL_CHR_INT_PORT                    P0
#define HAL_CHR_INT_BIT                     BV(7)
#define HAL_CHR_INT_SEL                     P0SEL
#define HAL_CHR_INT_DIR                     P0DIR
#define HAL_CHR_INT_INP                     P0INP

#define HAL_CHR_INT_IEN                     IEN1
#define HAL_CHR_INT_IENBIT                  BV(5)
#define HAL_CHR_INT_ICTL                    P0IEN
#define HAL_CHR_INT_ICTLBIT                 BV(7)
#define HAL_CHR_INT_PXIFG                   P0IFG
#define HAL_CHR_INT_IFGBIT                  0x7F

#define HAL_CHR_PULLUP_PORT                 P0
#define HAL_CHR_PULLUP_BIT                  BV(6)
#define HAL_CHR_PULLUP_SEL                  P0SEL
#define HAL_CHR_PULLUP_DIR                  P0DIR
#define HAL_CHR_PULLUP_INP                  POINP

typedef void (*halKeyCBack_t) (uint8 keys, uint8 state);

extern bool Hal_KeyIntEnable;

extern void HalKeyInit( void );

extern void HalKeyConfig( bool interruptEnable, const halKeyCBack_t cback );

extern uint8 HalKeyRead( void );

extern void HalKeyEnterSleep( void );

extern uint8 HalKeyExitSleep( void );

extern void HalKeyPoll( void );

#endif

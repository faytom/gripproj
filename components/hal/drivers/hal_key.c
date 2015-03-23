#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_drivers.h"
#include "osal.h"
#include "hal_key.h"

static uint8 halKeySavedKeys;
static halKeyCBack_t pHalKeyProcessFunction;
static uint8 halKeyConfigured;
bool Hal_KeyIntEnable;

void halProcessKeyInterrupt( void );
void halProcessBattInterrupt( void );

void HalKeyInit( void )
{
    halKeySavedKeys = 0;

    HAL_ACC_INT1_SEL &= ~(HAL_ACC_INT1_BIT);
    HAL_ACC_INT1_DIR &= ~(HAL_ACC_INT1_BIT);

    HAL_ACC_INT2_SEL &= ~(HAL_ACC_INT2_BIT);
    HAL_ACC_INT2_DIR &= ~(HAL_ACC_INT2_BIT);

    HAL_CHR_INT_SEL &= ~(HAL_CHR_INT_BIT);
    HAL_CHR_INT_DIR &= ~(HAL_CHR_INT_BIT);
    HAL_CHR_INT_INP |= HAL_CHR_INT_BIT;
    
    pHalKeyProcessFunction = NULL;
    
    halKeyConfigured = FALSE;
}

void HalKeyConfig( bool interruptEnable, halKeyCBack_t cback )
{
    Hal_KeyIntEnable = interruptEnable;
    
    pHalKeyProcessFunction = cback;
    
    if (Hal_KeyIntEnable)
    {
        PICTL |= HAL_P1_INPUT_EDGEBIT;
        PICTL |= HAL_P0_INPUT_EDGEBIT;

        HAL_ACC_INT1_ICTL |= HAL_ACC_INT1_ICTLBIT;
        HAL_ACC_INT1_IEN |= HAL_ACC_INT1_IENBIT;
        HAL_ACC_INT1_PXIFG = HAL_ACC_INT1_IFGBIT;
        
        HAL_ACC_INT2_ICTL |= HAL_ACC_INT2_ICTLBIT;
        HAL_ACC_INT2_IEN |= HAL_ACC_INT2_IENBIT;
        HAL_ACC_INT2_PXIFG = HAL_ACC_INT2_IFGBIT;

        HAL_CHR_INT_ICTL |= HAL_CHR_INT_ICTLBIT;
        HAL_CHR_INT_IEN |= HAL_CHR_INT_IENBIT;
        HAL_CHR_INT_PXIFG = HAL_CHR_INT_IFGBIT;

        if (halKeyConfigured == TRUE)
        {
            osal_stop_timerEx(Hal_TaskID, HAL_KEY_EVENT);
        }    
    }
    else
    {
        HAL_ACC_INT1_ICTL &= ~(HAL_ACC_INT1_ICTLBIT);
        HAL_ACC_INT1_IEN  &= ~(HAL_ACC_INT1_IENBIT);
        HAL_ACC_INT2_ICTL &= ~(HAL_ACC_INT2_ICTLBIT);
        HAL_ACC_INT2_IEN  &= ~(HAL_ACC_INT2_IENBIT);
        HAL_CHR_INT_ICTL  &= ~(HAL_CHR_INT_ICTLBIT);
        HAL_CHR_INT_IEN   &= ~(HAL_CHR_INT_IENBIT);

        osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
    }
    
    halKeyConfigured = TRUE;
}

uint8 HalKeyRead( void )
{
    uint8 keys = 0;

    if (!(HAL_ACC_INT1_PORT & HAL_ACC_INT1_BIT))
    {
        keys |= HAL_ACC_INT1;
    }

    if (!(HAL_ACC_INT2_PORT & HAL_ACC_INT2_BIT))
    {
        keys |= HAL_ACC_INT2;
    }
    
    if (!(HAL_CHR_INT_PORT & HAL_CHR_INT_BIT))
    {
        keys |= HAL_CHR_INT;
    }
    
    return keys;
}

void HalKeyPoll( void )
{
    uint8 keys = 0;
    uint8 notify = 0;
    
    if (!(HAL_ACC_INT1_PORT & HAL_ACC_INT1_BIT))
    {
        keys |= HAL_ACC_INT1;
    }

    if (!(HAL_ACC_INT2_PORT & HAL_ACC_INT2_BIT))
    {
        keys |= HAL_ACC_INT2;
    }
    
    if (!(HAL_CHR_INT_PORT & HAL_CHR_INT_BIT))
    {
        keys |= HAL_CHR_INT;
    }

    if (!Hal_KeyIntEnable)
    {
        if (keys == halKeySavedKeys)
        {
            return;
        }
        else
        {
            notify = 1;
        }    
    }
    else
    {
        if (keys)
        {
            notify = 1;
        }    
    }
    
    halKeySavedKeys = keys;
    
    if (notify && (pHalKeyProcessFunction))
    {
        (pHalKeyProcessFunction)(keys, HAL_KEY_STATE_NORMAL);
    }    
}

void halProcessAccInterrupt( void )
{
    if (HAL_ACC_INT1_PXIFG & HAL_ACC_INT1_BIT)
    {
        osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
    }

    if (HAL_ACC_INT2_PXIFG & HAL_ACC_INT2_BIT)
    {
        osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
    }   
}

void halProcessChrInterrupt( void )
{
    if (HAL_CHR_INT_PXIFG & HAL_CHR_INT_BIT)
    {
        osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
    }
}

void HalKeyEnterSleep( void )
{
}

uint8 HalKeyExitSleep( void )
{
    return( HalKeyRead() );
}

HAL_ISR_FUNCTION( halKeyPort0Isr, P0INT_VECTOR )
{
    HAL_ENTER_ISR();
 
    halProcessChrInterrupt();
    
    if (HAL_CHR_INT_PXIFG & HAL_CHR_INT_BIT)
    {
        HAL_CHR_INT_PXIFG = HAL_CHR_INT_IFGBIT;
    }
    
    HAL_KEY_CPU_PORT_0_IF = 0;
    
    CLEAR_SLEEP_MODE();
    
    HAL_EXIT_ISR();
    
    return;
}

HAL_ISR_FUNCTION( halKeyPort1Isr, P1INT_VECTOR )
{
    HAL_ENTER_ISR();

    halProcessAccInterrupt();
    
    if (HAL_ACC_INT1_PXIFG & HAL_ACC_INT1_BIT)
    {
        HAL_ACC_INT1_PXIFG = HAL_ACC_INT1_IFGBIT;
    }
    
    if (HAL_ACC_INT2_PXIFG & HAL_ACC_INT2_BIT)
    {
        HAL_ACC_INT2_PXIFG = HAL_ACC_INT2_IFGBIT;
    }

    HAL_KEY_CPU_PORT_1_IF = 0;
    
    CLEAR_SLEEP_MODE();
    
    HAL_EXIT_ISR();
    
    return;
}



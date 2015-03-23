#include "bcomdef.h"

#include "hal_key.h"
#include "hal_led.h"
#include "hal_adc.h"
#include "hal_acc.h"
#include "hal_aes.h"

#include "hitpoint.h"
#include "timeset.h"
#include "battservice.h"
#include "pedometer.h"
#include "historystep.h"
#include "systemtest.h"
#include "motorrun.h"

#include "app.h"

// clock
static uint8 currentTime[2];
static uint16 sleepTime;
static uint16 wakeTime;

// hitpoints
static uint8 hpUpdateState;
static uint32 aminagoStep;
static uint32 currentStep;
static uint8 exerciseDuration;
static uint8 sittingDuration;
static uint8 exerciseInterval;

// charger & battery
static uint8 chargerState;
static uint8 batteryValue;
static uint8 chargerRemindInterval;
static uint8 batteryFullyInterval;

// acceleration
static uint16 accelFilterArray[4];
static uint16 lastAccel;
static uint16 maxAccel;
static uint16 minAccel;
static uint16 accelThresholdParam;
static uint8  accelIntervalParam;
static uint16 accelAverageParam;
static uint8  accelInterval;

// pedometer
static uint8 paramUpdateInterval;
static uint8 pedometerPattern;
uint32 realtimeStep;
static uint8 stepUpdateWaiting;

static void AppHpReminds( void );
static void AppChargerRemind( void );

void AppInit( void )
{
    // hitpoints
    hpUpdateState = APP_TIME_WAKING;
    exerciseDuration = 0;
    sittingDuration = 0;
    exerciseInterval = 0;
    currentStep = 0;
    aminagoStep = 0;
    
    // clock
    currentTime[0] = 0;
    currentTime[1] = 0;
    sleepTime = 0;
    wakeTime = 0;
    uint8 timeArray[6];
    TS_GetParameter( TS_VALUE, timeArray );
    AppTimeSet( timeArray );
    
    // charger & battery
    chargerState = APP_CHR_UNPLUGGED;
    batteryValue = 50;
    chargerRemindInterval = 0;
    batteryFullyInterval = 0;
    
    // acceleration
    accelFilterArray[0] = 0;
    accelFilterArray[1] = 0;
    accelFilterArray[2] = 0;
    accelFilterArray[3] = 0;
    lastAccel = 0;
    maxAccel = 0;
    minAccel = 0;
    accelThresholdParam = 0;
    accelIntervalParam = 0;
    accelAverageParam = 0;
    accelInterval = 0;
    
    //pedometer
    paramUpdateInterval = 0;
    pedometerPattern = 0;
    realtimeStep = 0;
    stepUpdateWaiting = 0;
}

void AppStandby( void )
{
    HalAccStandby();
    HAL_CHR_INT_ICTL |= HAL_CHR_INT_ICTLBIT;
    HAL_CHR_INT_IEN  |= HAL_CHR_INT_IENBIT;
    HAL_CHR_INT_PXIFG = HAL_CHR_INT_IFGBIT;
}

void AppActive( void )
{
    HalAccReset();
    HalAccInit();
    HalAccActive();
}

void AppTimeSet( uint8 timeParams[6] )
{
    currentTime[0] = timeParams[0];
    currentTime[1] = timeParams[1];
    wakeTime = timeParams[3] * 100 + timeParams[2];
    sleepTime = timeParams[5] * 100 + timeParams[4];
}

void AppMotorRun( uint8 motorParam )
{
    if (chargerState == APP_CHR_UNPLUGGED)
    {
        if (motorParam & HAL_LED_1)
        {
            HalLedSet(HAL_LED_1, HAL_LED_MODE_FLASH);
        }
        
        if (motorParam & HAL_LED_2)
        {
            HalLedSet(HAL_LED_2, HAL_LED_MODE_FLASH);
        }
        
        if (motorParam & HAL_LED_3)
        {
            HalLedSet(HAL_LED_3, HAL_LED_MODE_FLASH);
        }
        
        if (motorParam & HAL_LED_4)
        {
            HalLedSet(HAL_LED_4, HAL_LED_MODE_FLASH);
        }
        
        if (motorParam & HAL_LED_5)
        {
            HalLedSet(HAL_LED_5, HAL_LED_MODE_FLASH);
        }
        
        if (motorParam & HAL_MOTOR)
        {
            if (hpUpdateState == APP_TIME_WAKING)
            {
                HalLedSet(HAL_MOTOR, HAL_LED_MODE_BLINK);
            }
            else
            {
                HalLedSet(HAL_LEDS, HAL_LED_MODE_BLINK);
            }
        }
    }
}

void AppTimeUpdate( void )
{
    currentTime[0]++;
    
    if (currentTime[0] > 59)
    {
        currentTime[0] = 0;
        currentTime[1]++;
        
        if (currentTime[1] > 23)
        {
            currentTime[1] = 0;
        }
    }
    
    uint16 currentTime_u16;
    currentTime_u16 = currentTime[1] * 100 + currentTime[0];
    
    if (sleepTime > wakeTime)
    {
        if ((currentTime_u16 >= wakeTime) && (currentTime_u16 <= sleepTime))
        {
            hpUpdateState = APP_TIME_WAKING;
        }
        else
        {
            hpUpdateState = APP_TIME_SLEEPING;
        }
    }
    else if (sleepTime == wakeTime)
    {
        hpUpdateState = APP_TIME_WAKING;
    }
    else
    {
        if ((currentTime_u16 <= wakeTime) || (currentTime_u16 >= sleepTime))
        {
            hpUpdateState = APP_TIME_WAKING;
        }
        else
        {
            hpUpdateState = APP_TIME_SLEEPING;
        }
    }
    
    uint8 timeArray[6];
    
    TS_GetParameter( TS_VALUE, timeArray );
    timeArray[0] = currentTime[0];
    timeArray[1] = currentTime[1];
    TS_SetParameter( TS_VALUE, TIME_LEN, timeArray );
}

uint8 AppHitpointUpdate( void )
{
    uint32 oneminuteStep;
    aminagoStep = currentStep;
    currentStep = realtimeStep;
    
    oneminuteStep = currentStep - aminagoStep;

    if (chargerState == APP_CHR_UNPLUGGED)
    {        
        if (oneminuteStep >= HP_VALID_STEP_NUM)
        {
            exerciseDuration++;
            exerciseInterval = 1;
        }
        else
        {
            if (hpUpdateState == APP_TIME_WAKING)
                sittingDuration++;
            if (exerciseInterval >= 1)
                exerciseInterval++;
        }
    }
    
    if (exerciseDuration >= HP_EXERCISE_DURATION)
    {
        uint8 HP_Value = 5;
        HP_GetParameter( HP_VALUE, &HP_Value );
        HP_Value++;
        
        if (HP_Value < 6)
        {
            HP_SetParameter( HP_VALUE, sizeof( uint8 ), &HP_Value );
            AppHpReminds();
        }
        
        sittingDuration = 0;
        exerciseDuration = 0;
    }
    
    if (exerciseInterval >= HP_EXERCISE_INTERVAL)
    {
        exerciseDuration = 0;
        exerciseInterval = 0;
    }
    
    if (sittingDuration >= HP_SITTING_DURATION)
    {
        uint8 HP_Value = 1;
        HP_GetParameter( HP_VALUE, &HP_Value );
        HP_Value--;
        
        if ((HP_Value > 0) && (HP_Value < 4))
        {
            HalLedSet(HAL_MOTOR, HAL_LED_MODE_BLINK);
            
            HP_Value = MAX(HP_Value, 1);
            HP_SetParameter( HP_VALUE, sizeof( uint8 ), &HP_Value );
            
            uint8 timeStamp[4] = {0, 0, 0, 0};
            timeStamp[0] = currentTime[0];
            timeStamp[1] = currentTime[1];
            MR_SetParameter( MR_STAMP, MR_STAMP_LEN, timeStamp );
        }
        else
        {
            HP_Value = MAX(HP_Value, 1);
            HP_SetParameter( HP_VALUE, sizeof( uint8 ), &HP_Value );
        }
        
        AppHpReminds();
        sittingDuration = 0;
    }
    
    uint8 HP_Value;
    HP_GetParameter( HP_VALUE, &HP_Value );
    return HP_Value;
}

uint8 AppBatteryUpdate( void )
{
    if (chargerState == APP_CHR_CHARGING)
    {
        if (batteryValue < 99)
        {
            batteryValue++;
            batteryFullyInterval = 0;
        }
        else if (batteryValue == 99)
        {
            if (batteryFullyInterval < 20)
            {
                batteryFullyInterval++;
            }
            else
            {
                batteryValue = 100;
                batteryFullyInterval = 0;
            }
        }
    }
    else if (chargerState == APP_CHR_UNPLUGGED)
    {
        if (currentTime[0] == 0)
        {
            if (batteryValue > 1)
            {
                batteryValue--;
            }
        }
    }
    
    uint16 batteryAdcValue;
    HalAdcSetReference( HAL_ADC_REF_125V );
    batteryAdcValue = HalAdcRead( HAL_ADC_CHANNEL_VDD, HAL_ADC_RESOLUTION_10 );
    
    if ( batteryAdcValue <= BATT_ADC_LEVEL_3_3V )
    {
        if ( batteryValue > 20 )
        {
            batteryValue = 20;
        }
    }
    
    if ( batteryAdcValue <= BATT_ADC_LEVEL_3V )
    {
        batteryValue = 0;
    }
    
    uint8 percent = batteryValue;
    
    if ( chargerState != APP_CHR_UNPLUGGED )
    {
        percent |= 0x80;
    }
    
    BT_SetParameter( BT_VALUE, sizeof( uint8 ), &percent );
    
    return percent;
}

void AppHistoryUpdate( void )
{
    if ( (currentTime[0] == 0) && (currentTime[1] == 0) )
    {
        uint8 historySteps[STEPS_LEN];
        
        HS_GetParameter( HS_DAY, historySteps );
        
        for ( uint8 i = 7; i > 1; i--)
        {
            historySteps[i*3 - 1] = historySteps[i*3 - 4];
            historySteps[i*3 - 2] = historySteps[i*3 - 5];
            historySteps[i*3 - 3] = historySteps[i*3 - 6];
        }
        
        historySteps[0] = (realtimeStep & 0xFF);
        historySteps[1] = ((realtimeStep >> 8) & 0xFF);
        historySteps[2] = ((realtimeStep >> 16) & 0xFF);
        
        HS_SetParameter( HS_DAY, STEPS_LEN, historySteps );
        
        uint8 HP_Value = 5;
        HP_SetParameter( HP_VALUE, sizeof( uint8 ), &HP_Value );
        
        uint8 realtimeStep_Array[3];
        realtimeStep_Array[0] = 0;
        realtimeStep_Array[1] = 0;
        realtimeStep_Array[2] = 0;
        
        PM_SetParameter( PM_VALUE, STEP_LEN, realtimeStep_Array );
        
        realtimeStep = 0;
        currentStep = 0;
        aminagoStep = 0;
    }
}

void AppDoubleClickHandler( void )
{
    HalAccClearTapIntr();
    
    if (chargerState == APP_CHR_UNPLUGGED)
    {
        AppHpReminds();
    }
}

uint32 AppAccDataReadyHandler( void )
{
    uint8 pData[ACC_F_WMRK_COUNT * 3];
    
    HalAccReadFifoData( pData );

    uint8 xAccel, yAccel, zAccel;
    int8 xAccel_s8, yAccel_s8, zAccel_s8;
    uint16 xAccel_u16, yAccel_u16, zAccel_u16;
    uint16 thisAccel;
    int16 diffAccel;
    
    for (uint8 i = 0; i < ACC_F_WMRK_COUNT; i++)
    {
        xAccel = pData[i * 3];
        yAccel = pData[i*3 + 1];
        zAccel = pData[i*3 + 2];
        
        if (xAccel > 0x80)
        {
            xAccel_s8 = -xAccel;
            xAccel_u16 = xAccel_s8;
        }
        else
        {
            xAccel_u16 = xAccel;
        }
        
        if (yAccel > 0x80)
        {
            yAccel_s8 = -yAccel;
            yAccel_u16 = yAccel_s8;
        }
        else
        {
            yAccel_u16 = yAccel;
        }
        
        if (zAccel > 0x80)
        {
            zAccel_s8 = -zAccel;
            zAccel_u16 = zAccel_s8;
        }
        else
        {
            zAccel_u16 = zAccel;
        }
        
        for (uint8 j = 3; j > 0; j--)
        {    
            accelFilterArray[j] = accelFilterArray[j - 1];
        }
        
        accelFilterArray[0] = xAccel_u16 * xAccel_u16 +
                              yAccel_u16 * yAccel_u16 +
                              zAccel_u16 * zAccel_u16;
        
        thisAccel = (accelFilterArray[0] >> 2) +
                    (accelFilterArray[1] >> 2) +
                    (accelFilterArray[2] >> 2) +
                    (accelFilterArray[3] >> 2);
        
        diffAccel = thisAccel - lastAccel;
        
        if ((ABS(diffAccel) > 800) && (paramUpdateInterval == 0))
        {
            paramUpdateInterval = 1;
            maxAccel = thisAccel;
            minAccel = thisAccel;
            stepUpdateWaiting = 3;
        }
        
        if (paramUpdateInterval > 0)
        {
            maxAccel = MAX(maxAccel, thisAccel);
            minAccel = MIN(minAccel, thisAccel);
            
            if (paramUpdateInterval > 50)
            {
                paramUpdateInterval = 0;
                pedometerPattern = 0;
                
                if ((maxAccel - minAccel) > PM_WALK_THRESHOLD)
                {
                    paramUpdateInterval = 1;
                    
                    pedometerPattern = 1;
                    accelThresholdParam = PM_WALK_THRESHOLD >> 2;
                    accelIntervalParam = PM_WALK_INTERVAL;
                    
                    if ((maxAccel - minAccel) > PM_TROT_THRESHOLD)
                    {
                        pedometerPattern = 2;
                        accelThresholdParam = PM_TROT_THRESHOLD >> 2;
                        accelIntervalParam = PM_TROT_INTERVAL;
                        
                        if ((maxAccel - minAccel) > PM_RUN_THRESHOLD)
                        {
                            pedometerPattern = 3;
                            accelThresholdParam = PM_RUN_THRESHOLD >> 2;
                            accelIntervalParam = PM_RUN_INTERVAL;
                        }
                    }
                }
                
                accelAverageParam = (maxAccel >> 1) + (minAccel >> 1);
                maxAccel = accelAverageParam;
                minAccel = accelAverageParam;
                
                PM_SetParameter( PM_STATE, sizeof( uint8 ), &pedometerPattern );
            }
            else
            {
                paramUpdateInterval++;
            }
        }
        
        if (pedometerPattern > 0)
        {
            if ((thisAccel > (accelAverageParam + accelThresholdParam)) &&
                (accelInterval > accelIntervalParam))
            {
                if (stepUpdateWaiting > 0)
                {
                    if (stepUpdateWaiting == 1)
                    {
                        realtimeStep += 5;
                    }
                    stepUpdateWaiting--;
                }
                else
                {
                    realtimeStep++;
                }

                accelInterval = 0;
                
                uint8 stepNumber[3];
                
                stepNumber[0] = (realtimeStep & 0xFF);
                stepNumber[1] = ((realtimeStep >> 8) & 0xFF);
                stepNumber[2] = ((realtimeStep >> 16) & 0xFF);
                
                PM_SetParameter( PM_VALUE, STEP_LEN, stepNumber );
            }
            else
            {
                accelInterval++;
            }
        }
        else
        {
            stepUpdateWaiting = 3;
        }
        
        lastAccel = thisAccel;
    }
    
    return realtimeStep;
}

uint8 AppChargerStateChangeHandler( void )
{
    if (HAL_CHR_INT_PORT & HAL_CHR_INT_BIT)
    {
        if (chargerState == APP_CHR_PLUGGED)
        {
            chargerState = APP_CHR_FULLY;
            batteryValue = 100;
            
            AppChargerRemind();
        }
        
        if (HAL_CHR_PULLUP_SEL & HAL_CHR_PULLUP_BIT)
        {
            if (chargerState != APP_CHR_UNPLUGGED)
            {
                chargerState = APP_CHR_UNPLUGGED;
                
                HAL_CHR_INT_ICTL |= HAL_CHR_INT_ICTLBIT;
                HAL_CHR_INT_IEN  |= HAL_CHR_INT_IENBIT;
                HAL_CHR_INT_PXIFG = HAL_CHR_INT_IFGBIT;
                
                AppChargerRemind();
            }
        }
        else
        {
            HAL_CHR_PULLUP_SEL |= HAL_CHR_PULLUP_BIT;
        }
    }
    else
    {
        HAL_CHR_INT_ICTL &= ~(HAL_CHR_INT_ICTLBIT);
        HAL_CHR_INT_IEN  &= ~(HAL_CHR_INT_IENBIT);
        HAL_CHR_INT_PXIFG = HAL_CHR_INT_IFGBIT;
        
        if (HAL_CHR_PULLUP_SEL & HAL_CHR_PULLUP_BIT)
        {
            HAL_CHR_PULLUP_SEL  &= ~(HAL_CHR_PULLUP_BIT);
            HAL_CHR_PULLUP_DIR  |= HAL_CHR_PULLUP_BIT;
            HAL_CHR_PULLUP_PORT |= HAL_CHR_PULLUP_BIT;
            
            chargerState = APP_CHR_PLUGGED;
        }
        else
        {
            chargerState = APP_CHR_CHARGING;
            AppChargerRemind();
        }
    }
    
    chargerRemindInterval++;
    
    return chargerState;
}

static void AppHpReminds( void )
{
    uint8 HP_Value = 0;
    
    HP_GetParameter( HP_VALUE, &HP_Value );
    
    HalLedSet( HAL_LEDS, HAL_LED_MODE_OFF );
    
    HalLedSet( HAL_LED_1, HAL_LED_MODE_FLASH );
    
    if (HP_Value > 1)
    {
        HalLedSet( HAL_LED_2, HAL_LED_MODE_FLASH );
    }
    
    if (HP_Value > 2)
    {
        HalLedSet( HAL_LED_3, HAL_LED_MODE_FLASH );
    }
    
    if (HP_Value > 3)
    {
        HalLedSet( HAL_LED_4, HAL_LED_MODE_FLASH );
    }
    
    if (HP_Value > 4)
    {
        HalLedSet( HAL_LED_5, HAL_LED_MODE_FLASH );
    }
}

static void AppChargerRemind( void )
{
    switch ( chargerState )
    {
        case APP_CHR_CHARGING:
            {
                if (batteryValue < 100)
                {
                    if (chargerRemindInterval >= 4)
                    {
                        chargerRemindInterval = 0;
                        
                        HalLedSet(HAL_LEDS, HAL_LED_MODE_OFF);
                        HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);
                        
                        if (batteryValue > 20)
                        {
                            HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
                            HalLedSet(HAL_LED_2, HAL_LED_MODE_BLINK);
                        }
                        
                        if (batteryValue > 40)
                        {
                            HalLedSet(HAL_LED_2, HAL_LED_MODE_ON);
                            HalLedSet(HAL_LED_3, HAL_LED_MODE_BLINK);
                        }
                        
                        if (batteryValue > 60)
                        {
                            HalLedSet(HAL_LED_3, HAL_LED_MODE_ON);
                            HalLedSet(HAL_LED_4, HAL_LED_MODE_BLINK);
                        }
                        
                        if (batteryValue > 80)
                        {
                            HalLedSet(HAL_LED_4, HAL_LED_MODE_ON);
                            HalLedSet(HAL_LED_5, HAL_LED_MODE_BLINK);
                        }
                    }
                    break;
                }
                else
                {
                    if (chargerRemindInterval >= 2)
                    {
                        chargerRemindInterval = 0;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            
        case APP_CHR_FULLY:
            {
                HalLedSet(HAL_LEDS, HAL_LED_MODE_OFF);
                HalLedSet(HAL_LEDS, HAL_LED_MODE_BLINK);
            }
            break;
            
        case APP_CHR_UNPLUGGED:
            {
                HalLedSet(HAL_LEDS, HAL_LED_MODE_OFF);
                HalLedSet(HAL_LED_1, HAL_LED_MODE_FLASH);
                
                if (batteryValue > 20)
                {
                    HalLedSet(HAL_LED_2, HAL_LED_MODE_FLASH);
                }
                
                if (batteryValue > 40)
                {
                    HalLedSet(HAL_LED_3, HAL_LED_MODE_FLASH);
                }
                
                if (batteryValue > 60)
                {
                    HalLedSet(HAL_LED_4, HAL_LED_MODE_FLASH);
                }
                
                if (batteryValue > 80)
                {
                    HalLedSet(HAL_LED_5, HAL_LED_MODE_FLASH);
                }
            }
            break;
            
        default:
            {
            }
            break;
    }
}
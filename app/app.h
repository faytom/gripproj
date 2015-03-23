#ifndef APP_H
#define APP_H

#define APP_CHR_CHARGING        0x01
#define APP_CHR_FULLY           0x02
#define APP_CHR_PLUGGED         0x04
#define APP_CHR_UNPLUGGED       0x08

#define APP_TIME_SLEEPING       0x00
#define APP_TIME_WAKING         0x01

#define BATT_ADC_LEVEL_3V       425
#define BATT_ADC_LEVEL_3_3V     450

#define HP_VALID_STEP_NUM       40

//#define HP_EXERCISE_DURATION    1
//#define HP_SITTING_DURATION     2

#define HP_EXERCISE_DURATION    3
#define HP_SITTING_DURATION     30
#define HP_EXERCISE_INTERVAL    3

#define PM_WALK_THRESHOLD       2000
#define PM_TROT_THRESHOLD       4500
#define PM_RUN_THRESHOLD        7000

#define PM_WALK_INTERVAL        25
#define PM_TROT_INTERVAL        20
#define PM_RUN_INTERVAL         15

extern uint32 realtimeStep;

void AppInit( void );
void AppStandby( void );
void AppActive( void );
void AppTimeSet( uint8 timeParams[6] );
void AppMotorRun( uint8 motorParam );
void AppTimeUpdate( void );
uint8 AppHitpointUpdate( void );
uint8 AppBatteryUpdate( void );
void AppHistoryUpdate( void );
void AppDoubleClickHandler( void );
uint32 AppAccDataReadyHandler( void );
uint8 AppChargerStateChangeHandler( void );

#endif

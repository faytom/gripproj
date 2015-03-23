/***********************************************************
/   Filename:       Betwine.h
/   Revised:        2014-05-28
/   Revision:       v1.0
/
/   Description:    This module contains...
\**********************************************************/
#ifndef BETWINE_H
#define BETWINE_H

/***********************************************************
/                   Constants
\**********************************************************/
#define BET_START_DEVICE_EVT            0x0001
#define BET_TIME_UPDATE_EVT             0x0002

#define BET_WECHAT_AUTH_EVT             0x0004
#define BET_WECHAT_INIT_EVT             0x0008
#define BET_WECHAT_SEND_EVT             0x0010


#define BET_SYSTEM_STANDBY              0x00
#define BET_SYSTEM_ACTIVE               0x01

/***********************************************************
/                   Functions
\**********************************************************/
extern void Betwine_Init( uint8 task_id );
extern uint16 Betwine_ProcessEvent( uint8 task_id, uint16 events );
static void systemActive( void );
static void systemStandby( void );

#endif

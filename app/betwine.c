/***********************************************************
/   Filename:       Betwine.c
/   Revised:        2014-05-28
/   Revision:       v1.0
/
/   Description:    This module contains...
\**********************************************************/

/***********************************************************
/                   Includes
\**********************************************************/

#include <string.h>
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OSAL_Clock.h"

#include "OnBoard.h"
#include "hal_key.h"
#include "hal_led.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"

#include "hitpoint.h"
#include "pedometer.h"
#include "motorrun.h"
#include "timeset.h"
#include "battservice.h"
#include "historystep.h"
#include "systemtest.h"
#include "wechatprofile.h"

#include "peripheral.h"

#include "gapbondmgr.h"

#include "betwine.h"

#include "app.h"
#include "oad.h"
#include "epb_MmBp.h"
#include "epb.h"
#include "epb_WristBand.h"

/***********************************************************
/                   Constants
\**********************************************************/

//----------------- BLE PARAMETER --------------------------

/* Advertising Parameter */
// Advertising interval (units of 625us, 1208=755ms, 1704=1065ms)
#define DEFAULT_ADVERTISING_INTERVAL        1208
// Advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE           GAP_ADTYPE_FLAGS_GENERAL

/* Connection Parameter */
// Minimum connection interval (uints of 1.25ms, 80=100ms, 236=295ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL   80
// Maximum connection interval (uints of 1.25ms, 100=125ms, 256=320ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL   100
// Slave latency
#define DEFAULT_DESIRED_SLAVE_LATENCY       4
// Supervision timeout value (uints of 10ms, 600=6s)
#define DEFAULT_DESIRED_CONN_TIMEOUT        600
// Enable automatic parameter update
#define DEFAULT_ENABLE_UPDATE_REQUEST       TRUE
// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL       5

//----------------- APP PARAMETER --------------------------
#define BET_TIME_UPDATE_PERIOD              60000
#define BET_ACC_WATCHDOG_PERIOD             10000

//----------------- WECHAT PARAMETER -----------------------

#define BET_WECHAT_AUTH_PERIOD              3000
#define BET_WECHAT_SEND_PERIOD              30000

typedef struct
{
  uint8 todaySteps[STEP_LEN];
  uint8 hp;
  uint8 battery;
  uint8 historySteps[STEPS_LEN];
} Wristband_Data;

static uint16 wechatSeq;

static uint8 *AuthReqData = NULL;
static uint8 *AuthReqPbData = NULL;
static BpFixHead *AuthReqHead = NULL;
int AuthReqPbLen;

static uint8 *InitReqData = NULL;
static uint8 *InitReqPbData = NULL;
static BpFixHead *InitReqHead = NULL;
int InitReqPbLen;

static uint8 *SendReqData = NULL;
static uint8 *SendReqPbData = NULL;
static uint8 *WristbandPbData = NULL;
static uint8 *SendDataBuf = NULL;
static BpFixHead *SendReqHead = NULL;

int SendReqPbLen;

static uint8 *receiveData = NULL;
static uint8 *receiveOffset = NULL;
static uint8 receiveRemainder = 0;
static char Timestamp_wc[15];

static uint8 Mac_Address[B_ADDR_LEN];
char *device_name = "BETWINE";
char *timezone = "UTC+8";
char *language = "zh-CN";
static uint8 batteryLevel;

#pragma location="IMAGE_HEADER"
const __code img_hdr_t _imgHdr = {
    0xFFFF,                         // CRC-shadow must be 0xFFFF for everything else
    OAD_IMAGE_VERSION,              // 15-bit Version #, left-shifted 1; OR with Image-B/Not-A bit.
    OAD_IMG_B_AREA * OAD_FLASH_PAGE_MULT,
    OAD_IMAGE_B_USER_ID,            // User-Id
    { 0xFF, 0xFF, 0xFF, 0xFF }      // Reserved
};
#pragma required=_imgHdr

#pragma location="AES_HEADER"
static const __code aes_hdr_t _aesHdr = {
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //AES
 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  //MD5
};
#pragma required=_aesHdr


//----------------- SYSTEM PARAMETER -----------------------

// Company Identifier
#define REVISION_ID                         0x1101

#define INVALID_CONNHANDLE                  0xFFFF

/***********************************************************
/                   Global Variables
\**********************************************************/
static uint8 Betwine_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
    // Tx power level
    0x02,
    GAP_ADTYPE_POWER_LEVEL,
    0,      // 0dBm
    
    // complete name
    0x08,
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    0x42,   // 'B'
    0x45,   // 'E'
    0x54,   // 'T'
    0x57,   // 'W'
    0x49,   // 'I'
    0x4E,   // 'N'
    0x45,   // 'E'
    
    0x0C,
    GAP_ADTYPE_MANUFACTURER_SPECIFIC,
    HI_UINT16( REVISION_ID ),
    LO_UINT16( REVISION_ID ),
    0xFE,
    0x01,
    0x01,  
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
    // Flags
    0x02,
    GAP_ADTYPE_FLAGS,
    DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    
    // service UUID
    0x05,
    GAP_ADTYPE_16BIT_COMPLETE,
    LO_UINT16( WC_SERV_UUID ),
    HI_UINT16( WC_SERV_UUID ),
    LO_UINT16( PM_SERV_UUID ),
    HI_UINT16( PM_SERV_UUID ),
    
    // MAC address
    0x09,
    GAP_ADTYPE_MANUFACTURER_SPECIFIC,
    HI_UINT16( REVISION_ID ),
    LO_UINT16( REVISION_ID ),
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "BETWINE";
/***********************************************************
/                   Local Variables
\**********************************************************/

static uint8 systemState = BET_SYSTEM_STANDBY;

/***********************************************************
/                   Local Functions
\**********************************************************/


//----------------- SYSTEM FUNCTIONS -----------------------

// 系统消息处理
static void Betwine_ProcessOSALMsg( osal_event_hdr_t *pMsg );
// 中断按键处理
static void Betwine_HandleKeys( uint8 shift, uint8 keys );
// 链接状态改变处理
static void peripheralStateNotificationCB( gaprole_States_t newState );

//----------------- CALLBACK FUNCTION ----------------------

static void motorrunChangeCB( uint8 param );
static void timesetChangeCB( uint8 param[TIME_LEN] );

//----------------- APP FUNCTION ---------------------------


//----------------- WECHAT FUNCTION ---------------------------

static void wechatChangeCB( uint8 param[WC_DATA_LEN] );
//static void pulseLight( void ) ;
//----------------- WECHAT FUNCTION ---------------------------

static void wechatAuthReqDataInit( void );
static void wechatAuthReqDataFree( void );
static void wechatInitReqDataInit( void );
static void wechatInitReqDataFree( void );
static void wechatSendWristbandReqDataInit( void );
static void wechatSendReqDataInit( void );
static void wechatSendReqDataFree( void );
static void wechatSendData( uint8 *pData );
static void wechatReceiveData( uint8 *pData );
static void wechatDataDecode( uint8 *pData );


/***********************************************************
/                   PROFILE CALLBACKS
\**********************************************************/
// GAP Role Callbacks
static gapRolesCBs_t Betwine_PeripheralCBs =
{
    peripheralStateNotificationCB,  // Profile State Change Callbacks
    NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t Betwine_BondMgrCBs =
{
    NULL,                     // Passcode callback (not used by application)
    NULL                      // Pairing / Bonding state Callback (not used by application)
};

static mrProfileCBs_t motorrunProfileCBs =
{
    motorrunChangeCB
};

static tsProfileCBs_t timesetProfileCBs =
{
    timesetChangeCB
};

static wcProfileCBs_t wcProfileChangeCBs = 
{
  wechatChangeCB
};

/***********************************************************
/   fn      Betwine_Init
/
/   brief:  Initialization function for the Betwine App Task.
/
/   params: task_id - the ID assigned by OSAL.
/
/   return: none
\**********************************************************/
void Betwine_Init( uint8 task_id )
{
    Betwine_TaskID = task_id;

    // Setup the GAP
    VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
    // Setup the GAP Peripheral Role Profile
    {
        // For betwine hardware platforms, device starts advertising upon initialization
        uint8 initial_advertising_enable = FALSE;

        // By setting this to zero, the device will go into the waiting state after
        // being discoverable for 30.72 second, and will not being advertising again
        // until the enabler is set back to TRUE
        uint16 gapRole_AdvertOffTime = 0;

        // 链接参数设置(包括链接后时间间隔，超时时间等)
        uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
        uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
        uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
        uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
        uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;
        
        // Set the GAP Role Parameters
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
        GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

        GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
        GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );
    
        GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
        GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
        GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
        GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
        GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
    }

    // Set the GAP Characteristics
    GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );
        
    // Set advertising interval
    {
        uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;
        
        GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
        GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
        GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
        GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
    }

    // Setup the GAP Bond Manager
    {
        uint32 passkey = 0; // passkey "000000"
        uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
        uint8 mitm = TRUE;
        uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
        uint8 bonding = TRUE;
        GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
        GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
        GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
        GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
        GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
    }

    // Initialize GATT attributes
    GGS_AddService( GATT_ALL_SERVICES );            // GAP
    GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
    DevInfo_AddService();                           // Device Information Service
    
    HP_AddService( GATT_ALL_SERVICES );
    PM_AddService( GATT_ALL_SERVICES );
    MR_AddService( GATT_ALL_SERVICES );
    ST_AddService( GATT_ALL_SERVICES );
    TS_AddService( GATT_ALL_SERVICES );
    BT_AddService( GATT_ALL_SERVICES );
    HS_AddService( GATT_ALL_SERVICES );
    WC_AddService( GATT_ALL_SERVICES );
    
    // 中断按键回环函数注册
    RegisterForKeys( Betwine_TaskID );
    
    // I/O Configure
    HalLedSet( HAL_LED_ALL, HAL_LED_MODE_OFF );
    
    P0SEL = 0x00;
    P1SEL = 0x00;
    P2SEL = 0x00;
    
    P0DIR = 0x7F;
    P1DIR = 0x3F;
    P2DIR = 0x1F;
    
    P0 = 0x40;
    P1 = 0x00;
    P2 = 0x00;
    
    P0IEN |= 0x80;
    IEN1 |= 0x20;
    P0IFG = 0x7F;
    
    VOID MR_RegisterAppCBs( &motorrunProfileCBs );
    VOID TS_RegisterAppCBs( &timesetProfileCBs );
    VOID WC_RegisterAppCBs( &wcProfileChangeCBs );
       
    // Enable clock divide on halt
    // This reduces active current while radio is active and CC254x MCU
    // is halted
    HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

    // Setup a delayed profile startup
    osal_set_event( Betwine_TaskID, BET_START_DEVICE_EVT );
}

/***********************************************************
/   fn      Betwine_ProcessEvent
/
/   brief:  Betwine Application Task event processor.
/
/   params: task_id - The OSAL assigned task ID.
/           events - events to process.
/
/   return: events not processed
\**********************************************************/
uint16 Betwine_ProcessEvent( uint8 task_id, uint16 events )
{
    VOID task_id; // OSAL required parameter that isn't used in this function

    if ( events & SYS_EVENT_MSG )
    {
        uint8 *pMsg;

        if ( (pMsg = osal_msg_receive( Betwine_TaskID )) != NULL )
        {
            Betwine_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

            // Release the OSAL message
            VOID osal_msg_deallocate( pMsg );
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if ( events & BET_START_DEVICE_EVT )
    {
        // Start the Device
        VOID GAPRole_StartDevice( &Betwine_PeripheralCBs );

        // Start Bond Manager
        VOID GAPBondMgr_Register( &Betwine_BondMgrCBs );
        
        AppInit();
        
        systemState = BET_SYSTEM_STANDBY;

        osal_set_event( Betwine_TaskID, BET_TIME_UPDATE_EVT );
        
        uint8 timeArray[6] = {0, 9, 0, 8, 45, 23};
        
        TS_SetParameter(TS_VALUE, 6, timeArray);
        
        // return unprocessed events
        return ( events ^ BET_START_DEVICE_EVT );
    }
    
    if ( events & BET_TIME_UPDATE_EVT )
    {
        if ( BET_TIME_UPDATE_PERIOD )
        {
            osal_start_timerEx( Betwine_TaskID, BET_TIME_UPDATE_EVT, BET_TIME_UPDATE_PERIOD );
        }
        
        AppTimeUpdate();
        AppHistoryUpdate();
        
        if ( systemState == BET_SYSTEM_ACTIVE )
        {
            
            batteryLevel = AppBatteryUpdate();
            if ( batteryLevel == 0 )
            {
                systemStandby();
            }
            else
            {
                AppHitpointUpdate();
            }
        }
        
        return ( events ^ BET_TIME_UPDATE_EVT );
    }
    
    if ( events & BET_WECHAT_AUTH_EVT )
  {   
    
      if(gapProfileState == GAPROLE_CONNECTED)
      {
          
        if(wechatSeq == 1) 
        {
          wechatAuthReqDataInit();
          wechatSendData( AuthReqData );
          wechatAuthReqDataFree();
        }
        
        
        if ( BET_WECHAT_AUTH_PERIOD && wechatSeq == 1)
        { 
          osal_start_timerEx( Betwine_TaskID, BET_WECHAT_AUTH_EVT, BET_WECHAT_AUTH_PERIOD );
        }
        
      }
      
    return ( events ^ BET_WECHAT_AUTH_EVT );
  }
    
  if ( events & BET_WECHAT_INIT_EVT )
  {
       
      wechatInitReqDataInit();
      wechatSendData( InitReqData );
      wechatInitReqDataFree();

     return ( events ^ BET_WECHAT_INIT_EVT );
   }    
    
    if ( events & BET_WECHAT_SEND_EVT )
    {
      

      if(gapProfileState == GAPROLE_CONNECTED)
      {
        wechatSendWristbandReqDataInit();
        wechatSendData( SendReqData );
        wechatSendReqDataFree();
        wechatSeq++;
        
        wechatSendReqDataInit();
        wechatSendData( SendReqData );
        wechatSendReqDataFree();
        wechatSeq++;
        
        if(wechatSeq > 3)
        {
          osal_start_timerEx( Betwine_TaskID, BET_WECHAT_SEND_EVT, BET_WECHAT_SEND_PERIOD );
        }
        
      }

       
      return ( events ^ BET_WECHAT_SEND_EVT );
    }
    
    
    
    // Discard unknown events
    return 0;
}

/***********************************************************
/   fn      peripheralStateNotificationCB
/
/   brief:  Notification from the profile of a state change.
/
/   params: newState - newState
/
/   return: none
\**********************************************************/
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
    if (gapProfileState != newState)
    {    
        switch ( newState )
        {
            case GAPROLE_STARTED:
                {
                    //uint8 ownAddress[B_ADDR_LEN];
                    uint8 systemId[DEVINFO_SYSTEM_ID_LEN];
                    
                    GAPRole_GetParameter(GAPROLE_BD_ADDR, Mac_Address);

                    systemId[0] = HI_UINT16( REVISION_ID );
                    systemId[1] = LO_UINT16( REVISION_ID );
                    systemId[2] = Mac_Address[5];
                    systemId[3] = Mac_Address[4];
                    systemId[4] = Mac_Address[3];
                    systemId[5] = Mac_Address[2];
                    systemId[6] = Mac_Address[1];
                    systemId[7] = Mac_Address[0];
                    
                    advertData[13] = Mac_Address[5];
                    advertData[14] = Mac_Address[4];
                    advertData[15] = Mac_Address[3];
                    advertData[16] = Mac_Address[2];
                    advertData[17] = Mac_Address[1];
                    advertData[18] = Mac_Address[0];
                    
                    scanRspData[19] = Mac_Address[5];
                    scanRspData[20] = Mac_Address[4];
                    scanRspData[21] = Mac_Address[3];
                    scanRspData[22] = Mac_Address[2];
                    scanRspData[23] = Mac_Address[1];
                    scanRspData[24] = Mac_Address[0];
                    
                    uint8 *Mac_Address_Rev = scanRspData + 19;
                    
                    WC_SetParameter( WC_READ, 6, Mac_Address_Rev );
                    DevInfo_SetParameter(Betwine_TaskID, DEVINFO_SYSTEM_ID_LEN, systemId);
                    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
                    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );
                    
                    
                }
                break;
    
            case GAPROLE_ADVERTISING:
                {
                  
                }
                break;
    
            case GAPROLE_CONNECTED:
                {
                  wechatSeq = 1; 

                  osal_start_timerEx( Betwine_TaskID, BET_WECHAT_AUTH_EVT, BET_WECHAT_AUTH_PERIOD );
                }
                break;
    
            case GAPROLE_WAITING:
                {
                }
                break;
    
            case GAPROLE_WAITING_AFTER_TIMEOUT:
                {
                }
                break;
    
            case GAPROLE_ERROR:
                {
                }
                break;
    
            default:
                {
                }
                break;
        }
    }
    gapProfileState = newState;
}

/***********************************************************
/   fn      Betwine_ProcessOSALMsg
/
/   brief:  Process an incoming task message.
/
/   params: pMsg - message to process
/
/   return: none
\**********************************************************/
static void Betwine_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
    switch ( pMsg->event )
    {
        case KEY_CHANGE:
            Betwine_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
            break;
            
        default:
            // do nothing
            break;
    }
}

/***********************************************************
/   fn      Betwine_HandleKeys
/
/   brief:  Process Interrupt message
/
/   params: shift - isn't used
/           keys - Interrupt message
/
/   return: none
\**********************************************************/
static void Betwine_HandleKeys( uint8 shift, uint8 keys )
{
    VOID shift;
    
    if ((keys & HAL_CHR_INT) && (systemState == BET_SYSTEM_STANDBY))
    {
        systemActive();
    }
    
    if (keys & HAL_ACC_INT1)
    {
        AppAccDataReadyHandler();
        AppChargerStateChangeHandler();
    }
    
    if (keys & HAL_ACC_INT2)
    {
        AppDoubleClickHandler();
    }
}

static void systemStandby()
{
    uint8 new_adv_enabled_status;
    uint8 current_adv_enabled_status;
    
    if ( gapProfileState == GAPROLE_CONNECTED )    
        GAPRole_TerminateConnection();
    
    //Find the current GAP advertisement status
    GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );
    
    if ( current_adv_enabled_status == TRUE )
    {
        new_adv_enabled_status = FALSE;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
    }
    
    AppStandby();
    
    systemState = BET_SYSTEM_STANDBY;
}

static void systemActive()
{
    uint8 new_adv_enabled_status;
    uint8 current_adv_enabled_status;
    
    //Find the current GAP advertisement status
    GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );
    
    if ( current_adv_enabled_status == FALSE )
    {
        new_adv_enabled_status = TRUE;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
    }
    
    AppActive();
    
    systemState = BET_SYSTEM_ACTIVE;
}

static void motorrunChangeCB( uint8 param )
{
    if (param == 0xE0)
    {
        HAL_SYSTEM_RESET();
    }
    
    if (param == 0xC0)
    {
        systemStandby();
    }

    if (!(param & 0xC0))
    {
        AppMotorRun(param);
    }
}

static void timesetChangeCB( uint8 param[TIME_LEN] )
{
    AppTimeSet(param);
}


// wechat

static void wechatChangeCB( uint8 param[WC_DATA_LEN] )
{
    wechatReceiveData( param );
}

static void wechatAuthReqDataInit()
{
    wechatAuthReqDataFree();
    
    // AuthRequest Protobuf Data
    //struct AuthRequest AuthReqMessage;
    //uint8 *AuthReqPbData;
    //uint8 AuthReqPbLen;
    
    AuthRequest *request = (AuthRequest *)osal_mem_alloc(sizeof(AuthRequest));
    //AuthRequest request;
    //request->md5_device_type_and_device_id.data = md5_type_id;
    //request->md5_device_type_and_device_id.len = sizeof(md5_type_id);
    request->base_request = NULL;
    request->has_md5_device_type_and_device_id = FALSE;
    request->md5_device_type_and_device_id.data = NULL;
    request->md5_device_type_and_device_id.len = 0;
    request->has_mac_address = TRUE;
    uint8 *Mac_Address_Rev = scanRspData + 19;
    request->mac_address.data = Mac_Address_Rev;
    request->mac_address.len = sizeof(Mac_Address);
    request->proto_version = 0x010004;
    request->auth_proto = 0x01;
    request->auth_method = EAM_macNoEncrypt;
    //no encrytion therefore, aes_sign data = null, and len = 0;
    request->has_aes_sign = FALSE;
    request->aes_sign.data = NULL;
    request->aes_sign.len = 0;

    request->has_time_zone = true;
    request->time_zone.str = timezone;
    request->time_zone.len = strlen(timezone);

    request->has_language = true;
    request->language.str = language;
    request->language.len = strlen(language);

    request->has_device_name = true;
    request->device_name.str = device_name;
    request->device_name.len = strlen(device_name);
    
    AuthReqPbLen = epb_auth_request_pack_size(request);
    AuthReqPbData = (uint8 *) osal_mem_alloc(AuthReqPbLen);
    epb_pack_auth_request(request, AuthReqPbData, AuthReqPbLen);
    osal_mem_free( (void *)request );

    //BpFixHeader difinition for Auth Request-------------------------
    
    AuthReqHead = (BpFixHead *)osal_mem_alloc(WC_HEAD_LEN);
    if (AuthReqHead != NULL)
    {
        uint16 Length = AuthReqPbLen + WC_HEAD_LEN;
        uint16 CmdId = ECI_req_auth;
        uint16 Seq = wechatSeq;
        AuthReqHead->bMagicNumber = 0xFE;
        AuthReqHead->bVer = 0x01;
        AuthReqHead->nLength_h = HI_UINT16(Length);
        AuthReqHead->nLength_l = LO_UINT16(Length);
        AuthReqHead->nCmdId_h = HI_UINT16(CmdId);
        AuthReqHead->nCmdId_l = LO_UINT16(CmdId);
        AuthReqHead->nSeq_h = HI_UINT16(Seq);
        AuthReqHead->nSeq_l = LO_UINT16(Seq);
    }    
    //---------------------------------------------------------------------------------------------------------
    
    AuthReqData = (uint8 *)osal_mem_alloc(AuthReqPbLen + WC_HEAD_LEN);
    

    VOID osal_memcpy(AuthReqData, AuthReqHead, WC_HEAD_LEN);
    osal_mem_free( (void *)AuthReqHead );
    VOID osal_memcpy(AuthReqData + WC_HEAD_LEN, AuthReqPbData, AuthReqPbLen);
    osal_mem_free( (void *)AuthReqPbData );

    //clear temp head data and protobuffer data
    
    
    
}

static void wechatAuthReqDataFree()
{
    if (AuthReqData != NULL)
    {
        osal_mem_free((void *)AuthReqData);
        AuthReqData = NULL;
    }
}

static void wechatInitReqDataInit()
{
    wechatInitReqDataFree();
    
    InitRequest *InitReqMessage = (InitRequest*) osal_mem_alloc(sizeof(InitRequest));

    uint8 random[] = {0x0a, 0x02, 0x08, 0x00};     
    uint8* field_filter = osal_mem_alloc(sizeof(uint8));
    *field_filter = 0x40;
    InitReqMessage->has_resp_field_filter = TRUE;
    InitReqMessage->resp_field_filter.len = sizeof(uint8);
    InitReqMessage->resp_field_filter.data = field_filter;
    InitReqMessage->has_challenge = TRUE;
    InitReqMessage->challenge.data = random;
    InitReqMessage->challenge.len = 4;
    
    InitReqPbLen = epb_init_request_pack_size(InitReqMessage);
    InitReqPbData = (uint8 *)osal_mem_alloc(InitReqPbLen);
    epb_pack_init_request(InitReqMessage, InitReqPbData, InitReqPbLen);
    osal_mem_free( (void *) field_filter );
    osal_mem_free( (void *) InitReqMessage );
    InitReqHead = (BpFixHead *)osal_mem_alloc(WC_HEAD_LEN);
    if (InitReqHead != NULL)
    {
        uint16 Length = InitReqPbLen + WC_HEAD_LEN;
        uint16 CmdId = ECI_req_init;
        uint16 Seq = wechatSeq;
        InitReqHead->bMagicNumber = 0xFE;
        InitReqHead->bVer = 0x01;
        InitReqHead->nLength_h = HI_UINT16(Length);
        InitReqHead->nLength_l = LO_UINT16(Length);
        InitReqHead->nCmdId_h = HI_UINT16(CmdId);
        InitReqHead->nCmdId_l = LO_UINT16(CmdId);
        InitReqHead->nSeq_h = HI_UINT16(Seq);
        InitReqHead->nSeq_l = LO_UINT16(Seq);
    }    
    
    InitReqData = (uint8 *)osal_mem_alloc(InitReqPbLen + WC_HEAD_LEN);

    VOID osal_memcpy(InitReqData, InitReqHead, WC_HEAD_LEN);
    osal_mem_free( (void *)InitReqHead );
    VOID osal_memcpy(InitReqData + WC_HEAD_LEN, InitReqPbData, InitReqPbLen);
    osal_mem_free( (void *)InitReqPbData );

    
    
    
}

static void wechatInitReqDataFree()
{
    if (AuthReqData != NULL)
    {
        osal_mem_free( (void *)InitReqData );
        InitReqData = NULL;
    }    
}

static void wechatSendWristbandReqDataInit()
{  
    wechatSendReqDataFree();
    
    MMOpen_WristbandRequest *request = (MMOpen_WristbandRequest*) osal_mem_alloc(sizeof(MMOpen_WristbandRequest));
    //osal_memset(request, 0, sizeof(MMOpen_WristbandRequest));
    request->step_data_count = 1;
    request->step_data  = (MMOpen_StepDataItem*) osal_mem_alloc(sizeof(MMOpen_StepDataItem));
    
    request->step_data->step = realtimeStep;
    request->step_data->has_timestamp = FALSE;
    request->step_data->timestamp = 0;    
    request->step_data->has_rtc_day = FALSE;
    request->step_data->rtc_day = 0;
    request->step_data->has_rtc_hour= FALSE;
    request->step_data->rtc_hour = 0;
    request->step_data->has_rtc_minute = FALSE;
    request->step_data->rtc_minute = 0;
    request->step_data->has_rtc_month = FALSE;
    request->step_data->rtc_month = 0;
    request->step_data->has_rtc_second = FALSE;
    request->step_data->rtc_second = 0;
    request->step_data->has_rtc_year = FALSE;
    request->step_data->rtc_year = 0;
    
    //uint8 HP_Value;
    //HP_GetParameter( HP_VALUE, &HP_Value );
    //uint8 data[2];
    //data[0] = HP_Value;
    //data[1] = batteryLevel;
      
    request->has_ext_data = FALSE;
    request->ext_data.len = 0;
    request->ext_data.data = NULL;
    
    SendReqPbLen = epb_mmopen_wristband_request_pack_size(request);

    
    WristbandPbData = (uint8 *) osal_mem_alloc(SendReqPbLen);
    epb_mmopen_pack_wristband_request(request, WristbandPbData, SendReqPbLen);
    osal_mem_free( (void *) request->step_data );
    osal_mem_free( (void *) request );
    
    SendDataRequest *SendReqMessage = (SendDataRequest*) osal_mem_alloc(sizeof(SendDataRequest));
    SendReqMessage->base_request = NULL;
    SendReqMessage->data.data = WristbandPbData;
    SendReqMessage->data.len = SendReqPbLen;
    SendReqMessage->has_type = TRUE;
    SendReqMessage->type = EDDT_wxWristBand;
    
    SendReqPbLen = epb_send_data_request_pack_size(SendReqMessage);
    SendReqPbData = (uint8 *)osal_mem_alloc(SendReqPbLen);
    epb_pack_send_data_request(SendReqMessage, SendReqPbData, SendReqPbLen);
    osal_mem_free( (void *) WristbandPbData );
    osal_mem_free( (void *) SendReqMessage );
    
    
    SendReqHead = (BpFixHead *)osal_mem_alloc(WC_HEAD_LEN);
    uint16 Length = SendReqPbLen + WC_HEAD_LEN;
    uint16 CmdId = ECI_req_sendData;
    uint16 Seq = wechatSeq;
    SendReqHead->bMagicNumber = 0xFE;
    SendReqHead->bVer = 0x01;
    SendReqHead->nLength_h = HI_UINT16(Length);
    SendReqHead->nLength_l = LO_UINT16(Length);
    SendReqHead->nCmdId_h = HI_UINT16(CmdId);
    SendReqHead->nCmdId_l = LO_UINT16(CmdId);
    SendReqHead->nSeq_h = HI_UINT16(Seq);
    SendReqHead->nSeq_l = LO_UINT16(Seq);
    
    SendReqData = (uint8 *)osal_mem_alloc(Length);
    
    VOID osal_memcpy(SendReqData, SendReqHead, WC_HEAD_LEN);
    osal_mem_free( (void *)SendReqHead );
    VOID osal_memcpy(SendReqData + WC_HEAD_LEN, SendReqPbData, SendReqPbLen);
    osal_mem_free( (void *)SendReqPbData );
    
    
    
}


static void wechatSendReqDataInit()
{
    wechatSendReqDataFree();
    
    SendDataRequest *SendReqMessage = (SendDataRequest*) osal_mem_alloc(sizeof(SendDataRequest));
    //uint8 *SendReqPbData;
    //uint8  SendReqPbLen;
    
    //initiate sending grips data
    //---------------------------------------------------------------------------
    
    Wristband_Data *SendWristbandData = (Wristband_Data*) osal_mem_alloc(sizeof(Wristband_Data));
    
    
    SendDataBuf = (uint8 *) osal_mem_alloc(sizeof(Wristband_Data));
        
    SendWristbandData->todaySteps[0] = (realtimeStep & 0xFF);
    SendWristbandData->todaySteps[1] = ((realtimeStep >> 8) & 0xFF);
    SendWristbandData->todaySteps[2] = ((realtimeStep >> 16) & 0xFF);

    uint8 _historySteps[STEPS_LEN];
    HS_GetParameter( HS_DAY, _historySteps );
    VOID osal_memcpy(SendWristbandData->historySteps, _historySteps, STEPS_LEN);
    
    uint8 batterylevel = 100;
    BT_GetParameter(BT_VALUE, &batterylevel);
    if(batterylevel >= 99) 
      batterylevel = 99;
    SendWristbandData->battery = batterylevel;
    
    uint8 HP = 5;
    HP_GetParameter(HP_VALUE, &HP);
    SendWristbandData->hp = HP;
    

      VOID osal_memcpy(SendDataBuf, SendWristbandData, sizeof(Wristband_Data));

      //VOID osal_memcpy(SendDataBuf, SendWristbandData, sizeof(SendWristbandData));
  

    osal_mem_free((void*) SendWristbandData);

    
    //---------------------------------------------------------------------------
    SendReqMessage->base_request = NULL;
    SendReqMessage->data.data = SendDataBuf;
    
    if(wechatSeq >= 6) 
    {
      SendReqMessage->data.len = sizeof(Wristband_Data)-STEPS_LEN;
    }
    else
    {
      SendReqMessage->data.len = sizeof(Wristband_Data);
    }
    
    SendReqMessage->has_type = TRUE;
    SendReqMessage->type = EDDT_manufatureSvr;
    
    SendReqPbLen = epb_send_data_request_pack_size(SendReqMessage);
    SendReqPbData = (uint8 *)osal_mem_alloc(SendReqPbLen);
    epb_pack_send_data_request(SendReqMessage, SendReqPbData, SendReqPbLen);
    osal_mem_free( (void *) SendDataBuf );
    osal_mem_free( (void *) SendReqMessage );
    
    SendReqHead = (BpFixHead *)osal_mem_alloc(WC_HEAD_LEN);
    uint16 Length = SendReqPbLen + WC_HEAD_LEN;
    uint16 CmdId = ECI_req_sendData;
    uint16 Seq = wechatSeq;
    SendReqHead->bMagicNumber = 0xFE;
    SendReqHead->bVer = 0x01;
    SendReqHead->nLength_h = HI_UINT16(Length);
    SendReqHead->nLength_l = LO_UINT16(Length);
    SendReqHead->nCmdId_h = HI_UINT16(CmdId);
    SendReqHead->nCmdId_l = LO_UINT16(CmdId);
    SendReqHead->nSeq_h = HI_UINT16(Seq);
    SendReqHead->nSeq_l = LO_UINT16(Seq);
    
    SendReqData = (uint8 *)osal_mem_alloc(Length);
    
    if ( SendReqData != NULL )
    {
      VOID osal_memcpy(SendReqData, SendReqHead, WC_HEAD_LEN);
      if ( SendReqHead != NULL )
      {
        osal_mem_free( (void *)SendReqHead );
      }
      
      VOID osal_memcpy(SendReqData + WC_HEAD_LEN, SendReqPbData, SendReqPbLen);
      if ( SendReqPbData != NULL )
      {
        osal_mem_free( (void *)SendReqPbData );
      }
    }
    
    
}

static void wechatSendReqDataFree()
{
    if (SendReqData != NULL)
    {
        osal_mem_free((void *)SendReqData);
        SendReqData = NULL;
    }    
}

static void wechatSendData( uint8 *pData )
{
    uint16 sendDataLen;
    uint8 sendData[WC_DATA_LEN];
    
    sendDataLen = BUILD_UINT16(pData[3], pData[2]);
    
    while (sendDataLen > 0)
    {
        if (sendDataLen > WC_DATA_LEN)
        {
            VOID osal_memcpy(sendData, pData, WC_DATA_LEN);
            
            pData += WC_DATA_LEN;
            sendDataLen -= WC_DATA_LEN;
        }
        else
        {
            VOID osal_memset(sendData, 0, WC_DATA_LEN);
            VOID osal_memcpy(sendData, pData, sendDataLen);
            
            sendDataLen = 0;
        }    
        
        if(WC_SetParameter( WC_INDICATE, WC_DATA_LEN, sendData ) == SUCCESS)
        {
          //HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);
        }
    }
}

static void wechatReceiveData( uint8 *pData )
{
    if (receiveData == NULL)
    {
        receiveRemainder = BUILD_UINT16(pData[3], pData[2]);
        receiveData = (uint8 *)osal_mem_alloc(receiveRemainder);
        receiveOffset = receiveData;
    }    
    
    if (receiveRemainder > WC_DATA_LEN)
    {
        VOID osal_memcpy(receiveOffset, pData, WC_DATA_LEN);
        
        receiveOffset += WC_DATA_LEN;
        receiveRemainder -= WC_DATA_LEN;
    }
    else
    {
        VOID osal_memcpy(receiveOffset, pData, receiveRemainder);
        
        wechatDataDecode(receiveData);
        
        osal_mem_free( (void *)receiveData );
        receiveData = NULL;
        receiveOffset = NULL;
        receiveRemainder = 0;
    }    
}



static void wechatDataDecode( uint8 *pData )
{
    BpFixHead *wechatHead;
    wechatHead = (BpFixHead *)pData;
    
    //uint8 MagicNumber = wechatHead->bMagicNumber;
    //uint8 Ver = wechatHead->bVer;
    uint16 Length = BUILD_UINT16(wechatHead->nLength_l,
                                wechatHead->nLength_h);
    uint16 CmdId = BUILD_UINT16(wechatHead->nCmdId_l,
                                wechatHead->nCmdId_h);
    uint16 Seq = BUILD_UINT16(wechatHead->nSeq_l,
                              wechatHead->nSeq_h);

    uint8 *wechatPbData;
    wechatPbData = pData + WC_HEAD_LEN;
    //wechatPbData[0] = (uint8)(Length - WC_HEAD_LEN);
    
    
    switch (CmdId)
    {
        case ECI_resp_auth:
          
            if (Seq == 1)
            {
                AuthResponse *AuthResp = epb_unpack_auth_response(wechatPbData, (int)(Length-WC_HEAD_LEN));
                
                //HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);
                
                if(AuthResp != NULL) 
                {
                
                                
                  if (AuthResp->base_response->has_err_msg == 0 || ( AuthResp->base_response->has_err_msg && AuthResp->base_response->err_code == 0))
                  {
                      wechatSeq++;
                      //HalLedSet(HAL_LED_2, HAL_LED_MODE_FLASH);
                      osal_stop_timerEx( Betwine_TaskID, BET_WECHAT_AUTH_EVT );
                      
                      osal_set_event( Betwine_TaskID, BET_WECHAT_INIT_EVT );
                  }
                  else
                  {
                    GAPRole_TerminateConnection();
                  }
                  
                  epb_unpack_auth_response_free(AuthResp);
                  
                }

                
                
            }
            break;
            
        case ECI_resp_init:
            if (Seq == wechatSeq)
            {
                InitResponse *InitResp = epb_unpack_init_response(wechatPbData, (int)(Length-WC_HEAD_LEN));
                
                if(InitResp != NULL) 
                {
                  if (InitResp->base_response->has_err_msg == 0 || ( InitResp->base_response->has_err_msg && InitResp->base_response->err_code == 0))
                  {
                   
                    if(InitResp->has_time_string)
                    {
                      osal_memcpy(Timestamp_wc, InitResp->time_string.str, InitResp->time_string.len);
                      uint8 timeset[6];
                      TS_GetParameter( TS_VALUE, timeset);
                      timeset[0] = (int)(Timestamp_wc[10]-48)*10 + (int)(Timestamp_wc[11]-48);
                      timeset[1] = (int)(Timestamp_wc[8]-48)*10 + (int)(Timestamp_wc[9]-48);
                      AppTimeSet(timeset);
                      
                    }
                    //osal_stop_timerEx( GripGame_TaskID, BET_WECHAT_AUTH_EVT );
                    wechatSeq++;
                    osal_set_event( Betwine_TaskID, BET_WECHAT_SEND_EVT );
                  }
                  else
                  {
                    
                    //uint8 temp[6];
                    //temp[0] =  InitResp->time & 0xFF;
                    //temp[1] =  (InitResp->time >> 8) & 0xFF;
                    //temp[2] =  (InitResp->time >> 16) & 0xFF;
                    //temp[3] =  (InitResp->time >> 24) & 0xFF;
                    //temp[4] = 0xf1;
                    //temp[5] = 0xf1;
                    //WC_SetParameter(WC_READ, 6, temp);
                    
                    GAPRole_TerminateConnection();
                    //osal_start_timerEx( GripGame_TaskID, BET_WECHAT_AUTH_EVT, BET_WECHAT_AUTH_PERIOD );
                  }
                  epb_unpack_init_response_free(InitResp);
                }
            }
            else
            {
              GAPRole_TerminateConnection();  
              //osal_start_timerEx( GripGame_TaskID, BET_WECHAT_AUTH_EVT, BET_WECHAT_AUTH_PERIOD );
            }
            
            
            break;
        
        case ECI_resp_sendData:
            if (Seq == wechatSeq)
            {
                SendDataResponse *SendResp = epb_unpack_send_data_response(wechatPbData, (int)(Length-WC_HEAD_LEN));;
        
                if(SendResp != NULL) 
                {
                  if (SendResp->base_response->has_err_msg == 0 || ( SendResp->base_response->has_err_msg && SendResp->base_response->err_code == 0))
                  {
                      
                      //osal_start_timerEx( GripGame_TaskID, BET_WECHAT_SEND_EVT, BET_WECHAT_SEND_PERIOD );
                    //wechatConnected = TRUE;
                    //osal_stop_timerEx( GripGame_TaskID, BET_WECHAT_AUTH_EVT );
                  }
                  else 
                  {
                    GAPRole_TerminateConnection();
                  }

                  epb_unpack_send_data_response_free(SendResp);
                }
                
            }
            break;
            
        case ECI_push_recvData:
            if (Seq == 0)
            {
                RecvDataPush *ServerPushMessage = epb_unpack_recv_data_push(wechatPbData, (int)(Length-WC_HEAD_LEN));
                
                if( ServerPushMessage->has_type == TRUE)
                {
                  if( ServerPushMessage->type == EDDT_manufatureSvr)
                  {
                    //HalLedSet(HAL_LED_1, HAL_LED_MODE_FLASH);
                  }   
                  
                  if( ServerPushMessage->type == EDDT_wxDeviceHtmlChatView)
                  {

                    if(*ServerPushMessage->data.data && HAL_MOTOR)
                    {
                      AppMotorRun(HAL_MOTOR);
                    }
                    
                    
                    uint8 timeArray[6] = {0,0,0,0,0,0};
                    TS_GetParameter(TS_VALUE, timeArray);
                    timeArray[2] = *(ServerPushMessage->data.data + 1);
                    timeArray[3] = *(ServerPushMessage->data.data + 2);
                    timeArray[4] = *(ServerPushMessage->data.data + 3); 
                    timeArray[5] = *(ServerPushMessage->data.data + 4);
                    TS_SetParameter(TS_VALUE, 6, timeArray);
                    AppTimeSet(timeArray);
                  }  
                }

                epb_unpack_recv_data_push_free( ServerPushMessage );
            }
            break;
            
        case ECI_push_switchView:
            if (Seq == 0)
            {
              //HalLedSet(HAL_LED_2, HAL_LED_MODE_FLASH);
            }    
            break;
            
        case ECI_push_switchBackgroud:
            if (Seq == 0)
            {
              //HalLedSet(HAL_LED_2, HAL_LED_MODE_FLASH);
            }    
            break;
            
        case ECI_err_decode:
            
        default:
            break;
    }
}

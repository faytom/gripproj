#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "pedometer.h"

#define SERVAPP_NUM_ATTR_SUPPORTED      9

CONST uint8 pmServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PM_SERV_UUID),
    HI_UINT16(PM_SERV_UUID)
};

CONST uint8 pmStateUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PM_STATE_UUID),
    HI_UINT16(PM_STATE_UUID)
};

CONST uint8 pmValueUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(PM_VALUE_UUID),
    HI_UINT16(PM_VALUE_UUID)
};

static CONST gattAttrType_t pmService = { ATT_BT_UUID_SIZE, pmServUUID };

static uint8 pmStateProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

static uint8 pmState = 0;

static gattCharCfg_t pmStateConfig[GATT_MAX_NUM_CONN];

static uint8 pmStateUserDesp[13] = "motion state\0";

static uint8 pmValueProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

static uint8 pmValue[STEP_LEN] = {0, 0, 0};

static gattCharCfg_t pmValueConfig[GATT_MAX_NUM_CONN];

static uint8 pmValueUserDesp[14] = "motion number\0";

static gattAttribute_t pedometerAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *)&pmService
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &pmStateProps
    },
    
    {
        { ATT_BT_UUID_SIZE, pmStateUUID },
        GATT_PERMIT_READ,
        0,
        &pmState
    },
    
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)pmStateConfig
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        pmStateUserDesp
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &pmValueProps
    },
    
    {
        { ATT_BT_UUID_SIZE, pmValueUUID },
        GATT_PERMIT_READ,
        0,
        pmValue
    },
    
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)pmValueConfig
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        pmValueUserDesp
    }
};

static uint8 PM_ReadAttrCB( uint16 connHandle, 
                            gattAttribute_t *pAttr, 
                            uint8 *pValue, 
                            uint8 *pLen, 
                            uint16 offset, 
                            uint8 maxLen );
                            
static bStatus_t PM_WriteAttrCB( uint16 connHandle, 
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue, 
                                 uint8 len, 
                                 uint16 offset );

static void PM_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

CONST gattServiceCBs_t pmCBs =
{
    PM_ReadAttrCB,
    PM_WriteAttrCB,
    NULL
};

bStatus_t PM_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, pmStateConfig );
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, pmValueConfig );
    
    VOID linkDB_Register( PM_HandleConnStatusCB );
    
    if ( services & PM_SERVICE )
    {
        status = GATTServApp_RegisterService( pedometerAttrTbl,
                                              GATT_NUM_ATTRS( pedometerAttrTbl ),
                                              &pmCBs );
    }
    
    return ( status );
}

bStatus_t PM_SetParameter( uint8 param, uint8 len, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case PM_STATE:
            if (len == sizeof(uint8))
            {
                pmState = *((uint8 *)pValue);
                
                GATTServApp_ProcessCharCfg( pmStateConfig,
                                           &pmState,
                                            FALSE,
                                            pedometerAttrTbl,
                                            GATT_NUM_ATTRS(pedometerAttrTbl),
                                            INVALID_TASK_ID );
            }    
            else
            {
                ret = bleInvalidRange;
            }
        
        case PM_VALUE:
            if (len == STEP_LEN)
            {
                VOID osal_memcpy( pmValue, pValue, STEP_LEN );
                
                GATTServApp_ProcessCharCfg( pmValueConfig,
                                            pmValue,
                                            FALSE,
                                            pedometerAttrTbl,
                                            GATT_NUM_ATTRS(pedometerAttrTbl),
                                            INVALID_TASK_ID ); 
            }    
            else
            {
                ret = bleInvalidRange;
            }
            
        default:
            ret = INVALIDPARAMETER;
            break;    
    }
    
    return (ret);
}

bStatus_t PM_GetParameter( uint8 param, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case PM_STATE:
            *((uint8 *)pValue) = pmState;
            break;
            
        case PM_VALUE:
            VOID osal_memcpy( pValue, pmValue, STEP_LEN );
            break;
            
        default:
            ret = INVALIDPARAMETER;
            break;    
    }
    
    return (ret);
}

static uint8 PM_ReadAttrCB( uint16 connHandle,
                            gattAttribute_t *pAttr,
                            uint8 *pValue,
                            uint8 *pLen,
                            uint16 offset,
                            uint8 maxLen )
{
    bStatus_t status = SUCCESS;
    
    if (offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }
    
    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        
        switch (uuid)
        {
            case PM_STATE_UUID:
                *pLen = 1;
                pValue[0] = *pAttr->pValue;
                break;
            
            case PM_VALUE_UUID:
                *pLen = STEP_LEN;
                VOID osal_memcpy( pValue, pAttr->pValue, STEP_LEN );
                break;
            
            default:
                *pLen = 0;
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;    
        }
    }
    else
    {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }    
    
    return (status);
}

static bStatus_t PM_WriteAttrCB( uint16 connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue,
                                 uint8 len,
                                 uint16 offset )
{
    bStatus_t status = SUCCESS;
    
    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch (uuid)
        {
            case GATT_CLIENT_CHAR_CFG_UUID:
                status = GATTServApp_ProcessCCCWriteReq( connHandle,
                                                         pAttr,
                                                         pValue,
                                                         len,
                                                         offset,
                                                         GATT_CLIENT_CFG_NOTIFY );
                break;
                
            default:
                status = ATT_ERR_ATTR_NOT_FOUND;
                break;
        }
    }    
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }    
    
    return ( status );
}

static void PM_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    if (connHandle != LOOPBACK_CONNHANDLE )
    {
        if ( (changeType == LINKDB_STATUS_UPDATE_REMOVED)    ||
            ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
             (!linkDB_Up( connHandle ))))
        {
            GATTServApp_InitCharCfg( connHandle, pmStateConfig );
            GATTServApp_InitCharCfg( connHandle, pmValueConfig );
        }     
    }    
}

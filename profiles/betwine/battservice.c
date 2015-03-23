#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "battservice.h"

#define SERVAPP_NUM_ATTR_SUPPORTED          5

CONST uint8 btServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(BT_SERV_UUID),
    HI_UINT16(BT_SERV_UUID)
};

CONST uint8 btValueUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(BT_VALUE_UUID),
    HI_UINT16(BT_VALUE_UUID)
};

static CONST gattAttrType_t btService = { ATT_BT_UUID_SIZE, btServUUID };

static uint8 btProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

static uint8 btValue = 50;

static gattCharCfg_t btConfig[GATT_MAX_NUM_CONN];

static uint8 btUserDesp[8] = "battery\0";

static gattAttribute_t batteryAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *)&btService
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &btProps
    },
    
    {
        { ATT_BT_UUID_SIZE, btValueUUID },
        GATT_PERMIT_READ,
        0,
        &btValue
    },
    
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)btConfig
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        btUserDesp
    }
};

static uint8 BT_ReadAttrCB( uint16 connHandle,
                            gattAttribute_t *pAttr,
                            uint8 *pValue,
                            uint8 *pLen,
                            uint16 offset,
                            uint8 maxLen );

static bStatus_t BT_WriteAttrCB( uint16 connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue,
                                 uint8 len,
                                 uint16 offset );

static void BT_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

CONST gattServiceCBs_t btCBs =
{
    BT_ReadAttrCB,
    BT_WriteAttrCB,
    NULL
};

bStatus_t BT_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, btConfig );
    
    VOID linkDB_Register( BT_HandleConnStatusCB );
    
    if ( services & BT_SERVICE )
    {
        status = GATTServApp_RegisterService( batteryAttrTbl,
                                              GATT_NUM_ATTRS( batteryAttrTbl ),
                                              &btCBs );
    }
    
    return ( status );
}

bStatus_t BT_SetParameter( uint8 param, uint8 len, void *pValue )
{
    bStatus_t ret = SUCCESS;
    
    switch (param)
    {
        case BT_VALUE:
            if (len == sizeof(uint8))
            {
                btValue = *((uint8 *)pValue);
                
                GATTServApp_ProcessCharCfg( btConfig,
                                           &btValue,
                                            FALSE,
                                            batteryAttrTbl,
                                            GATT_NUM_ATTRS(batteryAttrTbl),
                                            INVALID_TASK_ID );
            }
            else
            {
                ret = bleInvalidRange;
            }
            break;
            
        default:
            ret = INVALIDPARAMETER;
            break;    
    }
    
    return ( ret );
}

bStatus_t BT_GetParameter( uint8 param, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case BT_VALUE:
            *((uint8 *)pValue) = btValue;
            break;
            
        default:
            ret = INVALIDPARAMETER;
            break;    
    }
    
    return ( ret );
}

static uint8 BT_ReadAttrCB( uint16 connHandle,
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
            case BT_VALUE_UUID:
                *pLen = 1;
                pValue[0] = *pAttr->pValue;
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

static bStatus_t BT_WriteAttrCB( uint16 connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue,
                                 uint8 len,
                                 uint16 offset )
{
    bStatus_t status = SUCCESS;
    
    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
        
        switch ( uuid )
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

static void BT_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    if (connHandle != LOOPBACK_CONNHANDLE )
    {
        if ( (changeType == LINKDB_STATUS_UPDATE_REMOVED)       ||
            ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS)    &&
             (!linkDB_Up( connHandle ))))
        {
            GATTServApp_InitCharCfg( connHandle, btConfig );
        }     
    }    
}

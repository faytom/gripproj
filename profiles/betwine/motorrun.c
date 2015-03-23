#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "motorrun.h"

#define SERVAPP_NUM_ATTR_SUPPORTED      8

CONST uint8 mrServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MR_SERV_UUID),
    HI_UINT16(MR_SERV_UUID)
};

CONST uint8 mrValueUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MR_VALUE_UUID),
    HI_UINT16(MR_VALUE_UUID)
};

CONST uint8 mrStampUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(MR_STAMP_UUID),
    HI_UINT16(MR_STAMP_UUID)
};

static mrProfileCBs_t *mrProfile_AppCBs = NULL;

static CONST gattAttrType_t mrService = { ATT_BT_UUID_SIZE, mrServUUID };

static uint8 mrValueProps = GATT_PROP_WRITE;

static uint8 mrValue = 0;

static uint8 mrUserDesp[10] = "motor run\0";

static uint8 mrStampProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

static uint8 mrStamp[MR_STAMP_LEN] = {0, 0, 0, 0};

static gattCharCfg_t mrStampConfig[GATT_MAX_NUM_CONN];

static uint8 mrStampUserDesp[17] = "motor time stamp\0";

static gattAttribute_t motorrunAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *)&mrService
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &mrValueProps
    },
    
    {
        { ATT_BT_UUID_SIZE, mrValueUUID },
        GATT_PERMIT_WRITE,
        0,
        &mrValue
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        mrUserDesp
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &mrStampProps
    },
    
    {
        { ATT_BT_UUID_SIZE, mrStampUUID  },
        GATT_PERMIT_READ,
        0,
        mrStamp
    },
    
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)mrStampConfig
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        mrStampUserDesp
    }
};

static uint8 MR_ReadAttrCB( uint16 connHandle, 
                            gattAttribute_t *pAttr, 
                            uint8 *pValue, 
                            uint8 *pLen, 
                            uint16 offset, 
                            uint8 maxLen );
                            
static bStatus_t MR_WriteAttrCB( uint16 connHandle, 
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue, 
                                 uint8 len, 
                                 uint16 offset );

static void MR_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

CONST gattServiceCBs_t mrCBs =
{
    MR_ReadAttrCB,
    MR_WriteAttrCB,
    NULL
};

bStatus_t MR_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, mrStampConfig );
    
    VOID linkDB_Register( MR_HandleConnStatusCB );
    
    if ( services & MR_SERVICE )
    {
        status = GATTServApp_RegisterService( motorrunAttrTbl,
                                              GATT_NUM_ATTRS( motorrunAttrTbl ),
                                              &mrCBs );
    }
    
    return ( status );
}

bStatus_t MR_RegisterAppCBs( mrProfileCBs_t *appCallbacks )
{
    if ( appCallbacks )
    {
        mrProfile_AppCBs = appCallbacks;
        
        return ( SUCCESS );
    }
    else
    {
        return ( bleAlreadyInRequestedMode );
    }        
}

bStatus_t MR_SetParameter( uint8 param, uint8 len, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case MR_STAMP:
            if (len == MR_STAMP_LEN)
            {
                VOID osal_memcpy( mrStamp, pValue, MR_STAMP_LEN );
                
                GATTServApp_ProcessCharCfg( mrStampConfig,
                                            mrStamp,
                                            FALSE,
                                            motorrunAttrTbl,
                                            GATT_NUM_ATTRS(motorrunAttrTbl),
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

bStatus_t MR_GetParameter( uint8 param, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case MR_VALUE:
            *((uint8 *)pValue) = mrValue;
            break;
            
        case MR_STAMP:
            VOID osal_memcpy( pValue, mrStamp, MR_STAMP_LEN );
            break;
            
        default:
            ret = INVALIDPARAMETER;
            break;    
    }
    
    return (ret);
}

static uint8 MR_ReadAttrCB( uint16 connHandle,
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
            case MR_STAMP_UUID:
                *pLen = MR_STAMP_LEN;
                VOID osal_memcpy( pValue, pAttr->pValue, MR_STAMP_LEN );
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

static bStatus_t MR_WriteAttrCB( uint16 connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue,
                                 uint8 len,
                                 uint16 offset )
{
    bStatus_t status = SUCCESS;
    uint8 notifyApp = 0x00;
    
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
                
            case MR_VALUE_UUID:
                if ( offset == 0 )
                {
                    if ( len != 1 )
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }    
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }
                
                if ( status == SUCCESS )
                { 
                    notifyApp = pValue[0];
                    mrValue = pValue[0];
                }    
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
    
    if ((notifyApp != 0x00) && mrProfile_AppCBs && mrProfile_AppCBs->pfnMrProfileChange )
    {
        mrProfile_AppCBs->pfnMrProfileChange( notifyApp );
    }    
    return ( status );
}

static void MR_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    if (connHandle != LOOPBACK_CONNHANDLE )
    {
        if ( (changeType == LINKDB_STATUS_UPDATE_REMOVED)    ||
            ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
             (!linkDB_Up( connHandle ))))
        {
            GATTServApp_InitCharCfg( connHandle, mrStampConfig );
        }     
    }    
}

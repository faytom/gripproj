#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "wechatprofile.h"

CONST uint8 wcServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(WC_SERV_UUID),
    HI_UINT16(WC_SERV_UUID)
};

CONST uint8 wcWriteUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(WC_WRITE_UUID),
    HI_UINT16(WC_WRITE_UUID)
};

CONST uint8 wcIndicateUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(WC_INDICATE_UUID),
    HI_UINT16(WC_INDICATE_UUID)
};

CONST uint8 wcReadUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(WC_READ_UUID),
    HI_UINT16(WC_READ_UUID)
};
static wcProfileCBs_t *wcProfile_AppCBs = NULL;

static CONST gattAttrType_t wcService = {ATT_BT_UUID_SIZE, wcServUUID};

static uint8 wcWriteProps = GATT_PROP_WRITE;
static uint8 wcWrite[WC_DATA_LEN] = {0, 0, 0, 0, 0, 
                                  0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0};
static uint8 wcWriteDesp[13] = "wechat write\0";

//static uint8 wcReadProps = GATT_PROP_INDICATE | GATT_PROP_READ;
static uint8 wcIndicateProps = GATT_PROP_INDICATE;
static uint8 wcIndicate[WC_DATA_LEN] = {0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0};
static gattCharCfg_t wcIndicateConfig[GATT_MAX_NUM_CONN];
static uint8 wcIndicateDesp[16] = "wechat indicate\0";

static uint8 wcReadProps = GATT_PROP_READ;
static uint8 wcRead[6] = {0, 0, 0, 0, 0, 
                          0};
static uint8 wcReadDesp[12] = "wechat read\0";

static gattAttribute_t wechatAttrTbl[11] =
{
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *)&wcService
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &wcWriteProps
    },
    
    {
        { ATT_BT_UUID_SIZE, wcWriteUUID },
        GATT_PERMIT_WRITE,
        0,
        wcWrite
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        wcWriteDesp
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &wcIndicateProps
    },
    
    {
        { ATT_BT_UUID_SIZE, wcIndicateUUID },
        GATT_PERMIT_READ,
        0,
        wcIndicate
    },
    
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)wcIndicateConfig
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        wcIndicateDesp
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &wcReadProps
    },
    
    {
        { ATT_BT_UUID_SIZE, wcReadUUID },
        GATT_PERMIT_READ,
        0,
        wcRead
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        wcReadDesp
    }
};

static uint8 WC_ReadAttrCB( uint16 connHandle,
                            gattAttribute_t *pAttr,
                            uint8 *pValue,
                            uint8 *pLen,
                            uint16 offset,
                            uint8 maxLen );

static bStatus_t WC_WriteAttrCB( uint16 connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue,
                                 uint8 len,
                                 uint16 offset );

static void WC_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

CONST gattServiceCBs_t wcCBs =
{
    WC_ReadAttrCB,
    WC_WriteAttrCB,
    NULL
};

bStatus_t WC_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, wcIndicateConfig );
    
    VOID linkDB_Register( WC_HandleConnStatusCB );
    
    if ( services & WC_SERVICE )
    {
        status = GATTServApp_RegisterService( wechatAttrTbl,
                                              GATT_NUM_ATTRS( wechatAttrTbl ),
                                              &wcCBs );
    }
    
    return ( status );
}

bStatus_t WC_RegisterAppCBs( wcProfileCBs_t *appCallbacks )
{
    if ( appCallbacks )
    {
        wcProfile_AppCBs = appCallbacks;
        
        return (SUCCESS);
    }
    else
    {
        return (bleAlreadyInRequestedMode);
    }    
}

bStatus_t WC_SetParameter( uint8 param, uint8 len, void *pValue )
{
    bStatus_t ret = SUCCESS;
    
    if ( param == WC_INDICATE )
    {
        if (len == WC_DATA_LEN)
        {
            VOID osal_memcpy( wcIndicate, pValue, WC_DATA_LEN );
            // See if Notification has been enabled
            GATTServApp_ProcessCharCfg( wcIndicateConfig,
                                        wcIndicate,
                                        FALSE,
                                        wechatAttrTbl,
                                        GATT_NUM_ATTRS(wechatAttrTbl),
                                        INVALID_TASK_ID );
        }
        else
        {
            ret = bleInvalidRange;
        }
    }
    else if( param == WC_READ)
    {
      if(len == 6)
      {
        VOID osal_memcpy( wcRead, pValue, len );
      }
      else
      {
        ret = bleInvalidRange;
      }
    }
    else
    {
        ret = INVALIDPARAMETER;
    }
    
    return (ret);    
}

static bStatus_t WC_ReadAttrCB( uint16 connHandle,
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
        
        if (uuid == WC_INDICATE_UUID)
        {
            *pLen = WC_DATA_LEN;
            VOID osal_memcpy( pValue, pAttr->pValue, WC_DATA_LEN );
        }
        else if (uuid == WC_READ_UUID )
        {
          *pLen = 6;
            VOID osal_memcpy( pValue, pAttr->pValue, 6 );
        }
        else
        {
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
        }
    }
    else
    {
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }
    
    return (status);
}

static bStatus_t WC_WriteAttrCB( uint16 connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue,
                                 uint8 len,
                                 uint16 offset )
{
    bStatus_t status = SUCCESS;
    
    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1] );
        
        switch (uuid)
        {
            case GATT_CLIENT_CHAR_CFG_UUID:
                //HalLedSet(HAL_LED_4, HAL_LED_MODE_BLINK);
                status = GATTServApp_ProcessCCCWriteReq( connHandle,
                                                         pAttr,
                                                         pValue,
                                                         len,
                                                         offset,
                                                         GATT_CLIENT_CFG_INDICATE );
                break;
            
            case WC_WRITE_UUID:
                //HalLedSet(HAL_LED_5, HAL_LED_MODE_BLINK);
                if (offset == 0)
                {
                    if (len <= WC_DATA_LEN)
                    {
                        VOID osal_memset( wcWrite, 0, WC_DATA_LEN );
                        VOID osal_memcpy( wcWrite, pValue, len );
                        if ( wcProfile_AppCBs && wcProfile_AppCBs->pfnWcProfileChange )
                        {
                            wcProfile_AppCBs->pfnWcProfileChange( pValue );
                        }    
                    }
                    else
                    {
                        status = ATT_ERR_INVALID_VALUE_SIZE;
                    }    
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }
                
                break;

            default:
                status = ATT_ERR_ATTR_NOT_FOUND;
        }
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }
    
    return ( status );
}

static void WC_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    if ( connHandle != LOOPBACK_CONNHANDLE )
    {
        if ( (changeType == LINKDB_STATUS_UPDATE_REMOVED)       ||
            ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS)    &&
             (!linkDB_Up( connHandle ))))
        {
            GATTServApp_InitCharCfg( connHandle, wcIndicateConfig );
        }     
    }    
}

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "systemtest.h"

#define SERVAPP_NUM_ATTR_SUPPORTED      8

CONST uint8 stServUUID[ATT_BT_UUID_SIZE] = 
{
    LO_UINT16(ST_SERV_UUID),
    HI_UINT16(ST_SERV_UUID)
};

CONST uint8 stTest1UUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(ST_TEST1_UUID),
    HI_UINT16(ST_TEST1_UUID)
};

CONST uint8 stTest2UUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(ST_TEST2_UUID),
    HI_UINT16(ST_TEST2_UUID)
};

static stProfileCBs_t *stProfile_AppCBs = NULL;

static CONST gattAttrType_t stService = {ATT_BT_UUID_SIZE, stServUUID};

static uint8 stTest1Props = GATT_PROP_READ | GATT_PROP_NOTIFY;

static uint8 stTest1[ST_TEST_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static gattCharCfg_t stTest1Config[GATT_MAX_NUM_CONN];

static uint8 stTest1UserDesp[13] = "system test1\0";

static uint8 stTest2Props = GATT_PROP_WRITE;

static uint8 stTest2[ST_TEST_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static uint8 stTest2UserDesp[13] = "system test2\0";

static gattAttribute_t systemtestAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *)&stService
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &stTest1Props
    },
    
    {
        { ATT_BT_UUID_SIZE, stTest1UUID },
        GATT_PERMIT_READ,
        0,
        stTest1
    },
    
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)stTest1Config
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        stTest1UserDesp
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &stTest2Props
    },
    
    {
        { ATT_BT_UUID_SIZE, stTest2UUID },
        GATT_PERMIT_WRITE,
        0,
        stTest2
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        stTest2UserDesp
    }
};

static uint8 ST_ReadAttrCB( uint16 connHandle,
                            gattAttribute_t *pAttr,
                            uint8 *pValue,
                            uint8 *pLen,
                            uint16 offset,
                            uint8 maxLen );

static bStatus_t ST_WriteAttrCB( uint16 connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue,
                                 uint8 len,
                                 uint16 offset );

static void ST_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

CONST gattServiceCBs_t stCBs =
{
    ST_ReadAttrCB,
    ST_WriteAttrCB,
    NULL
};

bStatus_t ST_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, stTest1Config );
    
    VOID linkDB_Register( ST_HandleConnStatusCB );
    
    if ( services * ST_SERVICE )
    {
        status = GATTServApp_RegisterService( systemtestAttrTbl,
                                              GATT_NUM_ATTRS( systemtestAttrTbl ),
                                              &stCBs );
    }
    
    return ( status );
}

bStatus_t ST_RegisterAppCBs( stProfileCBs_t *appCallbacks )
{
    if ( appCallbacks )
    {
        stProfile_AppCBs = appCallbacks;
        
        return ( SUCCESS );
    }
    else
    {
        return ( bleAlreadyInRequestedMode );
    }
}

bStatus_t ST_SetParameter( uint8 param, uint8 len, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case ST_TEST1:
            if (len == ST_TEST_LEN)
            {
                VOID osal_memcpy( stTest1, pValue, ST_TEST_LEN );
                
                GATTServApp_ProcessCharCfg( stTest1Config,
                                            stTest1,
                                            FALSE,
                                            systemtestAttrTbl,
                                            GATT_NUM_ATTRS(systemtestAttrTbl),
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

bStatus_t ST_GetParameter( uint8 param, void *pValue )
{
    bStatus_t ret = SUCCESS;
    
    switch (param)
    {
        case ST_TEST1:
            VOID osal_memcpy( pValue, stTest1, ST_TEST_LEN );
            break;
            
        case ST_TEST2:
            VOID osal_memcpy( pValue, stTest2, ST_TEST_LEN );
            break;
        
        default:
            ret = INVALIDPARAMETER;
            break;
    }
    
    return (ret);
}

static uint8 ST_ReadAttrCB( uint16 connHandle,
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
            case ST_TEST1_UUID:
                *pLen = ST_TEST_LEN;
                VOID osal_memcpy( pValue, pAttr->pValue, ST_TEST_LEN );
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

static bStatus_t ST_WriteAttrCB( uint16 connHandle,
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
                
            case ST_TEST2_UUID:
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
                    
                    VOID osal_memcpy( stTest2, pValue, ST_TEST_LEN );
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
    
    if ((notifyApp == 0x01) && stProfile_AppCBs &&
        stProfile_AppCBs->pfnStProfileChange )
    {
        stProfile_AppCBs->pfnStProfileChange( stTest2 );
    }
    
    return ( status );
}

static void ST_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
    if (connHandle != LOOPBACK_CONNHANDLE )
    {
        if ( (changeType == LINKDB_STATUS_UPDATE_REMOVED)    ||
            ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) &&
             (!linkDB_Up( connHandle ))))
        {
            GATTServApp_InitCharCfg( connHandle, stTest1Config );
        }
    }
}
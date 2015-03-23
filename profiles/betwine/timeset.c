#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "timeset.h"

#define SERVAPP_NUM_ATTR_SUPPORTED      4

CONST uint8 tsServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(TS_SERV_UUID),
    HI_UINT16(TS_SERV_UUID)
};

CONST uint8 tsValueUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(TS_VALUE_UUID),
    HI_UINT16(TS_VALUE_UUID)
};

static tsProfileCBs_t *tsProfile_AppCBs = NULL;

static CONST gattAttrType_t tsService = { ATT_BT_UUID_SIZE, tsServUUID };

static uint8 tsProps = GATT_PROP_WRITE | GATT_PROP_READ;

static uint8 tsValue[TIME_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static uint8 tsUserDesp[9] = "time set\0";

static gattAttribute_t timesetAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *)&tsService
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &tsProps
    },
    
    {
        { ATT_BT_UUID_SIZE, tsValueUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        tsValue
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        tsUserDesp
    }
};

static uint8 TS_ReadAttrCB( uint16 connHandle, 
                            gattAttribute_t *pAttr, 
                            uint8 *pValue, 
                            uint8 *pLen, 
                            uint16 offset, 
                            uint8 maxLen );
                            
static bStatus_t TS_WriteAttrCB( uint16 connHandle, 
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue, 
                                 uint8 len, 
                                 uint16 offset );

CONST gattServiceCBs_t tsCBs =
{
    TS_ReadAttrCB,
    TS_WriteAttrCB,
    NULL
};

bStatus_t TS_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    
    if ( services & TS_SERVICE )
    {
        status = GATTServApp_RegisterService( timesetAttrTbl,
                                              GATT_NUM_ATTRS( timesetAttrTbl ),
                                              &tsCBs );
    }
    
    return ( status );
}

bStatus_t TS_RegisterAppCBs( tsProfileCBs_t *appCallbacks )
{
    if ( appCallbacks )
    {
        tsProfile_AppCBs = appCallbacks;
        
        return ( SUCCESS );
    }    
    else
    {
        return ( bleAlreadyInRequestedMode );
    }    
}

bStatus_t TS_SetParameter( uint8 param, uint8 len, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case TS_VALUE:
            if (len == TIME_LEN)
            {
                VOID osal_memcpy( tsValue, pValue, TIME_LEN );
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

bStatus_t TS_GetParameter( uint8 param, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case TS_VALUE:
            VOID osal_memcpy( pValue, tsValue, TIME_LEN );
            break;
            
        default:
            ret = INVALIDPARAMETER;
            break;    
    }
    
    return (ret);
}

static uint8 TS_ReadAttrCB( uint16 connHandle,
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
            case TS_VALUE_UUID:
                *pLen = TIME_LEN;
                VOID osal_memcpy( pValue, pAttr->pValue, TIME_LEN );
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

static bStatus_t TS_WriteAttrCB( uint16 connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue,
                                 uint8 len,
                                 uint16 offset )
{
    bStatus_t status = SUCCESS;
    uint8 notifyApp = 0xFF;
    
    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch (uuid)
        {
            case TS_VALUE_UUID:
                if ( offset == 0 )
                {
                    if ( len != TIME_LEN )
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
                    tsValue[0] = pValue[0];
                    tsValue[1] = pValue[1];
                    tsValue[2] = pValue[2];
                    tsValue[3] = pValue[3];
                    tsValue[4] = pValue[4];
                    tsValue[5] = pValue[5];
                    notifyApp = 1;
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
    
    if ((notifyApp == 1) && tsProfile_AppCBs && tsProfile_AppCBs->pfnTsProfileChange )
    {
        tsProfile_AppCBs->pfnTsProfileChange( pValue );
    }    
    
    return ( status );
}

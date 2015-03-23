#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "historystep.h"

#define SERVAPP_NUM_ATTR_SUPPORTED      4

CONST uint8 hsServUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(HS_SERV_UUID),
    HI_UINT16(HS_SERV_UUID)
};

CONST uint8 hsStepsUUID[ATT_BT_UUID_SIZE] =
{
    LO_UINT16(HS_STEPS_UUID),
    HI_UINT16(HS_STEPS_UUID)
};

static CONST gattAttrType_t hsService = { ATT_BT_UUID_SIZE, hsServUUID };

static uint8 hsStepsProps = GATT_PROP_READ;

static uint8 hsStepsValue[STEPS_LEN] = {0xFF, 0xFF, 0xFF, 
                                        0xFF, 0xFF, 0xFF, 
                                        0xFF, 0xFF, 0xFF, 
                                        0xFF, 0xFF, 0xFF,
                                        0xFF, 0xFF, 0xFF, 
                                        0xFF, 0xFF, 0xFF, 
                                        0xFF, 0xFF, 0xFF};

static uint8 hsStepsUserDesp[14] = "history steps\0";

static gattAttribute_t historystepAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8 *)&hsService
    },
    
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &hsStepsProps
    },
    
    {
        { ATT_BT_UUID_SIZE, hsStepsUUID },
        GATT_PERMIT_READ,
        0,
        hsStepsValue
    },
    
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        hsStepsUserDesp
    }
};

static uint8 HS_ReadAttrCB( uint16 connHandle, 
                            gattAttribute_t *pAttr, 
                            uint8 *pValue, 
                            uint8 *pLen, 
                            uint16 offset, 
                            uint8 maxLen );
                            
static bStatus_t HS_WriteAttrCB( uint16 connHandle, 
                                 gattAttribute_t *pAttr,
                                 uint8 *pValue, 
                                 uint8 len, 
                                 uint16 offset );


CONST gattServiceCBs_t hsCBs =
{
    HS_ReadAttrCB,
    HS_WriteAttrCB,
    NULL
};

bStatus_t HS_AddService( uint32 services )
{
    uint8 status = SUCCESS;
    
    if ( services & HS_SERVICE )
    {
        status = GATTServApp_RegisterService( historystepAttrTbl,
                                              GATT_NUM_ATTRS( historystepAttrTbl ),
                                              &hsCBs );
    }
    
    return ( status );
}

bStatus_t HS_SetParameter( uint8 param, uint8 len, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case HS_DAY:
            if (len == STEPS_LEN)
            {
                VOID osal_memcpy( hsStepsValue, pValue, STEPS_LEN );
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
    
    return (ret);
}

bStatus_t HS_GetParameter( uint8 param, void *pValue )
{
    bStatus_t ret = SUCCESS;
    switch (param)
    {
        case HS_DAY:
            VOID osal_memcpy( pValue, hsStepsValue, STEPS_LEN );
            break;                      
            
        default:
            ret = INVALIDPARAMETER;
            break;    
    }
    
    return (ret);
}

static uint8 HS_ReadAttrCB( uint16 connHandle,
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
            case HS_STEPS_UUID:
                *pLen = STEPS_LEN;
                VOID osal_memcpy( pValue, pAttr->pValue, STEPS_LEN );
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

static bStatus_t HS_WriteAttrCB( uint16 connHandle,
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

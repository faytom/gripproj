#ifndef WECHATPROFILE_H
#define WECHATPROFILE_H

#define WC_WRITE            0
#define WC_INDICATE         1
#define WC_READ             2

#define WC_SERV_UUID        0xFEE7

#define WC_WRITE_UUID       0xFEC7
#define WC_INDICATE_UUID    0xFEC8
#define WC_READ_UUID        0xFEC9


#define WC_SERVICE          0x00000001

#define WC_DATA_LEN            20

#define WC_HEAD_LEN            8

typedef struct
{
    uint8 bMagicNumber;
    uint8 bVer;
    uint8 nLength_h;
    uint8 nLength_l;
    uint8 nCmdId_h;
    uint8 nCmdId_l;
    uint8 nSeq_h;
    uint8 nSeq_l;
} BpFixHead;

typedef NULL_OK void (*wcProfileChange_t)(uint8 param[WC_DATA_LEN]);

typedef struct
{
    wcProfileChange_t       pfnWcProfileChange;
} wcProfileCBs_t;

extern bStatus_t WC_AddService( uint32 services );
extern bStatus_t WC_RegisterAppCBs( wcProfileCBs_t *appCallbacks );
extern bStatus_t WC_SetParameter( uint8 param, uint8 len, void *pValue );

#endif

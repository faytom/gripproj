//FEATURE_OAD_BIM
//HAL_IMAGE_B
//FEATURE_OAD
//OAD_KEEP_NV_PAGES

//"$PROJ_DIR$\config\cc254x_ubl_pp.bat" "$PROJ_DIR$" "ProdUBL" "$PROJ_DIR$\Debug\Exe\BetwineAPP"

#include "hal_types.h"
#include "hal_board.h"

#define OAD_FLASH_PAGE_MULT     ((uint16)(HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE))

#if !defined OAD_IMG_A_PAGE
#define OAD_IMG_A_PAGE          1
#define OAD_IMG_A_AREA          54
#endif

#if !defined OAD_IMG_B_PAGE
#define OAD_IMG_B_PAGE          6
#define OAD_IMG_B_AREA          (124 - OAD_IMG_A_AREA)
#endif

#define KEY_BLENGTH             16

typedef struct {
    uint16 crc1;        // CRC-shadow must be 0xFFFF.
    uint16 ver;         // User-defined Image Version Number - default logic uses simple a '!=' comparison to start an OAD.
    uint16 len;         // Image length in 4-byte blocks (i.e. HAL_FLASH_WORD_SIZE blocks).
    uint8  uid[4];      // User-defined Image Identification bytes.
    uint8  res[4];      // Reserved space for future use.
} img_hdr_t;

typedef struct {
    uint8 aes_sign[KEY_BLENGTH];     // The AES-128 CBC signature.
    uint8 md5_sign[KEY_BLENGTH];     // The DeviceType and DeviceId MD5.
} aes_hdr_t;



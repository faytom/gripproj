// Sensor I2C address
#define HAL_MMA8652FC_I2C_ADDRESS           0x1D

// MMA8652FC register addresses
#define ACC_REG_ADDR_STATUS                 0x00    //R
#define ACC_REG_ADDR_OUT_X_MSB              0x01    //R
#define ACC_REG_ADDR_OUT_X_LSB              0x02    //R
#define ACC_REG_ADDR_OUT_Y_MSB              0x03    //R
#define ACC_REG_ADDR_OUT_Y_LSB              0x04    //R
#define ACC_REG_ADDR_OUT_Z_MSB              0x05    //R
#define ACC_REG_ADDR_OUT_Z_LSB              0x06    //R

#define ACC_REG_ADDR_F_SETUP                0x09    //R/W
#define ACC_REG_ADDR_TRIG_CFG               0x0A    //R/W
#define ACC_REG_ADDR_SYSMOD                 0x0B    //R
#define ACC_REG_ADDR_INT_SOURCE             0x0C    //R
#define ACC_REG_ADDR_WHO_AM_I               0x0D    //R
#define ACC_REG_ADDR_XYZ_DATA_CFG           0x0E    //R/W
#define ACC_REG_ADDR_HP_FILTER_CUTOFF       0x0F    //R/W
#define ACC_REG_ADDR_PL_STATUS              0x10    //R
#define ACC_REG_ADDR_PL_CFG                 0x11    //R/W
#define ACC_REG_ADDR_PL_COUNT               0x12    //R/W
#define ACC_REG_ADDR_PL_BF_ZCOMP            0x13    //R/W
#define ACC_REG_ADDR_P_L_THS_REG            0x14    //R/W
#define ACC_REG_ADDR_FF_MT_CFG              0x15    //R/W
#define ACC_REG_ADDR_FF_MT_SRC              0x16    //R
#define ACC_REG_ADDR_FF_MT_THS              0x17    //R/W
#define ACC_REG_ADDR_FF_MT_COUNT            0x18    //R/W

#define ACC_REG_ADDR_TRANSIENT_CFG          0x1D    //R/W
#define ACC_REG_ADDR_TRANSIENT_SRC          0x1E    //R
#define ACC_REG_ADDR_TRANSIENT_THS          0x1F    //R/W
#define ACC_REG_ADDR_TRANSIENT_COUNT        0x20    //R/W
#define ACC_REG_ADDR_PULSE_CFG              0x21    //R/W
#define ACC_REG_ADDR_PULSE_SRC              0x22    //R
#define ACC_REG_ADDR_PULSE_THSX             0x23    //R/W
#define ACC_REG_ADDR_PULSE_THSY             0x24    //R/W
#define ACC_REG_ADDR_PULSE_THSZ             0x25    //R/W
#define ACC_REG_ADDR_PULSE_TMLT             0x26    //R/W
#define ACC_REG_ADDR_PULSE_LTCY             0x27    //R/W
#define ACC_REG_ADDR_PULSE_WIND             0x28    //R/W
#define ACC_REG_ADDR_ASLP_COUNT             0x29    //R/W
#define ACC_REG_ADDR_CTRL_REG1              0x2A    //R/W
#define ACC_REG_ADDR_CTRL_REG2              0x2B    //R/W
#define ACC_REG_ADDR_CTRL_REG3              0x2C    //R/W
#define ACC_REG_ADDR_CTRL_REG4              0x2D    //R/W
#define ACC_REG_ADDR_CTRL_REG5              0x2E    //R/W
#define ACC_REG_ADDR_OFF_X                  0x2F    //R/W
#define ACC_REG_ADDR_OFF_Y                  0x30    //R/W
#define ACC_REG_ADDR_OFF_Z                  0x31    //R/W

//----------------------------------------------------------
// ACC_REG_ADDR_STATUS
//----------------------------------------------------------
#define ACC_ZYXDR_READY                     0x08

//----------------------------------------------------------
// ACC_REG_ADDR_INT_SOURCE
//----------------------------------------------------------
#define ACC_SRC_FIFO                        0x40
#define ACC_SRC_PULSE                       0x08
#define ACC_SRC_DRDY                        0x01

//----------------------------------------------------------
// ACC_REG_ADDR_F_STATUS
//----------------------------------------------------------

// FIFO buffer overflow mode
#define ACC_F_MODE_MASK                     0xC0
#define ACC_F_MODE_DISABLE                  0x00
#define ACC_F_MODE_CIRCULAR                 0x40
#define ACC_F_MODE_OVERFLOW                 0x80
#define ACC_F_MODE_TRIGGER                  0xC0
// FIFO Event Sample Count Watemark
#define ACC_F_WMRK_MASK                     0x3F
#define ACC_F_WMRK_DISABLE                  0x00
#define ACC_F_WMRK_25                       0x19
#define ACC_F_WMRK_32                       0x20
#define ACC_F_WMRK_10                       0x0A
// Number of FIFO samples
#define ACC_F_WMRK_COUNT                    25

//----------------------------------------------------------
// ACC_REG_ADDR_XYZ_DATA_CFG
//----------------------------------------------------------

// Enable high-pass output data
#define ACC_HPF_OUT_ENABLE                  0x10
// Output buffer data format
#define ACC_FS_MASK                         0x03
#define ACC_FS_2G                           0x00
#define ACC_FS_4G                           0x01
#define ACC_FS_8G                           0x10

//----------------------------------------------------------
// ACC_REG_ADDR_HP_FILTER_CUTOFF
//----------------------------------------------------------

// Set High-Pass Filter
#define ACC_HIGH_PASS_BYPASS                0x20
// Set Low-Pass Filter
#define ACC_LOW_PASS_ENABLE                 0x10
// High-Pass cutoff frequency 50HZ/Low Power mode
#define ACC_CUTOFF_F_SEL_MASK               0x03
#define ACC_CUTOFF_F_SEL_HS                 0x00
#define ACC_CUTOFF_F_SEL_HN                 0x01
#define ACC_CUTOFF_F_SEL_LN                 0x02
#define ACC_CUTOFF_F_SEL_LS                 0x03

//----------------------------------------------------------
// ACC_REG_ADDR_PULSE_CFG
//----------------------------------------------------------

#define ACC_PULSE_CFG_DPA                   0x80
#define ACC_PULSE_CFG_ELE                   0x40
#define ACC_PULSE_CFG_ZDPEFE                0x20
#define ACC_PULSE_CFG_ZSPEFE                0x10
#define ACC_PULSE_CFG_YDPEFE                0x08
#define ACC_PULSE_CFG_YSPEFE                0x04
#define ACC_PULSE_CFG_XDPEFE                0x02
#define ACC_PULSE_CFG_XSPEFE                0x01

//----------------------------------------------------------
// ACC_REG_ADDR_PULSE_SRC
//----------------------------------------------------------

#define ACC_PULSE_SRC_EA                    0x80
#define ACC_PULSE_SRC_AXZ                   0x40
#define ACC_PULSE_SRC_DPE                   0x08
#define ACC_PULSE_SRC_POLZ                  0x04

//----------------------------------------------------------
// ACC_REG_ADDR_PULSE_PARAM
//----------------------------------------------------------

#define ACC_PULSE_TMLT                      20
#define ACC_PULSE_LTCY                      10
#define ACC_PULSE_WIND                      20
#define ACC_PULSE_THSZ                      25

//----------------------------------------------------------
// ACC_REG_ADDR_CTRL_REG1
//----------------------------------------------------------

// Set Auto_WAKE sample frequency
#define ACC_ASLP_RATE_MASK                  0xC0
#define ACC_ASLP_RATE_50                    0x00
#define ACC_ASLP_RATE_12_5                  0x40
#define ACC_ASLP_RATE_6_25                  0x80
#define ACC_ASLP_RATE_1_56                  0xC0
// Data rate selection
#define ACC_DATA_RATE_MASK                  0xC7
#define ACC_DATA_RATE_800                   0x00
#define ACC_DATA_RATE_400                   0x08
#define ACC_DATA_RATE_200                   0x10
#define ACC_DATA_RATE_100                   0x18
#define ACC_DATA_RATE_50                    0x20
#define ACC_DATA_RATE_12_5                  0x28
#define ACC_DATA_RATE_6_25                  0x30
#define ACC_DATA_RATE_1_56                  0x38
// Fast-read mode
#define ACC_FAST_MODE                       0x02
// Full-scale selection
#define ACC_ACTIVE_MODE                     0x01
#define ACC_STANDBY_MODE                    0xFE

//----------------------------------------------------------
// ACC_REG_ADDR_CTRL_REG2
//----------------------------------------------------------
// Software Reset
#define ACC_RESET                           0x40
// SLEEP mode power scheme selection
#define ACC_SMODS_MASK                      0x18
#define ACC_SMODS_NORMAL                    0x00
#define ACC_SMODS_LOW_NOISE                 0x08
#define ACC_SMODS_HIGH_RESOLUTION           0x10
#define ACC_SMODS_LOW_POWER                 0x18
// AUTO-SLEEP ENABLE      
#define ACC_AUTO_SLEEP_ENABLE               0x04
// ACTIME mode power scheme selection
#define ACC_MODS_MASK                       0x03
#define ACC_MODS_NORMAL                     0x00
#define ACC_MODS_LOW_NOISE                  0x01
#define ACC_MODS_HIGH_RESOLUTION            0x02
#define ACC_MODS_LOW_POWER                  0x03

//----------------------------------------------------------
// ACC_REG_ADDR_CTRL_REG3
//----------------------------------------------------------
// Wake from Pulse interrupt
#define ACC_INT_WAKE_PULSE                  0x10
// Interrupt polarity
#define ACC_INT_ACTIVE_HIGH                 0x02
// Push-Pull/Open-Drain selection
#define ACC_INT_OPEN_DRAIN                  0x01

//----------------------------------------------------------
// ACC_REG_ADDR_CTRL_REG4
//----------------------------------------------------------

// Auto_SLEEP/WAKE Interrupt Enable
#define ACC_INT_EN_ASLP                     0x80
// FIFO Interrupt Enable
#define ACC_INT_EN_FIFO                     0x40
// Pulse Detection Interrupt Enable
#define ACC_INT_EN_PULSE                    0x08
// Data Ready Interrupt Enable
#define ACC_INT_EN_DRDY                     0x01

//----------------------------------------------------------
// ACC_REG_ADDR_CTRL_REG5
//----------------------------------------------------------

// Auto_SLEEP/WAKE INT1/INT2 Configuration
#define ACC_INT1_CFG_ASLP                   0x80
// FIFO INT1/INT2 Configuration
#define ACC_INT1_CFG_FIFO                   0x40
// Pulse INT1/INT2 Configuration
#define ACC_INT1_CFG_PULSE                  0x08
// Data Ready INT1/INT2 Configuration
#define ACC_INT1_CFG_DRDY                   0x01

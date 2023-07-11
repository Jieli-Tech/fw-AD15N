
//===============================================================================//
//
//      input IO define
//
//===============================================================================//
#define PA0_IN  1
#define PA1_IN  2
#define PA2_IN  3
#define PA3_IN  4
#define PA4_IN  5
#define PA5_IN  6
#define PA6_IN  7
#define PA7_IN  8
#define PA8_IN  9
#define PA9_IN  10
#define PA10_IN  11
#define PA11_IN  12
#define PA12_IN  13
#define PB0_IN  14
#define PB1_IN  15
#define PB2_IN  16
#define PB3_IN  17

//===============================================================================//
//
//      function input select sfr
//
//===============================================================================//
typedef struct {
    __RW __u8 FI_GP_ICH0;
    __RW __u8 FI_GP_ICH1;
    __RW __u8 FI_GP_ICH2;
    __RW __u8 FI_GP_ICH3;
    __RW __u8 FI_GP_ICH4;
    __RW __u8 FI_GP_ICH5;
    __RW __u8 FI_GP_ICH6;
    __RW __u8 FI_GP_ICH7;
    __RW __u8 FI_SPI0_CLK;
    __RW __u8 FI_SPI0_DA0;
    __RW __u8 FI_SPI0_DA1;
    __RW __u8 FI_SPI0_DA2;
    __RW __u8 FI_SPI0_DA3;
    __RW __u8 FI_SPI1_CLK;
    __RW __u8 FI_SPI1_DA0;
    __RW __u8 FI_SPI1_DA1;
    __RW __u8 FI_I2C0_SCL;
    __RW __u8 FI_I2C0_SDA;
    __RW __u8 FI_UART0_RX;
    __RW __u8 FI_UART1_RX;
    __RW __u8 FI_TOTAL;
} JL_IMAP_TypeDef;

#define JL_IMAP_BASE      (ls_base + map_adr(0x39, 0x00))
#define JL_IMAP           ((JL_IMAP_TypeDef   *)JL_IMAP_BASE)


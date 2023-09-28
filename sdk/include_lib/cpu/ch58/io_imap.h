
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
#define PC0_IN  14
#define PC1_IN  15
#define PC2_IN  16
#define PC3_IN  17
#define PC4_IN  18
#define PC5_IN  19
#define PC6_IN  20
#define PC7_IN  21
#define PC8_IN  22
#define PC9_IN  23
#define PC10_IN  24
#define PC11_IN  25
#define PC12_IN  26
#define PC13_IN  27
#define PC14_IN  28
#define PC15_IN  29
#define PD0_IN  30
#define PD1_IN  31
#define PD2_IN  32
#define PD3_IN  33
#define PD4_IN  34
#define PD5_IN  35
#define PD6_IN  36
#define PD7_IN  37
#define PD8_IN  38
#define PD9_IN  39
#define PD10_IN  40
#define PD11_IN  41
#define PD12_IN  42
#define PD13_IN  43
#define PD14_IN  44
#define PD15_IN  45
#define USBDP_IN  46
#define USBDM_IN  47

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
    __RW __u8 FI_SPI1_CLK;
    __RW __u8 FI_SPI1_DA0;
    __RW __u8 FI_SPI1_DA1;
    __RW __u8 FI_SPI1_DA2;
    __RW __u8 FI_SPI1_DA3;
    __RW __u8 FI_SPI2_CLK;
    __RW __u8 FI_SPI2_DA0;
    __RW __u8 FI_SPI2_DA1;
    __RW __u8 FI_IIC0_SCL;
    __RW __u8 FI_IIC0_SDA;
    __RW __u8 FI_IIC1_SCL;
    __RW __u8 FI_IIC1_SDA;
    __RW __u8 FI_UART0_RX;
    __RW __u8 FI_UART1_RX;
    __RW __u8 FI_UART2_RX;
    __RW __u8 FI_UART2_CTS;
    __RW __u8 FI_TOTAL;
} JL_IMAP_TypeDef;

#define JL_IMAP_BASE      (ls_base + map_adr(0x3a, 0x00))
#define JL_IMAP           ((JL_IMAP_TypeDef   *)JL_IMAP_BASE)


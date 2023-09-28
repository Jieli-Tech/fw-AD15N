
//===============================================================================//
//
//      output function define
//
//===============================================================================//
#define FO_GP_OCH0        ((0 << 2)|BIT(1))
#define FO_GP_OCH1        ((1 << 2)|BIT(1))
#define FO_GP_OCH2        ((2 << 2)|BIT(1))
#define FO_GP_OCH3        ((3 << 2)|BIT(1))
#define FO_GP_OCH4        ((4 << 2)|BIT(1))
#define FO_GP_OCH5        ((5 << 2)|BIT(1))
#define FO_GP_OCH6        ((6 << 2)|BIT(1))
#define FO_GP_OCH7        ((7 << 2)|BIT(1))
#define FO_SPI1_CLK        ((8 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA0        ((9 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA1        ((10 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA2        ((11 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA3        ((12 << 2)|BIT(1)|BIT(0))
#define FO_SPI2_CLK        ((13 << 2)|BIT(1)|BIT(0))
#define FO_SPI2_DA0        ((14 << 2)|BIT(1)|BIT(0))
#define FO_SPI2_DA1        ((15 << 2)|BIT(1)|BIT(0))
#define FO_IIC0_SCL        ((16 << 2)|BIT(1)|BIT(0))
#define FO_IIC0_SDA        ((17 << 2)|BIT(1)|BIT(0))
#define FO_IIC1_SCL        ((18 << 2)|BIT(1)|BIT(0))
#define FO_IIC1_SDA        ((19 << 2)|BIT(1)|BIT(0))
#define FO_UART0_TX        ((20 << 2)|BIT(1)|BIT(0))
#define FO_UART1_TX        ((21 << 2)|BIT(1)|BIT(0))
#define FO_UART2_TX        ((22 << 2)|BIT(1)|BIT(0))
#define FO_UART2_RTS        ((23 << 2)|BIT(1)|BIT(0))

//===============================================================================//
//
//      IO output select sfr
//
//===============================================================================//
typedef struct {
    __RW __u8 PA0_OUT;
    __RW __u8 PA1_OUT;
    __RW __u8 PA2_OUT;
    __RW __u8 PA3_OUT;
    __RW __u8 PA4_OUT;
    __RW __u8 PA5_OUT;
    __RW __u8 PA6_OUT;
    __RW __u8 PA7_OUT;
    __RW __u8 PA8_OUT;
    __RW __u8 PA9_OUT;
    __RW __u8 PA10_OUT;
    __RW __u8 PA11_OUT;
    __RW __u8 PA12_OUT;
    __RW __u8 PC0_OUT;
    __RW __u8 PC1_OUT;
    __RW __u8 PC2_OUT;
    __RW __u8 PC3_OUT;
    __RW __u8 PC4_OUT;
    __RW __u8 PC5_OUT;
    __RW __u8 PC6_OUT;
    __RW __u8 PC7_OUT;
    __RW __u8 PC8_OUT;
    __RW __u8 PC9_OUT;
    __RW __u8 PC10_OUT;
    __RW __u8 PC11_OUT;
    __RW __u8 PC12_OUT;
    __RW __u8 PC13_OUT;
    __RW __u8 PC14_OUT;
    __RW __u8 PC15_OUT;
    __RW __u8 PD0_OUT;
    __RW __u8 PD1_OUT;
    __RW __u8 PD2_OUT;
    __RW __u8 PD3_OUT;
    __RW __u8 PD4_OUT;
    __RW __u8 PD5_OUT;
    __RW __u8 PD6_OUT;
    __RW __u8 PD7_OUT;
    __RW __u8 PD8_OUT;
    __RW __u8 PD9_OUT;
    __RW __u8 PD10_OUT;
    __RW __u8 PD11_OUT;
    __RW __u8 PD12_OUT;
    __RW __u8 PD13_OUT;
    __RW __u8 PD14_OUT;
    __RW __u8 PD15_OUT;
    __RW __u8 USBDP_OUT;
    __RW __u8 USBDM_OUT;
} JL_OMAP_TypeDef;

#define JL_OMAP_BASE      (ls_base + map_adr(0x36, 0x00))
#define JL_OMAP           ((JL_OMAP_TypeDef   *)JL_OMAP_BASE)


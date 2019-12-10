/*
*********************************************************************************************************
*                                            uC/TCP-IP V2
*                                      The Embedded TCP/IP Suite
*                                            EXAMPLE CODE
*
*                          (c) Copyright 2003-2014; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/TCP-IP is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.  However,
*               please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                            NETWORK BOARD SUPPORT PACKAGE (BSP) FUNCTIONS
*
*                                        Freescale Kinetis K64
*                                               on the
*
*                                         Freescale FRDM-K64F
*                                          Evaluation Board
*
* Filename      : net_bsp.c
* Version       : V3.01.00
* Programmer(s) : FF
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    NET_BSP_MODULE
#include  <os.h>
#include  <net_bsp.h>
#include  <MK64F12.h>
#include  <system_MK64F12.h>
#include  <fsl_interrupt_manager.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MACROS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*
* Note(s) : (1) (a) Each network device maps to a unique network interface number.
*
*               (b) Instances of network devices' interface number SHOULD be named using the following
*                   convention :
*
*                       <Board><Device>[Number] _IF_Nbr
*
*                           where
*                                   <Board>         Development board name
*                                   <Device>        Network device name (or type)
*                                   [Number]        Network device number for each specific instance
*                                                       of device (optional if the development board
*                                                       does NOT support multiple instances of the
*                                                       specific device)
*
*                   For example, the network device interface number variable for the #2 MACB Ethernet
*                   controller on an Atmel AT91SAM92xx should be named 'AT91SAM92xx_MACB_2_IF_Nbr'.
*
*               (c) Network device interface number variables SHOULD be initialized to 'NET_IF_NBR_NONE'.
*********************************************************************************************************
*/

#ifdef  NET_IF_ETHER_MODULE_EN
static  NET_IF_NBR  K64F120M_0_IF_Nbr = NET_IF_NBR_NONE;
#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*
* Note(s) : (1) Device driver BSP functions may be arbitrarily named.  However, it is recommended that
*               device BSP functions be named using the names provided below.
*
*               (a) (1) BSP interface functions SHOULD be named using the following convention :
*
*                           NetDev_<Function Name>[Number] ()
*
*                               where
*                                       <Function Name>     Network device BSP function name
*                                       [Number]            Network device number for each specific instance
*                                                               of device (optional if the development board
*                                                               does NOT support multiple instances of the
*                                                               specific device)
*
*                       For example, the NetDev_CfgClk() function for the #2 MACB Ethernet controller
*                       on an Atmel AT91SAM92xx should be named NetDev_CfgClk2{}.
*
*
*                   (2) BSP-level device ISR handlers SHOULD be named using the following convention :
*
*                           NetDev_ISR_Handler<Device><Type>[Number] ()
*
*                               where
*                                       <Device>            Network device name or type
*                                       <Type>              Network device interrupt type (optional,
*                                                               if generic interrupt type or unknown)
*                                       [Number]            Network device number for each specific instance
*                                                               of device (optional if the development board
*                                                               does NOT support multiple instances of the
*                                                               specific device)
*
*               (b) All BSP function prototypes should be located within the development board's network
*                   BSP C source file ('net_bsp.c') & be declared as static functions to prevent name
*                   clashes with other network protocol suite BSP functions/files.
*********************************************************************************************************
*/

static  void        NetDev_CfgClk      (NET_IF   *pif,
                                        NET_ERR  *perr);

static  void        NetDev_CfgIntCtrl  (NET_IF   *pif,
                                        NET_ERR  *perr);

static  void        NetDev_CfgGPIO_RMII(NET_IF   *pif,
                                        NET_ERR  *perr);

static  CPU_INT32U  NetDev_ClkFreqGet  (NET_IF   *pif,
                                        NET_ERR  *perr);

static  void        NetBSP_ISR_Handler (void);



/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                    NETWORK DEVICE BSP INTERFACE
*
* Note(s) : (1) Device board-support package (BSP) interface structures are used by the device driver to
*               call specific devices' BSP functions via function pointer instead of by name.  This enables
*               the network protocol suite to compile & operate with multiple instances of multiple devices
*               & drivers.
*
*           (2) In most cases, the BSP interface structure provided below SHOULD suffice for most devices'
*               BSP functions exactly as is with the exception that BSP interface structures' names MUST be
*               unique & SHOULD clearly identify the development board, device name, & possibly the specific
*               device number (if the development board supports multiple instances of any given device).
*
*               (a) BSP interface structures SHOULD be named using the following convention :
*
*                       NetDev_BSP_<Board><Device>[Number] {}
*
*                           where
*                                   <Board>         Development board name
*                                   <Device>        Network device name (or type)
*                                   [Number]        Network device number for each specific instance
*                                                       of device (optional if the development board
*                                                       does NOT support multiple instances of the
*                                                       specific device)
*
*                   For example, the BSP interface structure for the #2 MACB Ethernet controller on an
*                   Atmel AT91SAM92xx should be named NetDev_BSP_AT91SAM92xx_MACB_2{}.
*
*               (b) The API structure MUST also be externally declared in the development board's network
*                   BSP header file ('net_bsp.h') with the exact same name & type.
*********************************************************************************************************
*/

#ifdef  NET_IF_ETHER_MODULE_EN                                            /* K64F120M BSP fnct ptrs :                 */
const  NET_DEV_BSP_ETHER  NetDev_BSP_FRDM_K64F = { NetDev_CfgClk,         /*   Cfg clk(s)                             */
                                                   NetDev_CfgIntCtrl,     /*   Cfg int ctrl(s)                        */
                                                   NetDev_CfgGPIO_RMII,   /*   Cfg GPIO                               */
                                                   NetDev_ClkFreqGet      /*   Get clk freq                           */
                                                  };
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   NETWORK DEVICE DRIVER FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           NetDev_CfgClk()
*
* Description : Configure clocks for the specified interface/device.
*
* Argument(s) : pif         Pointer to network interface to configure.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device clock successfully configured.
*                               NET_DEV_ERR_INVALID_CFG         Invalid device/PHY configuration.
*
* Return(s)   : none.
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : (1) The external Ethernet Phy may be clocked using several different sources.  The
*                   resulting Phy clock MUST be 25MHz for MII and 50MHz for RMII :
*
*                   (a) RCC_MCO_NoClock    External Phy clocked directly via external crystal.
*                   (b) RCC_MCO_XT1        External Phy clocked by HSE   via MCO output.
*                   (c) RCC_MCO_PLL3CLK    External Phy clocked by PLL3  via MCO output.
*********************************************************************************************************
*/

static  void  NetDev_CfgClk (NET_IF   *pif,
                             NET_ERR  *perr)
{
    (void)&pif;                                                 /* Prevent 'variable unused' compiler warning.          */


    DEF_BIT_SET(SIM_SCGC2, SIM_SCGC2_ENET_MASK);                /* Enable ENET Clock Module                             */

    MPU_CESR = 0u;
   *perr     = NET_DEV_ERR_NONE;
}


/*
*********************************************************************************************************
*                                         NetDev_CfgIntCtrl()
*
* Description : Configure interrupt controller for the specified interface/device.
*
* Argument(s) : pif         Pointer to network interface to configure.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device interrupt(s) successfully configured.
*
* Return(s)   : none.
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : none.
*
* Note(s)     : (1) Phy interrupts NOT supported when using the Generic Ethernet Phy driver.
*********************************************************************************************************
*/

static  void  NetDev_CfgIntCtrl (NET_IF   *pif,
                                 NET_ERR  *perr)
{
    K64F120M_0_IF_Nbr = pif->Nbr;                               /* Configure Device with Specific Interface Number      */

    INT_SYS_InstallHandler(ENET_Transmit_IRQn, NetBSP_ISR_Handler);
    INT_SYS_InstallHandler(ENET_Receive_IRQn,  NetBSP_ISR_Handler);
    INT_SYS_InstallHandler(ENET_Error_IRQn,    NetBSP_ISR_Handler);

    INT_SYS_EnableIRQ(ENET_Transmit_IRQn);
    INT_SYS_EnableIRQ(ENET_Receive_IRQn);
    INT_SYS_EnableIRQ(ENET_Error_IRQn);

   *perr = NET_DEV_ERR_NONE;
}


/*
*********************************************************************************************************
*                                          NetDev_CfgGPIO_RMII()
*
* Description : Configure general-purpose I/O (GPIO) for the specified interface/device.
*
* Argument(s) : pif         Pointer to network interface to configure.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device GPIO successfully configured.
*                               NET_DEV_ERR_INVALID_CFG         Invalid device/PHY configuration.
*
* Return(s)   : none.
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : (1) On the FRDM-K64F board, there is no external pullup available on MDIO signal when
*                   MK64FN1M0VLL12 MCU requests status of the Ethernet link connection. An Internal pullup
*                   is required when port configuration for MDIO signal is enabled.
*********************************************************************************************************
*/

static  void  NetDev_CfgGPIO_RMII (NET_IF   *pif,
                                   NET_ERR  *perr)
{
   (void)&pif;                                                  /* Prevent 'variable unused' compiler warning.          */


    DEF_BIT_SET(SIM_SCGC5, SIM_SCGC5_PORTB_MASK);               /* Clock Gate Control: Enable the PORT B Clock.         */
    DEF_BIT_SET(SIM_SCGC5, SIM_SCGC5_PORTA_MASK);               /* Clock Gate Control: Enable the PORT A Clock.         */

                                                                /* See Note (1) for PCR_MUX Configuration               */
    PORTB_PCR0  = (0u | PORT_PCR_MUX(4u));                      /* Configure MACNET RMII0_MDIO to Alternative 4         */
                                                                /* See Note 1.                                          */
    DEF_BIT_SET(PORTB_PCR0, (PORT_PCR_ODE_MASK  |               /* Disable Open drain output on pin.                    */
                             PORT_PCR_PS_MASK   |               /* Select pin internal resistor as pullup.              */
                             PORT_PCR_PE_MASK));                /* Enable pin internal pull resistor.                   */
    PORTB_PCR1  = (0u | PORT_PCR_MUX(4u));                      /* Configure MACNET RMII0_MDC  to Alternative 4         */
    PORTA_PCR14 = (0u | PORT_PCR_MUX(4u));                      /* Configure MACNET RMII0_CRS  to Alternative 4         */
    PORTA_PCR12 = (0u | PORT_PCR_MUX(4u));                      /* Configure MACNET RMII0_RXD1 to Alternative 4         */
    PORTA_PCR13 = (0u | PORT_PCR_MUX(4u));                      /* Configure MACNET RMII0_RXD0 to Alternative 4         */
    PORTA_PCR15 = (0u | PORT_PCR_MUX(4u));                      /* Configure MACNET RMII0_TXEN to Alternative 4         */
    PORTA_PCR16 = (0u | PORT_PCR_MUX(4u));                      /* Configure MACNET RMII0_TXD0 to Alternative 4         */
    PORTA_PCR17 = (0u | PORT_PCR_MUX(4u));                      /* Configure MACNET RMII0_TXD1 to Alternative 4         */

   *perr = NET_DEV_ERR_NONE;
}


/*
*********************************************************************************************************
*                                         NetDev_ClkFreqGet()
*
* Description : Get device clock frequency.
*
* Argument(s) : pif         Pointer to network interface to get clock frequency.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device clock frequency successfully returned.
*
* Return(s)   : MAC Device clock divider frequency (in Hz).
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : (1) The MDC input clock frequency is used by the device driver to determine which MDC
*                   divider to select in order to reduce the MDC / MII bus frequency to the ~2MHz range.
*********************************************************************************************************
*/

static  CPU_INT32U  NetDev_ClkFreqGet (NET_IF   *pif,
                                       NET_ERR  *perr)
{
    CPU_INT32U  clk_freq;


    (void)&pif;                                                 /* Prevent 'variable unused' compiler warning.          */

    DEF_BIT_SET(OSC_CR, OSC_CR_ERCLKEN_MASK);                   /* Enable External Reference Clk, OSCERCLK              */

    clk_freq = SystemCoreClock;                                 /* SystemCoreClock running at 120 MHz                   */
   *perr     = NET_DEV_ERR_NONE;

    return (clk_freq);
}


/*
*********************************************************************************************************
*                                         NetBSP_ISR_Handler()
*
* Description : BSP-level ISR handler for device receive & transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU &/or device interrupts.
*
* Note(s)     : (1) (a) Each device interrupt, or set of device interrupts, MUST be handled by a
*                       unique BSP-level ISR handler which maps each specific device interrupt to
*                       its corresponding network interface ISR handler.
*
*                   (b) BSP-level device ISR handlers SHOULD be named using the following convention :
*
*                           NetDev_ISR_Handler<Dev><Type>[Nbr]()
*
*                               where
*                                   (1) Dev         Network device name or type
*                                   (2) Type        Network device interrupt type (optional,
*                                                       if generic interrupt type or unknown)
*                                   (3) Nbr         Network device number (optional;
*                                                       see 'NETWORK DEVICE BSP INTERFACE  Note #2a3')
*********************************************************************************************************
*/

static  void  NetBSP_ISR_Handler (void)
{
    NET_IF_NBR        if_nbr;
    NET_DEV_ISR_TYPE  type;
    NET_ERR           err;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    OSIntEnter();                                               /* Tell the OS that we are starting an ISR              */
    CPU_CRITICAL_EXIT();


    if_nbr = K64F120M_0_IF_Nbr;                                 /* See Note #2b3.                                       */
    type   = NET_DEV_ISR_TYPE_UNKNOWN;                          /* See Note #2b2.                                       */

    NetIF_ISR_Handler(if_nbr, type, &err);
    OSIntExit();                                                /* Tell the OS that we are leaving the ISR              */
}

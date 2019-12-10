/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                              uC/TCP-IP
*                                           APPLICATION CODE
*
* Filename      : app_tcpip.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <app_tcpip.h>

/*
*********************************************************************************************************
*                                               ENABLE
*********************************************************************************************************
*/

#if (APP_CFG_TCPIP_EN == DEF_ENABLED)

#include <app_dhcp-c.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            AppTCPIP_Init()
*
* Description : Initialize uC/TCP-IP.
*
* Arguments   : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  AppTCPIP_Init (void)
{
    NET_IF_NBR     if_nbr_ether;
    NET_ERR        net_err;
#if (APP_CFG_DHCP_C_EN == DEF_DISABLED)
    NET_IPv4_ADDR  ip;
    NET_IPv4_ADDR  msk;
    NET_IPv4_ADDR  gateway;
#else
    CPU_BOOLEAN    started;
#endif

                                                                /* --------------------- INIT TCPIP ------------------- */
    APP_TRACE_INFO(("\r\n"));
    APP_TRACE_INFO(("===================================================================\r\n"));
    APP_TRACE_INFO(("=                       TCPIP INITIALIZATION                      =\r\n"));
    APP_TRACE_INFO(("===================================================================\r\n"));
    APP_TRACE_INFO(("Initializing TCPIP...\r\n"));

    net_err = Net_Init(&NetRxTaskCfg,                           /* Initialize uC/TCP-IP.                                */
                       &NetTxDeallocTaskCfg,
                       &NetTmrTaskCfg);
    if (net_err != NET_ERR_NONE) {
        APP_TRACE_INFO(("Net_Init() failed w/err = %d \r\n", net_err));
        return (DEF_FAIL);
    }

    APP_TRACE_INFO(("\r\n"));
                                                                /* -------------- ETHERNET CONFIGURATION -------------- */
    if_nbr_ether = NetIF_Add((void    *)&NetIF_API_Ether,       /* Ethernet  Interface API.                             */
                             (void    *)&NetDev_API_MACNET,     /* K64F120M  Device API.                                */
                             (void    *)&NetDev_BSP_FRDM_K64F,  /* FRDM-K64F Device BSP.                                */
                             (void    *)&NetDev_Cfg_K64F120M,   /* K64F120M  Device Configuration.                      */
                             (void    *)&NetPhy_API_Generic,    /* Generic   Phy API.                                   */
                             (void    *)&NetPhy_Cfg_K64F120M,   /* K64F120M  PHY Configuration.                         */
                             (NET_ERR *)&net_err);
    if (net_err != NET_IF_ERR_NONE) {
        APP_TRACE_INFO(("NetIF_Add() failed w/err = %d \r\n", net_err));
        return (DEF_FAIL);
    }

                                                                /* ------------------ START IF NBR 1 ------------------ */
    NetIF_Start(if_nbr_ether, &net_err);
    if (net_err != NET_IF_ERR_NONE) {
        APP_TRACE_INFO(("NetIF_Start() failed w/err = %d \r\n", net_err));
        return (DEF_FAIL);
    }

    NetIF_LinkStateWaitUntilUp(if_nbr_ether, 20, 200, &net_err);
    if (net_err != NET_IF_ERR_NONE) {
        APP_TRACE_INFO(("NetIF_LinkStateWaitUntilUp() failed w/err = %d \r\n", net_err));
        return (DEF_FAIL);
    }

                                                                /* ------------------- CFG IF NBR 1 ------------------- */
#if  APP_CFG_DHCP_C_EN == DEF_ENABLED
    started = AppDHCPc_Init(if_nbr_ether);

    if (started != DEF_YES) {
        APP_TRACE_INFO(("App_DHCPc_Init() returned an error, uC/DHCPc is not started"));
        return (DEF_FAIL);
    }
#else

    ip      = NetASCII_Str_to_IPv4((CPU_CHAR *)"10.10.1.240",   &err);
    msk     = NetASCII_Str_to_IPv4((CPU_CHAR *)"255.255.255.0", &err);
    gateway = NetASCII_Str_to_IPv4((CPU_CHAR *)"10.10.1.1",     &err);

    (void)NetIPv4_CfgAddrAdd( if_nbr_ether,
                              ip,
                              msk,
                              gateway,
                             &err);
    if (err != NET_IPv4_ERR_NONE) {
        APP_TRACE_INFO(("NetIPv4_CfgAddrAdd() failed w/err = %d \r\n", err));
        return (DEF_FAIL);
    }
#endif

    return (DEF_OK);
}

/*
*********************************************************************************************************
*                                             ENABLE END
*********************************************************************************************************
*/

#endif

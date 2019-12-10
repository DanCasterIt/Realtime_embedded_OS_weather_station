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
*                                              uC/HTTP-s
*                                           APPLICATION CODE
*
* Filename      : app_http-s.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <app_http-s.h>

/*
*********************************************************************************************************
*                                               ENABLE
*********************************************************************************************************
*/

#if (APP_CFG_HTTP_S_EN == DEF_ENABLED)

//#include  <webpages_static.h>
#include  <Source/http-s.h>
#include  <http-s_instance_cfg.h>
#include  <FS/Static/http-s_fs_static.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                    LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             AppHTTPs_Init
*
* Description : Initialize HTTP server.
*
* Arguments   : none.
*
* Returns     : none.
*
* Notes       : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  AppHTTPs_Init (unsigned char * str1, CPU_INT32U len1, CPU_CHAR * html_name1, unsigned char * str2, CPU_INT32U len2, CPU_CHAR * html_name2)
{
    HTTPs_ERR        err_https;
    HTTPs_INSTANCE  *p_https_instance;

                                                                /* --------------- ADDING STATIC FILES ---------------- */
    HTTPs_FS_Init();
    
    HTTPs_FS_AddFile((CPU_CHAR *)&html_name1[0u],
                     (void     *)&str1[0u],
                     (CPU_INT32U) len1);
    
    HTTPs_FS_AddFile((CPU_CHAR *)&html_name2[0u],
                     (void     *)&str2[0u],
                     (CPU_INT32U) len2);

                                                                /* -------------- INITIALIZE HTTP SERVER -------------- */
    APP_TRACE_INFO(("\r\n"));
    APP_TRACE_INFO(("===================================================================\r\n"));
    APP_TRACE_INFO(("=                       HTTPs INITIALIZATION                      =\r\n"));
    APP_TRACE_INFO(("===================================================================\r\n"));
    APP_TRACE_INFO(("Initialize HTTP server ...\n\r"));

    err_https = HTTPs_Init();
    if (err_https != HTTPs_ERR_NONE) {
        APP_TRACE_INFO(("HTTP server: HTTPs_Init() failed\n\r"));
        return (DEF_FAIL);
    }

    if (err_https == HTTPs_ERR_NONE) {
        p_https_instance = HTTPs_InstanceInit(&HTTPs_CfgInstance_0,
                                              &HTTPs_FS_API_Static,
                                              &err_https);

        if (err_https != HTTPs_ERR_NONE) {
            APP_TRACE_INFO(("HTTP server: HTTPs_InstanceInit() failed\n\r"));
            return (DEF_FAIL);
        }

        HTTPs_InstanceStart(p_https_instance, &err_https);
    }

    if (err_https == HTTPs_ERR_NONE) {
        APP_TRACE_INFO(("HTTP server successfully initialized\n\r"));
        return (DEF_OK);
    } else {
        APP_TRACE_INFO(("HTTP server initialization failed\n\r"));
        return (DEF_FAIL);
    }
}

#endif

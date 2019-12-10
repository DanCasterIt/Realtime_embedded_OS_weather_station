/*
*********************************************************************************************************
*                                              uC/HTTPs
*                                 Hypertext Transfer Protocol (server)
*
*                          (c) Copyright 2004-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/HTTPs is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at: https://doc.micrium.com
*
*               You can contact us at: http://www.micrium.com
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                     HTTP SERVER MEMORY LIBRARY
*
* Filename      : http-s_mem.h
* Version       : V2.10.01
* Programmer(s) : AA
*                 MM
*********************************************************************************************************
* Note(s)       : (1) NO compiler-supplied standard library functions are used by the HTTP server.
*                     'http-s_util.*' implements ALL HTTP server specific library functions.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This main network protocol suite header file is protected from multiple pre-processor
*               inclusion through use of the HTTP memory module present pre-processor macro definition.
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  HTTPs_MEM_PRESENT                                          /* See Note #1.                                         */
#define  HTTPs_MEM_PRESENT

/*
*********************************************************************************************************
*********************************************************************************************************
*                                         HTTPs INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  "http-s.h"

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*********************************************************************************************************
*/

#ifdef   HTTPs_MEM_MODULE
#define  HTTPs_MEM_EXT
#else
#define  HTTPs_MEM_EXT  extern
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

                                                                              /* Instance functionalities.            */
void                 HTTPs_Mem_InstanceInit      (void);

HTTPs_INSTANCE      *HTTPs_Mem_InstanceGet       (HTTPs_ERR           *p_err);

void                 HTTPs_Mem_InstanceRelease   (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_ERR           *p_err);

                                                                              /* Conn functionalities.                */
void                 HTTPs_Mem_ConnPoolInit      (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_ERR           *p_err);

HTTPs_CONN          *HTTPs_Mem_ConnGet           (HTTPs_INSTANCE      *p_instance,
                                                  NET_SOCK_ID          sock_id,
                                                  NET_SOCK_ADDR        client_addr);

void                 HTTPs_Mem_ConnRelease       (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn);


#if (HTTPs_CFG_TOKEN_PARSE_EN == DEF_ENABLED)
CPU_BOOLEAN          HTTPs_Mem_TokenGet          (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn);

void                 HTTPs_Mem_TokenRelease      (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn);
#endif


#if (HTTPs_CFG_CGI_EN == DEF_ENABLED)
HTTPs_CGI_DATA      *HTTPs_Mem_CGI_DataGet       (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn);

void                 HTTPs_Mem_CGI_DataReleaseAll(HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn);
#endif


#if (HTTPs_CFG_HDR_EN == DEF_ENABLED)
HTTPs_HDR_BLK       *HTTPs_Mem_RespHdrGet        (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn,
                                                  HTTPs_HDR_FIELD      hdr_fied,
                                                  HTTPs_HDR_VAL_TYPE   val_type,
                                                  HTTPs_ERR           *p_err);

void                 HTTPs_Mem_RespHdrRelease    (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn,
                                                  HTTPs_HDR_BLK       *p_resp_hdr_field_blk);


HTTPs_HDR_BLK       *HTTPs_Mem_ReqHdrGet         (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn,
                                                  HTTPs_HDR_FIELD      hdr_fied,
                                                  HTTPs_HDR_VAL_TYPE   val_type,
                                                  HTTPs_ERR           *p_err);

void                 HTTPs_Mem_ReqHdrRelease     (HTTPs_INSTANCE      *p_instance,
                                                  HTTPs_CONN          *p_conn,
                                                  HTTPs_HDR_BLK       *p_req_hdr_field_blk);
#endif


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of HTTPs mem module include.                     */


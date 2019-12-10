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
*                                    uC/Apps FILE SYSTEM INTERFACE
*
*                                              FS PORT
*
* Filename      : http-s_fs_static.h
* Version       : V2.10.01
* Programmer(s) : FBJ
* *               AA
*                 SL
*                 BAN
*********************************************************************************************************
* Note(s)       : (1) Should be compatible with the following TCP/IP application versions (or more recent):
*
*                     (a) uC/HTTPs V2.00.00
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  NET_FS_STATIC_MODULE_PRESENT
#define  NET_FS_STATIC_MODULE_PRESENT


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <lib_mem.h>
#include  <FS/net_fs.h>
#include  <http-s_fs_static_cfg.h>


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

extern  const  NET_FS_API  HTTPs_FS_API_Static;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

CPU_BOOLEAN  HTTPs_FS_Init   (void);

CPU_BOOLEAN  HTTPs_FS_AddFile(CPU_CHAR          *p_name,
                              void              *p_data,
                              CPU_INT32U         size);


CPU_BOOLEAN  HTTPs_FS_SetTime(NET_FS_DATE_TIME  *p_time);


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of net fs static module include.                 */



/*
*********************************************************************************************************
*                                              uC/HTTPs
*                                 Hypertext Transfer Protocol (server)
*
*                          (c) Copyright 2004-2014; Micrium, Inc.; Weston, FL
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
*                                     NET FS STATIC CONFIGURATION
*
* Filename      : http-s_fs_static_cfg.h
* Version       : V2.10.00
* Programmer(s) : AA
*********************************************************************************************************
*/

#ifndef HTTPs_FS_STATIC_CFG_MODULE_PRESENT
#define HTTPs_FS_STATIC_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                             NET FS CFG
*********************************************************************************************************
*/

                                                                /* Configure external argument check feature ...        */
#define  HTTPs_FS_CFG_ARG_CHK_EXT_EN                     DEF_ENABLED
                                                                /*   DEF_DISABLED     External argument check DISABLED  */
                                                                /*   DEF_ENABLED      External argument check ENABLED   */


#define  HTTPs_FS_CFG_MAX_FILE_NAME_LEN                   25    /* Configure maximum file name length.                  */
#define  HTTPs_FS_CFG_NBR_FILES                            3    /* Configure number of files.                           */
#define  HTTPs_FS_CFG_NBR_DIRS                             1    /* Configure number of directories.                     */


#endif

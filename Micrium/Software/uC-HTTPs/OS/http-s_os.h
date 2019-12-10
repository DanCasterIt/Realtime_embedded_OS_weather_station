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
*                                 HTTP SERVER OPERATING SYSTEM LAYER
*
*                                   Micrium uC/OS-II & uC/OS-III
*
* Filename      : http-s_os.h
* Version       : V2.10.01
* Programmer(s) : AA
*********************************************************************************************************
* Note(s)       : (1) If uC/OS-II is used :
*                     (a) Assumes uC/OS-II V2.86 (or more recent version) is included in the project build.
*
*                     (b) REQUIREs the following uC/OS-II feature(s) to be ENABLED :
*
*                             --------- FEATURE --------    -- MINIMUM CONFIGURATION FOR HTTPs/OS PORT --
*
*                         (1) Tasks Delete
*                             (A) OS_TASK_DEL_EN            Enabled, if Stop/Restart is enabled.
*
*                         (2) Semaphores
*                             (A) OS_SEM_EN                 Enabled, if Stop/Restart is enabled.
*
*                 (2) If uC/OS-III is used :
*                     (a) Assumes uC/OS-III V3.01.0 (or more recent version) is included in the project build.
*
*                     (b) REQUIREs the following uC/OS-III feature(s) to be ENABLED :
*
*                             --------- FEATURE --------    -- MINIMUM CONFIGURATION FOR HTTPs/OS PORT --
*
*                         (1) Tasks Delete
*                             (A) OS_CFG_TASK_DEL_EN        Enabled
*
*                         (2) Semaphores
*                             (A) OS_CFG_SEM_EN             Enabled, if Stop/Restart is enabled.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  "..\Source\http-s.h"
#include  <lib_mem.h>


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                                DEFINES
*********************************************************************************************************
*/


#define  HTTPs_OS_STR_NAME_TASK                  "HTTP Instance"
#define  HTTPs_OS_STR_NAME_LOCK                  "HTTP Instance Lock"
#define  HTTPs_OS_STR_NAME_SEM_STOP_REQ          "HTTP Instance Stop req"
#define  HTTPs_OS_STR_NAME_SEM_STOP_COMPLETED    "HTTP Instance Stop compl"
#define  HTTPs_OS_STR_NAME_TMR                   "HTTP Conn Timeout"


/*
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void        *HTTPs_OS_LockCreate                  (       HTTPs_ERR        *p_err);

void         HTTPs_OS_LockAcquire                 (       void             *p_lock_obj,
                                                          CPU_BOOLEAN       blocking,
                                                          HTTPs_ERR        *p_err);

void         HTTPs_OS_LockRelease                 (       void             *p_lock_obj);

void        *HTTPs_OS_TmrCreate                   (       HTTPs_ERR        *p_err);

void         HTTPs_OS_TmrStart                    (       void            **p_tmr_obj,
                                                          CPU_INT32U        timeout_s,
                                                          void             *p_callback,
                                                          void             *p_arg,
                                                          HTTPs_ERR        *p_err);

void         HTTPs_OS_TmrStop                     (       void             *p_tmr_obj);

#if (HTTPs_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
void         HTTPs_OS_InstanceIsCfgValid          (const  HTTPs_CFG        *p_cfg,
                                                          HTTPs_ERR        *p_err);
#endif

void         HTTPs_OS_InstanceObjInit             (       HTTPs_INSTANCE   *p_instance,
                                                          HTTPs_ERR        *p_err);

void         HTTPs_OS_InstanceTaskCreate          (       HTTPs_INSTANCE   *p_instance,
                                                          HTTPs_ERR        *p_err);


void         HTTPs_OS_InstanceTaskDel             (       HTTPs_INSTANCE   *p_instance);

void         HTTPs_OS_InstanceStopReqSignal       (       HTTPs_INSTANCE   *p_instance,
                                                          HTTPs_ERR        *p_err);

CPU_BOOLEAN  HTTPs_OS_InstanceStopReqPending      (       HTTPs_INSTANCE   *p_instance);

void         HTTPs_OS_InstanceStopCompletedSignal (       HTTPs_INSTANCE   *p_instance);

void         HTTPs_OS_InstanceStopCompletedPending(       HTTPs_INSTANCE   *p_instance,
                                                          HTTPs_ERR        *p_err);

void         HTTPs_OS_TimeDly_ms                  (       CPU_INT32U        time_dly_ms);


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/



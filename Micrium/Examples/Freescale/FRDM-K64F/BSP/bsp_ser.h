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
*                                     MICIUM BOARD SUPPORT PACKAGE
*                                       SERIAL (UART) INTERFACE
*
*                                        Freescale Kinetis K64
*
* Filename      : bsp_ser.h
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
* Note(s)       : None.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               BSP_SER present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  BSP_SER_MODULE_PRESENT
#define  BSP_SER_MODULE_PRESENT


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu_core.h>
#include  <app_cfg.h>


/*
*********************************************************************************************************
*                                     EXTERNAL C LANGUAGE LINKAGE
*
* Note(s) : (1) C++ compilers MUST 'extern'ally declare ALL C function prototypes & variable/object
*               declarations for correct C language linkage.
*********************************************************************************************************
*/

#ifdef __cplusplus
extern  "C" {                                  /* See Note #1.                                         */
#endif


/*
*********************************************************************************************************
*                                              EXTERNS
*********************************************************************************************************
*/

#ifdef   BSP_SER_MODULE
#define  BSP_SER_EXT
#else
#define  BSP_SER_EXT  extern
#endif



/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/


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

#define  BSP_SER_COMM_UART_00                     0u
#define  BSP_SER_COMM_UART_01                     1u
#define  BSP_SER_COMM_UART_02                     2u
#define  BSP_SER_COMM_UART_03                     3u
#define  BSP_SER_COMM_UART_04                     4u
#define  BSP_SER_COMM_UART_05                     5u
#define  BSP_SER_COMM_UART_MAX                    6u


/*
*********************************************************************************************************
*                                              MACROS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

                                                                /* ----------------- RS-232 SERVICES ------------------ */
void        BSP_Ser_Init  (CPU_INT32U  baud_rate);

CPU_INT08U  BSP_Ser_RdByte(void);
void        BSP_Ser_RdStr (CPU_CHAR    *p_str,
                           CPU_INT16U   len);

void        BSP_Ser_WrByte(CPU_INT08U   c);
void        BSP_Ser_WrStr (CPU_CHAR    *p_str);

void        BSP_Ser_Printf(CPU_CHAR    *format,
                            ...);


/*
*********************************************************************************************************
*                                   EXTERNAL C LANGUAGE LINKAGE END
*********************************************************************************************************
*/

#ifdef __cplusplus
}                                              /* End of 'extern'al C lang linkage.                    */
#endif


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef  BSP_CFG_SER_COMM_SEL
#error  "BSP_CFG_SER_COMM_SEL   not  #define'd in 'app_cfg.h'                     "
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of BSP_SER module include.                       */

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
* Filename      : bsp_ser.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
* Note(s)       : None.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    MICRIUM_SOURCE
#define    BSP_SER_MODULE
#include  <bsp_ser.h>

#include  <stdarg.h>
#include  <fsl_interrupt_manager.h>
#include  <fsl_uart_common.h>
#include  <fsl_uart_driver.h>

#include  <pin_mux.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
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
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  uart_state_t  BSP_UartState;
static  semaphore_t   BSP_SerLock;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void           BSP_Ser_WrByteUnlocked(CPU_INT08U  c);
static  CPU_INT08U     BSP_Ser_RdByteUnlocked(void);
static  void           BSP_Ser_ISR_Handler   (void);
static  uart_status_t  RxCallBackFnct        (CPU_INT08U  *p_buf,
                                              void        *p_param);

extern  void           UART_DRV_IRQHandler   (uint32_t    instance);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            BSP_Ser_Init()
*
* Description : Initialize a serial port for communication.
*
* Argument(s) : baud_rate           The desired RS232 baud rate.
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_Init (CPU_INT32U  baud_rate)
{
    uart_user_config_t  uart_cfg;
    uart_status_t       uart_status;


    configure_uart_pins(BSP_CFG_SER_COMM_SEL);                  /* Configure UARTx pins                                 */
                                                                /* Install UART ISR handler                             */
    INT_SYS_InstallHandler(g_uartRxTxIrqId[BSP_CFG_SER_COMM_SEL],
                           BSP_Ser_ISR_Handler);

                                                                /* ----------------- CFG UART STRUCTURE --------------- */
    uart_cfg.baudRate        = baud_rate;
    uart_cfg.bitCountPerChar = kUart8BitsPerChar;
    uart_cfg.parityMode      = kUartParityDisabled;
    uart_cfg.stopBitCount    = kUartOneStopBit;

    OSA_SemaCreate(&BSP_SerLock, 1);                            /* Initialize OS semaphore as mutex                     */

                                                                /* --------------- INITIALIZE UART DRIVER ------------- */
    uart_status = UART_DRV_Init( BSP_CFG_SER_COMM_SEL,
                                &BSP_UartState,
                                &uart_cfg);

    if (uart_status != kStatus_UART_Success) {
        return;
    }

    UART_DRV_InstallRxCallback( BSP_CFG_SER_COMM_SEL,
                               &RxCallBackFnct,
                                0u);
}


/*
*********************************************************************************************************
*                                           BSP_Ser_Printf()
*
* Description : Formatted output to the serial port.
*               This funcion writes a string to a serial port. This call blocks until a
*               character appears at the port and the last character is a Carriage
*               Return (0x0D).
*
* Argument(s) : p_fmt    a pointer to a format string follwing the C format convention.
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_Printf (CPU_CHAR *p_fmt,
                      ...)
{
    CPU_CHAR  str[300u + 1u];
    va_list   vArgs;


    va_start(vArgs, p_fmt);

    vsprintf((char       *)str,
             (char const *)p_fmt,
                           vArgs);

    va_end(vArgs);

    BSP_Ser_WrStr(str);
}


/*
*********************************************************************************************************
*                                           BSP_Ser_RdByte()
*
* Description : Receive a single byte.
*
* Argument(s) : none.
*
* Return(s)   : The received byte
*
* Caller(s)   : Application
*
* Note(s)     : (1) This functions blocks until a data is received.
*
*               (2) It can not be called from an ISR.
*********************************************************************************************************
*/

CPU_INT08U  BSP_Ser_RdByte (void)
{
    CPU_INT08U    rx_byte;
    osa_status_t  os_status;


    os_status = OSA_SemaWait(&BSP_SerLock,                      /* Obtain access to the serial interface                */
                              OSA_WAIT_FOREVER);
    if (os_status != kStatus_OSA_Success) {
        return (0);
    }

    UART_DRV_ReceiveDataBlocking( BSP_CFG_SER_COMM_SEL,
                                 &rx_byte,
                                  sizeof(rx_byte),
                                  OSA_WAIT_FOREVER);

    OSA_SemaPost(&BSP_SerLock);                                 /* Release access to the serial interface               */


    return (rx_byte);
}


/*
*********************************************************************************************************
*                                            BSP_Ser_RdStr()
*
* Description : This function reads a string from a UART.
*
* Argument(s) : p_str      A pointer to a buffer at which the string can be stored.
*
*               len         The size of the string that will be read.
*
* Return(s)   : none.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_RdStr (CPU_CHAR    *p_str,
                     CPU_INT16U   len)
{
    CPU_CHAR      rx_data;
    CPU_CHAR      rx_buf_ix;
    osa_status_t  os_status;


    rx_buf_ix = 0u;
    p_str[0u] = 0u;

    os_status = OSA_SemaWait(&BSP_SerLock,                      /* Obtain access to the serial interface                */
                              OSA_WAIT_FOREVER);
    if (os_status != kStatus_OSA_Success) {
        return;
    }

    while (DEF_TRUE)
    {
        rx_data = BSP_Ser_RdByteUnlocked();

        if ((rx_data == ASCII_CHAR_CARRIAGE_RETURN) ||          /* Is it '\r' or '\n' character  ?                      */
            (rx_data == ASCII_CHAR_LINE_FEED      )) {

            BSP_Ser_WrByteUnlocked((CPU_INT08U)ASCII_CHAR_LINE_FEED);
            BSP_Ser_WrByteUnlocked((CPU_INT08U)ASCII_CHAR_CARRIAGE_RETURN);

            p_str[rx_buf_ix] = 0u;                              /* set the null character at the end of the string      */
            break;                                              /* exit the loop                                        */
        }

        if (rx_data == ASCII_CHAR_BACKSPACE) {                  /* Is backspace character                               */
            if (rx_buf_ix > 0u) {
                BSP_Ser_WrByteUnlocked((CPU_INT08U)ASCII_CHAR_BACKSPACE);
                BSP_Ser_WrByteUnlocked((CPU_INT08U)ASCII_CHAR_SPACE);
                BSP_Ser_WrByteUnlocked((CPU_INT08U)ASCII_CHAR_BACKSPACE);

                rx_buf_ix--;                                    /* Decrement the index                                  */
                p_str[rx_buf_ix] = 0u;
            }
        }

        if (ASCII_IsPrint(rx_data)) {                           /* Is it a printable character ... ?                    */
            BSP_Ser_WrByteUnlocked((CPU_INT08U)rx_data);        /* Echo-back                                            */
            p_str[rx_buf_ix] = rx_data;                         /* Save the received character in the buffer            */
            rx_buf_ix++;                                        /* Increment the buffer index                           */
            if (rx_buf_ix >= len) {
               rx_buf_ix = len;
            }
        }
    }

    OSA_SemaPost(&BSP_SerLock);                                 /* Release access to the serial interface               */
}


/*
*********************************************************************************************************
*                                           BSP_Ser_WrStr()
*
* Description : Transmits a string.
*
* Argument(s) : p_str      Pointer to the string that will be transmitted
*
* Caller(s)   : Application
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_WrStr (CPU_CHAR  *p_str)
{
    CPU_SIZE_T    len;
    osa_status_t  os_status;


    os_status = OSA_SemaWait(&BSP_SerLock,                      /* Obtain access to the serial interface                */
                              OSA_WAIT_FOREVER);
    if (os_status != kStatus_OSA_Success) {
        return;
    }

    len = Str_Len(p_str);

    UART_DRV_SendDataBlocking(               BSP_CFG_SER_COMM_SEL,
                              (CPU_INT08U *)&p_str[0u],
                                             len,
                                             OSA_WAIT_FOREVER);

    OSA_SemaPost(&BSP_SerLock);                                 /* Release access to the serial interface               */
}


/*
*********************************************************************************************************
*                                           BSP_Ser_WrByte()
*
* Description : Writes a single byte to a serial port.
*
* Argument(s) : c     The character to output.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Ser_WrByte(CPU_INT08U  c)
{
    osa_status_t  os_status;


    os_status = OSA_SemaWait(&BSP_SerLock,                      /* Obtain access to the serial interface                */
                              OSA_WAIT_FOREVER);
    if (os_status != kStatus_OSA_Success) {
        return;
    }

    BSP_Ser_WrByteUnlocked(c);

    OSA_SemaPost(&BSP_SerLock);                                 /* Release access to the serial interface               */
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_Ser_ISR_Handler()
*
* Description : Serial ISR
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_Ser_ISR_Handler (void)
{
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    OSIntEnter();                                               /* Tell the OS that we are starting an ISR              */
    CPU_CRITICAL_EXIT();

    UART_DRV_IRQHandler(BSP_CFG_SER_COMM_SEL);
    OSIntExit();                                                /* Tell the OS that we are leaving the ISR              */
}


/*
*********************************************************************************************************
*                                       BSP_Ser_WrByteUnlocked()
*
* Description : Writes a single byte to a serial port.
*
* Argument(s) : c    The character to output.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Ser_WrByte()
*               BSP_Ser_WrByteUnlocked()
*
* Note(s)     : (1) This function blocks until room is available in the UART for the byte to be sent.
*********************************************************************************************************
*/

static  void  BSP_Ser_WrByteUnlocked (CPU_INT08U c)
{
    UART_DRV_SendDataBlocking( BSP_CFG_SER_COMM_SEL,
                              &c,
                               sizeof(c),
                               OSA_WAIT_FOREVER);
}


/*
*********************************************************************************************************
*                                       BSP_Ser_RdByteUnlocked()
*
* Description : Receive a single byte.
*
* Argument(s) : none.
*
* Return(s)   : The received byte
*
* Caller(s)   : BSP_Ser_RdByte()
*               BSP_Ser_RdStr()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT08U  BSP_Ser_RdByteUnlocked (void)
{
    CPU_INT08U  rx_byte;


    UART_DRV_ReceiveDataBlocking( BSP_CFG_SER_COMM_SEL,
                                 &rx_byte,
                                  sizeof(rx_byte),
                                  OSA_WAIT_FOREVER);

    return (rx_byte);
}


/*
*********************************************************************************************************
*                                          RxCallBackFnct()
*
* Description : UART Rx Call back funtion.
*
* Argument(s) : none.
*
* Return(s)   : The received byte
*
* Caller(s)   : UART_DRV_IRQHandler()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  uart_status_t  RxCallBackFnct (CPU_INT08U  *p_buf,
                                       void        *p_param)
{
    return kStatus_UART_RxCallBackEnd;
}

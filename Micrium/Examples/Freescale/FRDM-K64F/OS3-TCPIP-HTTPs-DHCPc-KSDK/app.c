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
*                                            EXAMPLE CODE
*
*                                        Freescale Kinetis K64
*                                               on the
*
*                                         Freescale FRDM-K64F
*                                          Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include  <string.h>
#include  <webpages_static.h>

#include "fsl_interrupt_manager.h"
#include "fsl_gpio_common.h"

#include "derivative.h" 				// Include peripheral declarations
#include "FXOS8700CQ.h"					// Include declarations of FXOS8700CQ registers

#include  <app_cfg.h>

#include  <lib_math.h>
#include  <cpu_core.h>
#include  <os.h>

#include  <fsl_os_abstraction.h>
#include  <system_MK64F12.h>
#include  <board.h>

#include  <bsp_ser.h>

#if (APP_CFG_TCPIP_EN > 0u)
#include  <app_tcpip.h>

#if (APP_CFG_HTTP_S_EN > 0u)
#include  <app_http-s.h>
#endif
#endif


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
uint16_t ftm0_overflow_count,ftm0_zero_count;

volatile uint32_t ftm0_pulse;
static OS_SEM ftm0sem;

volatile uint16_t ptb2value_adc0_int_hdlr;
static OS_SEM adc0sem;

static OS_SEM PTC6sem;

unsigned char AccelMagData[12];
short Xout_Accel_14_bit, Yout_Accel_14_bit, Zout_Accel_14_bit;
float Xout_g, Yout_g, Zout_g, A, B, C;
uint32_t cnt = 0;

#define I2C_DisableAck()       I2C0_C1 |= I2C_C1_TXAK_MASK
#define I2C_EnableAck()        I2C0_C1 &= ~I2C_C1_TXAK_MASK
#define I2C_RepeatedStart()    I2C0_C1 |= I2C_C1_RSTA_MASK
#define I2C_EnterRxMode()      I2C0_C1 &= ~I2C_C1_TX_MASK
#define I2C_write_byte(data)   I2C0_D = data

#define I2C_Start()            I2C0_C1 |= I2C_C1_TX_MASK;\
                               I2C0_C1 |= I2C_C1_MST_MASK

#define I2C_Stop()             I2C0_C1 &= ~I2C_C1_MST_MASK;\
                               I2C0_C1 &= ~I2C_C1_TX_MASK

#define I2C_Wait()             while((I2C0_S & I2C_S_IICIF_MASK)==0) {} \
                               I2C0_S |= I2C_S_IICIF_MASK;

static OS_TCB	     TaskFtm0TCB;
static CPU_STK	     TaskFtm0Stk[APP_CFG_TASK_START_STK_SIZE];

static OS_TCB	     TaskIrqAdc0TCB;
static CPU_STK	     TaskIrqAdc0Stk[APP_CFG_TASK_START_STK_SIZE];

static OS_TCB	     AccelerometerTaskTCB;
static CPU_STK	     AccelerometerTaskStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB       Print_taskTCB;
static  CPU_STK      Print_taskStk[APP_CFG_TASK_START_STK_SIZE];

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppObjCreate (void);
static  void  AppTaskCreate(void);
static  void  AppTaskStart (void  *p_arg);

static	void ftm0_setup( void );
static	void TaskFtm0( void *p_arg );
static	void ftm0_int_hdlr( void );
static	void TaskIrqAdc0( void *p_arg );
static	void adc0_int_hdlr( void );
static	void setup_adc0( void );
static  void I2C_Init(void);
static  void I2C_WriteRegister(unsigned char u8SlaveAddress, unsigned char u8RegisterAddress, /*unsigned*/ char u8Data);
unsigned char I2C_ReadRegister(unsigned char u8SlaveAddress, unsigned char u8RegisterAddress);
static  void I2C_ReadMultiRegisters(unsigned char u8SlaveAddress, unsigned char u8RegisterAddress, unsigned char n, unsigned char *r);
static  void FXOS8700CQ_Init (void);
static  void FXOS8700CQ_Accel_Calibration (void);
static  void PTC6_int_hdlr( void );
static  void AccelerometerTask(void *p_arg);
static  void PrintInWebPage(char* string);
static  void Print_task( void *p_arg );

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This the main standard entry point.
*
* Note(s)     : none.
*********************************************************************************************************
*/

int  main (void)
{
    OS_ERR   err;

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR  cpu_err;
#endif

    hardware_init();

    INT_SYS_EnableIRQ(FTM0_IRQn);
    INT_SYS_EnableIRQ(ADC0_IRQn);
    INT_SYS_EnableIRQ(PORTC_IRQn);

    OSSemCreate(&ftm0sem, "PTC3 Semaphore", 0u, &err);
    OSSemCreate(&adc0sem, "ADC0 Semaphore", 0u, &err);
    OSSemCreate(&PTC6sem, "PTC6 Semaphore", 0u, &err);

    INT_SYS_InstallHandler(FTM0_IRQn, ftm0_int_hdlr);
    INT_SYS_InstallHandler(ADC0_IRQn, adc0_int_hdlr);
    INT_SYS_InstallHandler(PORTC_IRQn, PTC6_int_hdlr);

    GPIO_DRV_Init(accelIntPins, ledPins);

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet((CPU_CHAR *)"MK64FN1M0VMD12",
                (CPU_ERR  *)&cpu_err);
#endif

    OSA_Init();                                                 /* Init uC/OS-III.                                      */

    OSTaskCreate(&AppTaskStartTCB,                              /* Create the start task                                */
                 "App Task Start",
                  AppTaskStart,
                  0u,
                  APP_CFG_TASK_START_PRIO,
                 &AppTaskStartStk[0u],
                 (APP_CFG_TASK_START_STK_SIZE / 10u),
                  APP_CFG_TASK_START_STK_SIZE,
                  0u,
                  0u,
                  0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);

    OSA_Start();                                                /* Start multitasking (i.e. give control to uC/OS-III). */

    while (DEF_ON) {                                            /* Should Never Get Here                                */
        ;
    }
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Argument(s) : p_arg   is the argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Notes       : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
#if (APP_CFG_TCPIP_EN == DEF_ENABLED)
    CPU_BOOLEAN  started;
#endif
    OS_ERR       os_err;

    (void)p_arg;                                                /* See Note #1                                          */


    CPU_Init();                                                 /* Initialize the uC/CPU Services.                      */
    Mem_Init();                                                 /* Initialize the Memory Management Module              */
    Math_Init();                                                /* Initialize the Mathematical Module                   */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&os_err);                            /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    BSP_Ser_Init(115200u);

#if (APP_CFG_TCPIP_EN == DEF_ENABLED)
    started = AppTCPIP_Init();

    if (started == DEF_YES) {
#if (APP_CFG_HTTP_S_EN == DEF_ENABLED)
        PrintInWebPage("No data yet");
        started = AppHTTPs_Init((unsigned char*)Page, (CPU_INT32U) PAGE_LENGTH, STATIC_INDEX_HTML_NAME, (unsigned char*)Background, (CPU_INT32U) BACKGROUND_LENGTH, STATIC_BACKGROUND_NAME);                              /* Initalize HTTPs modules.                          */
        if (started != DEF_YES) {
            APP_TRACE_INFO(("App_HTTPs_Init() returned an error, uC/HTTPs is not started"));
        }
#endif
    } else {
        APP_TRACE_INFO(("Initializing uC/TCP-IP ... FAILED!! \r\n"));
    }
#endif

    APP_TRACE_DBG(("\n\r\n\r"));
    APP_TRACE_DBG(("Creating Application Objects...\n\r"));
    AppObjCreate();                                             /* Create Applicaiton Kernel Objects                    */

    APP_TRACE_DBG(("Creating Application Tasks...\n\r"));
    AppTaskCreate();                                            /* Create Application Tasks                             */

    int led = 0;

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
    	GPIO_DRV_SetPinOutput(BOARD_GPIO_LED_RED);
    	GPIO_DRV_SetPinOutput(BOARD_GPIO_LED_GREEN);
    	GPIO_DRV_SetPinOutput(BOARD_GPIO_LED_BLUE);
    	switch (led)	{
    	case 0:
    		GPIO_DRV_ClearPinOutput(BOARD_GPIO_LED_RED);
    		break;
    	case 1:
    		GPIO_DRV_ClearPinOutput(BOARD_GPIO_LED_GREEN);
    		break;
    	case 2:
    		GPIO_DRV_ClearPinOutput(BOARD_GPIO_LED_BLUE);
    		break;
    	}
    	if(led < 2) led++;
    	else led = 0;
        OSTimeDlyHMSM(0u, 0u, 0u, 500u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &os_err);
    }
}


/*
*********************************************************************************************************
*                                            AppObjCreate()
*
* Description:  Creates the application kernel objects.
*
* Argument(s) :  none.
*
* Return(s)   :  none.
*
* Caller(s)   :  App_TaskStart().
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static  void  AppObjCreate (void)
{
}


/*
*********************************************************************************************************
*                                           AppTaskCreate()
*
* Description :  This function creates the application tasks.
*
* Argument(s) :  none.
*
* Return(s)   :  none.
*
* Caller(s)   :  App_TaskStart().
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
    OS_ERR   err;

    OSTaskCreate(&TaskFtm0TCB,
                 "Task Ftm0",
		  TaskFtm0,
                  0u,
                  APP_CFG_TASK_START_PRIO,
                 &TaskFtm0Stk[0u],
                 (APP_CFG_TASK_START_STK_SIZE / 10u),
                  APP_CFG_TASK_START_STK_SIZE,
                  0u,
                  0u,
                  0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);

    OSTaskCreate(&TaskIrqAdc0TCB,
                 "Task Irq Adc0",
		  TaskIrqAdc0,
                  0u,
                  APP_CFG_TASK_START_PRIO,
                 &TaskIrqAdc0Stk[0u],
                 (APP_CFG_TASK_START_STK_SIZE / 10u),
                  APP_CFG_TASK_START_STK_SIZE,
                  0u,
                  0u,
                  0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);

    OSTaskCreate(&AccelerometerTaskTCB,
                 "Accelerometer Task",
		  AccelerometerTask,
                  0u,
                  APP_CFG_TASK_START_PRIO,
                 &AccelerometerTaskStk[0u],
                 (APP_CFG_TASK_START_STK_SIZE / 10u),
                  APP_CFG_TASK_START_STK_SIZE,
                  0u,
                  0u,
                  0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
    
    OSTaskCreate(&Print_taskTCB,
                 "Print task",
		  Print_task,
                  0u,
                  APP_CFG_TASK_START_PRIO,
                 &Print_taskStk[0u],
                 (APP_CFG_TASK_START_STK_SIZE / 10u),
                  APP_CFG_TASK_START_STK_SIZE,
                  0u,
                  0u,
                  0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &err);
}

static  void ftm0_setup( void )
{
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK; //Turn on FTM0 timer

	FTM0_CONF = 0xC0; //Set the timer in Debug mode, with BDM mode = 0xC0

	FTM0_FMS = 0x0; //Enable modifications to the FTM0 configuration
	FTM0_MODE |= (FTM_MODE_WPDIS_MASK|FTM_MODE_FTMEN_MASK); //Enable writing

	FTM0_CNTIN = FTM_CNTIN_INIT(0); //Initial value of 16 bit counter
	FTM0_MOD = FTM_MOD_MOD(0xFFFF); //Modulo of the count over 16 bits

	FTM0_SC = (FTM_SC_PS(7)| //ENABLE THE FTM0 with prescaler
			FTM_SC_CLKS(0x1)| //set to FTM_C_PS(0)=1
			FTM_SC_TOIE_MASK); //CLOCK set to 60Mhz
	//interrupt enabled
	/*
	 * Dual-edge capturing for FTM0_CH2 (PTC3) and FTM0_CH3
	 */
	FTM0_COMBINE = FTM_COMBINE_DECAPEN1_MASK; //Set channel 2 as input for dual capture
	FTM0_C2SC = FTM_CnSC_ELSA_MASK; //Capture Rising Edge only on channel 2
	FTM0_C3SC = (FTM_CnSC_ELSB_MASK|FTM_CnSC_CHIE_MASK); //Capture Falling Edge
	//on channel 3 and
	//enable the interrupt
}

void    ftm0_int_hdlr( void )
{
	OS_ERR err;

	CPU_CRITICAL_ENTER();
	OSIntEnter();

	FTM0_SC &= 0x7F; // Then, after the TOF bit has been read, we clear them to be ready for the next handler call

	if(FTM0_STATUS & 0x8) {
	   FTM0_STATUS &= 0x0;

           if(FTM0_C2V > FTM0_C3V)      ftm0_pulse=0xFFFF+FTM0_C3V-FTM0_C2V+(ftm0_overflow_count-1)*65536;
           else ftm0_pulse=FTM0_C3V-FTM0_C2V+(ftm0_overflow_count)*65536;
           ftm0_zero_count = 0;
	   OSSemPost(&ftm0sem, OS_OPT_POST_1 | OS_OPT_POST_NO_SCHED, &err);
	   ftm0_overflow_count=0; // FTM0 overflow count is set again to 0 for the next measurement.
	} else if(FTM0_STATUS & 0x4) { // Count overflow only if nothing has happened on CH03, but something has happened on CH2 (half period is considered)
	   ftm0_overflow_count++;
           ftm0_zero_count=0;
	} else  {
        ftm0_zero_count++;
          if(ftm0_zero_count > 100) C = 0;
        }

	CPU_CRITICAL_EXIT();
	OSIntExit();
}

static  void TaskFtm0( void *p_arg )
{
	OS_ERR os_err;
	(void)p_arg;

	ftm0_setup();
	while (DEF_TRUE) {
		//Setting DECAP1 bit to launch the dual capture process
		FTM0_COMBINE |= FTM_COMBINE_DECAP1_MASK;

		OSSemPend(&ftm0sem, 0u, OS_OPT_PEND_BLOCKING, 0u, &os_err);
                
                C = ((double) 175781.25 / ftm0_pulse); //(0.75*(30000000/(128.0*ftm0_pulse)))

		OSTimeDlyHMSM(0u, 0u, 1u, 500u, OS_OPT_TIME_HMSM_STRICT, &os_err);
	}
}

static  void setup_adc0( void )
{
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;
	ADC0_CFG1 |= ADC_CFG1_MODE(3); /* 16bits ADC*/
	ADC0_SC1A |= ADC_SC1_ADCH(31); /* Disable the module, ADCH = 1111 */
}

static  void TaskIrqAdc0( void *p_arg )
{
	OS_ERR os_err;

	(void)p_arg;

	setup_adc0();
	while (DEF_TRUE) {
		ADC0_SC1A = (12 & ADC_SC1_ADCH_MASK) | ADC_SC1_AIEN_MASK;

		OSSemPend(&adc0sem, 0u, OS_OPT_PEND_BLOCKING, 0u, &os_err); //for PTB2

		A = (ptb2value_adc0_int_hdlr * (double) 0.0003051757812); //((20*ptb2value_adc0_int_hdlr)/(65536))

		ADC0_SC1A = (13 & ADC_SC1_ADCH_MASK) | ADC_SC1_AIEN_MASK;

		OSSemPend(&adc0sem, 0u, OS_OPT_PEND_BLOCKING, 0u, &os_err); //for PTB3

		B = (ptb2value_adc0_int_hdlr * (double) 0.001220703125) - 20; //(((80*ptb2value_adc0_int_hdlr)/(65536))-20)

		OSTimeDlyHMSM(0u, 0u, 60u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
	}
}

static  void adc0_int_hdlr( void )
{
	OS_ERR os_err;

	CPU_CRITICAL_ENTER();
	OSIntEnter();

	ptb2value_adc0_int_hdlr = ADC0_RA;

	OSSemPost( &adc0sem, OS_OPT_POST_1 | OS_OPT_POST_NO_SCHED, &os_err );

	CPU_CRITICAL_EXIT();
        OSIntExit();
}

static  void I2C_Init(void)
{
        //I2C0 module initialization
        SIM_SCGC4 |= SIM_SCGC4_I2C0_MASK;        // Turn on clock to I2C0 module
        SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;       // Turn on clock to Port E module
        PORTE_PCR24 |= PORT_PCR_MUX(0x5);         // PTE24 pin is I2C0 SCL line
        PORTE_PCR25 |= PORT_PCR_MUX(0x5);         // PTE25 pin is I2C0 SDA line
        I2C0_F  |= I2C_F_ICR(0x14);              // SDA hold time = 2.125us, SCL start hold time = 4.25us, SCL stop hold time = 5.125us
        I2C0_C1 |= I2C_C1_IICEN_MASK;            // Enable I2C0 module
}

static  void I2C_WriteRegister(unsigned char u8SlaveAddress, unsigned char u8RegisterAddress, /*unsigned*/ char u8Data)
{
        OS_ERR os_err;
        
	I2C_Start();
	I2C0_D = u8SlaveAddress << 1;									/* Send I2C device address with W/R bit = 0 */
	I2C_Wait();

	I2C0_D = u8RegisterAddress;										/* Send register address */
	I2C_Wait();

	I2C0_D = u8Data;												/* Send the data */
	I2C_Wait();

	I2C_Stop();

        OSTimeDlyHMSM(0u, 0u, 1u, 38u, OS_OPT_TIME_HMSM_STRICT, &os_err);
}

unsigned char I2C_ReadRegister(unsigned char u8SlaveAddress, unsigned char u8RegisterAddress)
{
	unsigned char result;

	I2C_Start();
	I2C0_D = u8SlaveAddress << 1;									/* Send I2C device address with W/R bit = 0 */
	I2C_Wait();

	I2C0_D = u8RegisterAddress;										/* Send register address */
	I2C_Wait();

	I2C_RepeatedStart();

	I2C0_D = (u8SlaveAddress << 1) | 0x01;							/* Send I2C device address this time with W/R bit = 1 */
	I2C_Wait();

	I2C_EnterRxMode();
	I2C_DisableAck();

	result = I2C0_D;
	I2C_Wait();
	I2C_Stop();
	result = I2C0_D;
	return result;
}

static  void I2C_ReadMultiRegisters(unsigned char u8SlaveAddress, unsigned char u8RegisterAddress, unsigned char n, unsigned char *r)
{
	char i;

	I2C_Start();
	I2C0_D = u8SlaveAddress << 1;									/* Send I2C device address with W/R bit = 0 */
	I2C_Wait();

	I2C0_D = u8RegisterAddress;										/* Send register address */
	I2C_Wait();

	I2C_RepeatedStart();

	I2C0_D = (u8SlaveAddress << 1) | 0x01;							/* Send I2C device address this time with W/R bit = 1 */
	I2C_Wait();

	I2C_EnterRxMode();
	I2C_EnableAck();

	i = I2C0_D;
	I2C_Wait();

	for(i=0; i<n-2; i++)
	{
	    *r = I2C0_D;
	    r++;
	    I2C_Wait();
	}

	I2C_DisableAck();
	*r = I2C0_D;
	r++;
	I2C_Wait();
	I2C_Stop();
	*r = I2C0_D;
}

static  void FXOS8700CQ_Init (void)
{
        OS_ERR os_err;
        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG2, 0x40);          // Reset all registers to POR values

        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, XYZ_DATA_CFG_REG, 0x00);   // +/-2g range with 0.244mg/LSB

        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, M_CTRL_REG1, 0x1F);        // Hybrid mode (accelerometer + magnetometer), max OSR
        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, M_CTRL_REG2, 0x20);        // M_OUT_X_MSB register 0x33 follows the OUT_Z_LSB register 0x06 (used for burst read)

        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG2, 0x02);          // High Resolution mode
        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG3, 0x00);          // Push-pull, active low interrupt
        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG4, 0x01);          // Enable DRDY interrupt
        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG5, 0x01);          // DRDY interrupt routed to INT1 - PTD4
        I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG1, 0x35);          // ODR = 3.125Hz, Reduced noise, Active mode
     
        OSTimeDlyHMSM(0u, 0u, 0u, 11u, OS_OPT_TIME_HMSM_STRICT, &os_err);
}

static  void FXOS8700CQ_Accel_Calibration (void)
{
	char X_Accel_offset, Y_Accel_offset, Z_Accel_offset;

	I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG1, 0x00);		// Standby mode

	I2C_ReadMultiRegisters(FXOS8700CQ_I2C_ADDRESS, OUT_X_MSB_REG, 6, AccelMagData);		// Read data output registers 0x01-0x06

	Xout_Accel_14_bit = ((short) (AccelMagData[0]<<8 | AccelMagData[1])) >> 2;		// Compute 14-bit X-axis acceleration output value
	Yout_Accel_14_bit = ((short) (AccelMagData[2]<<8 | AccelMagData[3])) >> 2;		// Compute 14-bit Y-axis acceleration output value
	Zout_Accel_14_bit = ((short) (AccelMagData[4]<<8 | AccelMagData[5])) >> 2;		// Compute 14-bit Z-axis acceleration output value

	X_Accel_offset = Xout_Accel_14_bit / 8 * (-1);		// Compute X-axis offset correction value
	Y_Accel_offset = Yout_Accel_14_bit / 8 * (-1);		// Compute Y-axis offset correction value
	Z_Accel_offset = (Zout_Accel_14_bit - SENSITIVITY_2G) / 8 * (-1);		// Compute Z-axis offset correction value

	I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, OFF_X_REG, X_Accel_offset);
	I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, OFF_Y_REG, Y_Accel_offset);
	I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, OFF_Z_REG, Z_Accel_offset);

	I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG1, 0x35);		// Active mode again
}

static  void PTC6_int_hdlr( void )
{
	uint32_t ifsr;
	uint32_t portBaseAddr = g_portBaseAddr[GPIO_EXTRACT_PORT(kGpioAccelINT1)];
	OS_ERR os_err;

	CPU_CRITICAL_ENTER();
	OSIntEnter();

	ifsr = PORT_HAL_GetPortIntFlag(portBaseAddr);
	if((ifsr & (1 << GPIO_EXTRACT_PIN(kGpioAccelINT1)))){
		OSSemPost( &PTC6sem, OS_OPT_POST_1 | OS_OPT_POST_NO_SCHED, &os_err );
		GPIO_DRV_ClearPinIntFlag(kGpioAccelINT1);
	}

	CPU_CRITICAL_EXIT();
	OSIntExit();
}

static  void AccelerometerTask(void *p_arg)
{
	OS_ERR os_err;
	(void)p_arg;

	I2C_Init();
	FXOS8700CQ_Init();

	OSSemPend(&PTC6sem, 0u, OS_OPT_PEND_BLOCKING, 0u, &os_err);
        OSTimeDlyHMSM(0u, 0u, 1u, 53u, OS_OPT_TIME_HMSM_STRICT, &os_err);
	FXOS8700CQ_Accel_Calibration();

	while(DEF_TRUE){
		//I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG1, 0x35); //ready
		OSSemPend(&PTC6sem, 0u, OS_OPT_PEND_BLOCKING, 0u, &os_err);

		I2C_ReadMultiRegisters(FXOS8700CQ_I2C_ADDRESS, OUT_X_MSB_REG, 12, AccelMagData);		// Read data output registers 0x01-0x06 and 0x33 - 0x38

		// 14-bit accelerometer data
		Xout_Accel_14_bit = ((short) (AccelMagData[0]<<8 | AccelMagData[1])) >> 2;		// Compute 14-bit X-axis acceleration output value
		Yout_Accel_14_bit = ((short) (AccelMagData[2]<<8 | AccelMagData[3])) >> 2;		// Compute 14-bit Y-axis acceleration output value
		Zout_Accel_14_bit = ((short) (AccelMagData[4]<<8 | AccelMagData[5])) >> 2;		// Compute 14-bit Z-axis acceleration output value

		// Accelerometer data converted to g's
		Xout_g = ((float) Xout_Accel_14_bit) / SENSITIVITY_2G;		// Compute X-axis output value in g's
		Yout_g = ((float) Yout_Accel_14_bit) / SENSITIVITY_2G;		// Compute Y-axis output value in g's
		Zout_g = ((float) Zout_Accel_14_bit) / SENSITIVITY_2G;		// Compute Z-axis output value in g's

		//I2C_WriteRegister(FXOS8700CQ_I2C_ADDRESS, CTRL_REG1, 0x00); //stand by mode
		OSTimeDlyHMSM(0u, 0u, 1u, 0u,OS_OPT_TIME_HMSM_STRICT, &os_err);
	}

}

static  void PrintInWebPage(char* string)
{
	strcpy((char*)Page, (char const*)PageP1);
	strcat((char*)Page, (char const*)string);
	strcat((char*)Page, (char const*)PageP2);
}

static  void Print_task( void *p_arg )
{
	OS_ERR os_err;
	char str_buff[150];
  
	while(1)      {
          sprintf(str_buff, "<%u> <%f [m/s]> <%u [mm]> <%d [C]>\n\rX = %f, Y = %f, Z = %f\n\r",
            cnt, C , (unsigned int)A, (int)B, Xout_g, Yout_g, Zout_g);
          BSP_Ser_Printf(str_buff);
          PrintInWebPage(str_buff);
          cnt++;
          OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
	}
}
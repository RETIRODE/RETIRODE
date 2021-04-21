/* ----------------------------------------------------------------------------
 * Copyright (c) 2019 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * app.c
 *  - Simple example on how to use RSL10 UART CMSIS-Driver
 * ------------------------------------------------------------------------- */

#include "app.h"
#include <printf.h>

/* Global variables */
ARM_DRIVER_USART *uart;
char tx_buffer[4];
char rx_buffer[101];


/* ----------------------------------------------------------------------------
 * Function      : void Button_EventCallback(void)
 * ----------------------------------------------------------------------------
 * Description   : This function is a callback registered by the function
 *                 Initialize. Based on event argument different actions are
 *                 executed.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Usart_EventCallBack(uint32_t event)
{
	RETIRODE_RMP_UARTEventHandler(event);
}


/* ----------------------------------------------------------------------------
 * Function      : void Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the system, configures GPIO and USART driver.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Initialize(void)
{
    /* Mask all interrupts */
    __set_PRIMASK(PRIMASK_DISABLE_INTERRUPTS);

    /* Disable all existing interrupts, clearing all pending source */
    Sys_NVIC_DisableAllInt();
    Sys_NVIC_ClearAllPendingInt();

    /* Test DIO12 to pause the program to make it easy to re-flash */
    DIO->CFG[RECOVERY_DIO] = DIO_MODE_INPUT  | DIO_WEAK_PULL_UP |
                             DIO_LPF_DISABLE | DIO_6X_DRIVE;
    //while (DIO_DATA->ALIAS[RECOVERY_DIO] == 0);

    /* Prepare the 48 MHz crystal
     * Start and configure VDDRF */
    ACS_VDDRF_CTRL->ENABLE_ALIAS = VDDRF_ENABLE_BITBAND;
    ACS_VDDRF_CTRL->CLAMP_ALIAS  = VDDRF_DISABLE_HIZ_BITBAND;

    /* Wait until VDDRF supply has powered up */
    while (ACS_VDDRF_CTRL->READY_ALIAS != VDDRF_READY_BITBAND);

    /* Enable RF power switches */
    SYSCTRL_RF_POWER_CFG->RF_POWER_ALIAS   = RF_POWER_ENABLE_BITBAND;

    /* Remove RF isolation */
    SYSCTRL_RF_ACCESS_CFG->RF_ACCESS_ALIAS = RF_ACCESS_ENABLE_BITBAND;

    /* Start the 48 MHz oscillator without changing the other register bits */
    RF->XTAL_CTRL = ((RF->XTAL_CTRL & ~XTAL_CTRL_DISABLE_OSCILLATOR) |
                     XTAL_CTRL_REG_VALUE_SEL_INTERNAL);

    /* Enable 48 MHz oscillator divider at desired prescale value */
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_1_BYTE;

    /* Wait until 48 MHz oscillator is started */
    while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS !=
           ANALOG_INFO_CLK_DIG_READY_BITBAND);

    /* Switch to (divided 48 MHz) oscillator clock */
    Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1   |
                               EXTCLK_PRESCALE_1 |
                               SYSCLK_CLKSRC_RFCLK);

    /* Initialize usart driver structure */
    uart = &Driver_USART0;

    /* Initialize usart, register callback function */
    uart->Initialize(Usart_EventCallBack);

    uart->PowerControl(ARM_POWER_FULL);

    uart->Control(ARM_USART_MODE_ASYNCHRONOUS |
    					ARM_USART_DATA_BITS_8 |
						ARM_USART_PARITY_NONE |
						ARM_USART_STOP_BITS_1 |
						ARM_USART_FLOW_CONTROL_NONE,115200);

    uart->Control(ARM_USART_CONTROL_TX,1);
    uart->Control(ARM_USART_CONTROL_RX,1);
    /* Stop masking interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
    __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);
}

void RETIRODE_RMP_Handler(RETIRODE_RMP_Event_t event,
        						const void *p_param)
{
	switch(event)
	{
		case RETIRODE_RMP_EVENT_MEASUREMENT_DATA_READY:
		{
			int a = 5;
			break;
		}
		case RETIRODE_RMP_EVENT_QUERY_RESPONSE_READY:
		{
			int a = 5;
			break;
		}
		case RETIRODE_RMP_EVENT_ERROR:
		{
			int a = 5;
			break;
		}
		case RETIRODE_RMP_EVENT_READY:
		{
			int a = 5;
			break;
		}
	}

	return;
}


/* ----------------------------------------------------------------------------
 * Function      : int main(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the system and the USART driver. Configures
 *                 the button interrupt and sends data on button press.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int main(void)
{
	/* Initialize the system */
	Initialize();

	RETIRODE_RMP_Initialize(uart, RETIRODE_RMP_Handler);
	PRINTF("DEVICE INITIALIZED\n");

	//RETIRODE_RMP_WriteCommand("N64\r");
	int i = 0;
	/* Spin loop */
	while (true)
	{
		i++;
		RETIRODE_RMP_MainLoop();


		if(i == 100)
		{
			RETIRODE_RMP_PowerUpCommand();
		}



		if(i == 20000)
		{
			Sys_Delay_ProgramROM(SystemCoreClock);
			RETIRODE_RMP_QueryCommand(B_REGISTER);
		}


		if(i == 99000)
		{
			Sys_Delay_ProgramROM(SystemCoreClock);
			RETIRODE_RMP_MeasureCommand(1);
		}

		/* Refresh the watchdog timer */
 		Sys_Watchdog_Refresh();
	}
}

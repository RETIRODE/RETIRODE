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
DRIVER_GPIO_t *gpio;
char tx_buffer[] __attribute__ ((aligned(4))) = "R??\r";
char rx_buffer[sizeof(tx_buffer)] __attribute__ ((aligned(4)));


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

    /* Check if receive complete event occured */
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        /* Check if received data matches sent tx */
        if (!strcmp(tx_buffer, rx_buffer))
        {
            PRINTF("Data received");
        }

        /* Receive next data */
        uart->Receive(rx_buffer, sizeof(tx_buffer));
    }
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
//    while (DIO_DATA->ALIAS[RECOVERY_DIO] == 0);

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
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_6_BYTE;

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
    TRACE_INIT();
    PRINTF("DEVICE INITIALIZED\n");



    /* Non-blocking receive. User is notified through Usart_EventCallBack
     * after UART_BUFFER_SIZE bytes are received */
    uart->Receive(rx_buffer, (sizeof rx_buffer));


    uart->Send(tx_buffer, sizeof(tx_buffer)); /* start transmission */

    int received, sended;
    /* Spin loop */
    while (true)
    {
    	received = uart->GetRxCount();
    	sended = uart->GetTxCount();

        /* Refresh the watchdog timer */
        Sys_Watchdog_Refresh();
    }
}

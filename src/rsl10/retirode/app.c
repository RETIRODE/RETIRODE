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
char tx_buffer[] __attribute__ ((aligned(4))) = "RSL10 UART TEST";
char rx_buffer[sizeof(tx_buffer)] __attribute__ ((aligned(4)));


static APP_Environemnt_t app_env = { 0 };

/** Cache for storing of image data before it gets transmitted over BLE. */
static uint8_t app_data_cache_storage[APP_DATA_CACHE_SIZE];

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
void Button_EventCallback(uint32_t event)
{
    static bool ignore_next_dio_int = false;
    if (ignore_next_dio_int)
    {
        ignore_next_dio_int = false;
    }
    /* Button is pressed: Ignore next interrupt.
     * This is required to deal with the debounce circuit limitations. */
    else if (event == GPIO_EVENT_0_IRQ)
    {
		    ignore_next_dio_int = true;
        uart->Send(tx_buffer, sizeof(tx_buffer)); /* start transmission */
        PRINTF("BUTTON PRESSED: START TRANSMISSION\n");
		
    }
}

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
            /* Toggle led */
            ToggleLed(2, 500);
            PRINTF("LED BLINKED: CORRECT_DATA_RECEIVED\n");
        }

        /* Receive next data */
        uart->Receive(rx_buffer, sizeof(tx_buffer));
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void ToggleLed(uint32_t n, uint32_t delay_ms)
 * ----------------------------------------------------------------------------
 * Description   : Toggle the led pin.
 * Inputs        : n        - number of toggles
 *                  : delay_ms - delay between each toggle [ms]
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void ToggleLed(uint32_t n, uint32_t delay_ms)
{
    for (; n > 0; n--)
    {
        /* Refresh the watchdog */
        Sys_Watchdog_Refresh();

        /* Toggle led diode */
        gpio->ToggleValue(LED_DIO);

        /* Delay */
        Sys_Delay_ProgramROM((delay_ms / 1000.0) * SystemCoreClock);
    }
}


static void APP_ReadNextDataChunk(void)
{
    if ((!app_env.isp_read_in_progress)
        && (CIRCBUF_GetFree(&app_env.data_cache)
            >= 4*64))
    {


        app_env.isp_read_in_progress = true;
    }
}

static void APP_PushData(void)
{
    do
    {
        uint32_t max_data_to_push = RMTS_GetMax_TOFD_PushSize();
        uint32_t data_available = CIRCBUF_GetUsed(&app_env.data_cache);

        if ((max_data_to_push > 0) && (data_available > 0))
        {
            uint8_t buf[4*64];
            uint32_t status;
            uint32_t buf_to_write =
                    (max_data_to_push > data_available) ? data_available :
                                                          max_data_to_push;

            if (buf_to_write > 4*64)
            {
                buf_to_write = 4*64;
            }

            status = CIRCBUF_PopFront(buf, buf_to_write, &app_env.data_cache);
            //ENSURE(status == 0);

            status = RMTS_TOFD_Push(buf, buf_to_write);
            //ENSURE(status == PTSS_OK);

            /* for unused variable warnings. */
            (void)status;
        }
        else
        {
            break;
        }
    } while (1);
}

/**
 * Event handler for the Range Finder BLE service.
 */
void APP_RMTS_EventHandler(RMTS_ControlPointOpCode_t opcode,
        const void *p_param)
{
    switch (opcode)
    {
       /* Connected peer device requested continuous image capture. */
        case RMTS_OP_START_REQ:
        {


            break;
        }

        /* Connected peer device requested to abort any ongoing capture
         * operation.
         */
        case RMTS_OP_CANCEL_REQ:
        {

            break;
        }

        /* Connected peer device indicates that it received image info and is
         * ready to accept image data.
         */
        case RMTS_OP_DATA_TRANSFER_REQ:
        {

        	CIRCBUF_Initialize(app_data_cache_storage, APP_DATA_CACHE_SIZE,
        	        	                    &app_env.data_cache);

            app_env.isp_read_in_progress = 0;

            APP_ReadNextDataChunk();


            break;
        }


        /* RMTS is able to accept more data. */
        case RMTS_OP_DATA_SPACE_AVAIL_IND:
        {
            /* Push any cached data. */
            APP_PushData();

            /* Start receiving more image data if cache has enough free space.
             */
            APP_ReadNextDataChunk();

            break;
        }

        default:
        {

            break;
        }
    }
}


/**
 * Event handler for the External Sensors Trigger BLE service.
 */
void APP_ESTS_EventHandler(ESTS_RF_SETTING_ID_t sidx,
        const void *p_param)
{
    switch (sidx)
    {
       /* Connected peer device requested server reset. */
        case ESTS_OP_SW_RESET:
        {
            break;
        }
		case ESTS_OP_LASER_VOLTAGE:
		{
			break;
		}
		case ESTS_OP_S_BIAS_POWER_VOLTAGE:
		{
			break;
		}
		case ESTS_OP_CALIBRATE:
		{
			break;
		}
		case ESTS_OP_PULSE_COUNT:
		{
			break;
		}
		default:
		{

			break;
		}
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
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_6_BYTE;

    /* Wait until 48 MHz oscillator is started */
    while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS !=
           ANALOG_INFO_CLK_DIG_READY_BITBAND);

    /* Switch to (divided 48 MHz) oscillator clock */
    Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1   |
                               EXTCLK_PRESCALE_1 |
                               SYSCLK_CLKSRC_RFCLK);

    /* Initialize gpio structure */
    gpio = &Driver_GPIO;

    /* Initialize gpio driver */
    gpio->Initialize(Button_EventCallback);

    /* Initialize usart driver structure */
    uart = &Driver_USART0;

    /* Initialize usart, register callback function */
    uart->Initialize(Usart_EventCallBack);

    /* Stop masking interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
    __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);

    /* Initialize the kernel and Bluetooth stack */
        {
            int32_t status;

            APP_BLE_PeripheralServerInitialize(ATT_RMTS_COUNT);

            status = RMTS_Initialize(APP_RMTS_EventHandler);


        }
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
    int i;

    /* Initialize the system */
    Device_Initialize();
    PRINTF("DEVICE INITIALIZED\n");

    /* Non-blocking receive. User is notified through Usart_EventCallBack
     * after UART_BUFFER_SIZE bytes are received */
    //uart->Receive(rx_buffer, (sizeof tx_buffer));

    bool isp_busy = false;

    /* Spin loop */
    while (true)
    {
    	Sys_Watchdog_Refresh();
    	Kernel_Schedule();
        /* Here we demonstrate the ABORT_TRANSFER feature. Wait for first few bytes
         * to be transferred and if particular byte does not match abort the transfer. */
        //i = uart->GetRxCount() - BUFFER_OFFSET;
       // if ((i >= 0) && (tx_buffer[i] != rx_buffer[i]))
        //{
            /* Abort current receive operation */
         //   uart->Control(ARM_USART_ABORT_RECEIVE, 0);

            /* Re-start receive operation */
          //  uart->Receive(rx_buffer, (sizeof tx_buffer));
        //}

        /* Refresh the watchdog timer */
        //Sys_Watchdog_Refresh();
    }
}

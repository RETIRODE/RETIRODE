/* ----------------------------------------------------------------------------
 * Copyright (c) 2018 Semiconductor Components Industries, LLC (d/b/a
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
 * app_config.c
 * - Application configuration source file
 * ------------------------------------------------------------------------- */

#include "app.h"


static void Device_SetPowerSupplies(void)
{
    uint8_t trim_status;

    trim_status = Load_Trim_Values_And_Calibrate_MANU_CALIB();
    if (trim_status != VOLTAGES_CALIB_NO_ERROR)
    {
        while (true)
        {
            Sys_Watchdog_Refresh();
        }
    }

    /* Configure the current trim settings for VCC, VDDA */
    ACS_VCC_CTRL->ICH_TRIM_BYTE  = VCC_ICHTRIM_16MA_BYTE;
    ACS_VDDA_CP_CTRL->PTRIM_BYTE = VDDA_PTRIM_16MA_BYTE;

    /* Enable buck converter for VBAT > 1.4V */
    ACS_VCC_CTRL->BUCK_ENABLE_ALIAS = VCC_BUCK_BITBAND;

    /* Enable CDDRF to start 48 MHz XTAL oscillator */
    ACS_VDDRF_CTRL->ENABLE_ALIAS = VDDRF_ENABLE_BITBAND;
    ACS_VDDRF_CTRL->CLAMP_ALIAS  = VDDRF_DISABLE_HIZ_BITBAND;

    /* Wait until VDDRF supply has powered up */
    while (ACS_VDDRF_CTRL->READY_ALIAS != VDDRF_READY_BITBAND);

    /* Disable RF TX power amplifier supply voltage and
     * connect the switched output to VDDRF regulator */
    ACS_VDDPA_CTRL->ENABLE_ALIAS = VDDPA_DISABLE_BITBAND;
    ACS_VDDPA_CTRL->VDDPA_SW_CTRL_ALIAS    = VDDPA_SW_VDDRF_BITBAND;

    /* Enable RF power switches */
    SYSCTRL_RF_POWER_CFG->RF_POWER_ALIAS   = RF_POWER_ENABLE_BITBAND;
}

/**
 * Configures frequencies of internal clocks required for operation.
 *
 * Clocks must be configured after POR and after wake-up from deep sleep.
 */
static void Device_SetClockDividers(void)
{
    /* Enable the 48 MHz oscillator divider using the desired prescale value */
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_1_BYTE;

    /* Wait until 48 MHz oscillator is started */
    while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS !=
           ANALOG_INFO_CLK_DIG_READY_BITBAND);

    /* Switch to (divided 48 MHz) oscillator clock */
    Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1   |
                               EXTCLK_PRESCALE_1 |
                               SYSCLK_CLKSRC_RFCLK);

    /* Configure clock dividers
     * SLOWCLK = 1 MHz
     * BBCLK = 16 MHz
     * BBCLK_DIV = 1 MHz
     * USRCLK = 48 MHz
     * CPCLK = 250 kHz
     * DCCLK = 12 MHz - Recommended DCCLK frequency for 3.3V
     */
    CLK->DIV_CFG0 = (SLOWCLK_PRESCALE_48 | BBCLK_PRESCALE_6 |
                     USRCLK_PRESCALE_1);
    CLK->DIV_CFG2 = (CPCLK_PRESCALE_4 | DCCLK_PRESCALE_4);

    /* Configure peripheral clock dividers:
     * PWM0CLK = 100 kHz
     * PWM1CLK = 100 kHz
     * UARTCLK - Configured later by peripheral driver.
     * AUDIOCLK - Unused
     */
    CLK_DIV_CFG1->PWM0CLK_PRESCALE_BYTE = 9;
    CLK_DIV_CFG1->PWM1CLK_PRESCALE_BYTE = 9;

    BBIF->CTRL    = (BB_CLK_ENABLE | BBCLK_DIVIDER_8 | BB_DEEP_SLEEP);
}

static void Device_ResetDioPads(void)
{
    /* Set initial DIO configuration.
     * Overrides the default value after POR (DISABLED with weak pull-up) to
     * all pads disabled with no pull resistors.
     */
    for (int i = 0; i < 16; ++i)
    {
        Sys_DIO_Config(i, DIO_MODE_DISABLE);
    }

    /* Lower drive strength (required when VDDO > 2.7)*/
    DIO->PAD_CFG = PAD_LOW_DRIVE;

    /* Disable JTAG debug port to gain application control over DIO 13,14,15 */
    DIO_JTAG_SW_PAD_CFG->CM3_JTAG_DATA_EN_ALIAS = 0;
    DIO_JTAG_SW_PAD_CFG->CM3_JTAG_TRST_EN_ALIAS = 0;
}

/**
 * Configure hardware and initialize BLE stack.
 */
void Device_Initialize(void)
{
    /* Mask all interrupts */
    __set_PRIMASK(PRIMASK_DISABLE_INTERRUPTS);

    /* Disable all interrupts and clear any pending interrupts */
    Sys_NVIC_DisableAllInt();
    Sys_NVIC_ClearAllPendingInt();

    Device_SetPowerSupplies();

    /* Remove RF isolation to start 48 MHz oscillator. */
    SYSCTRL_RF_ACCESS_CFG->RF_ACCESS_ALIAS = RF_ACCESS_ENABLE_BITBAND;

    /* Start the 48 MHz oscillator without changing the other register bits */
    RF->XTAL_CTRL = ((RF->XTAL_CTRL & ~XTAL_CTRL_DISABLE_OSCILLATOR) |
                     XTAL_CTRL_REG_VALUE_SEL_INTERNAL);

    /* Configure internal clock frequencies. */
    Device_SetClockDividers();

    Device_ResetDioPads();

    /* Disable pad retention in case of deep sleep wake-up from flash.
     *
     * Peripherals (i.e. DIO, I2C) are not guaranteed to report correct DIO pad
     *  status before this point in certain deep sleep use cases.
     */


    /* Start the XTAL32K crystal oscillator.
     * Required for RTC and deep sleep operation.
     */


    /* Start RTC counter. */


    /* Seed the random number generator.
     * Required dependency for BLE stack.
     */
    srand(1);

    /* Set default interrupt priority for all interrupts.
     * This allows to use BASEPRI based critical sections since BASEPRI is
     * unable to mask interrupts with highest priority (0).
     *
     * Modules that use BASEPRI critical sections by default on CM3:
     * SEGGER RTT, SEGGER SYSVIEW, CMSIS-FreeRTOS
     */
    for (int i = WAKEUP_IRQn; i < BLE_AUDIO2_IRQn; ++i)
    {
        NVIC_SetPriority(i, 1);
    }




    /* APP INITIALIZE  */

    /* Initialize the kernel and Bluetooth stack */


    {
		APP_BLE_PeripheralServerInitialize(ATT_RMTS_COUNT + ATT_ESTS_COUNT);

		RMTS_Initialize(APP_RMTS_EventHandler);

		ESTS_Initialize(APP_ESTS_EventHandler);

    }


#if (CFG_SMARTSHOT_APP_SLEEP_ENABLED == 1)
    /* Trim startup RC oscillator to 3 MHz (required by Sys_PowerModes_Wakeup)
     */
    Sys_Clocks_OscRCCalibratedConfig(3000);

    /* Configure deep sleep feature. */
    APP_SleepModeConfigure();
#endif /* if (CFG_SMARTSHOT_APP_SLEEP_ENABLED == 1) */

    /* Initialize peripheral drivers.
     * Automatic peripheral configuration must be enabled in RTE_Device.h !
     */

    /* Stop masking interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
    __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);


}

/**
 * Restore hardware state after wake-up from deep sleep mode with RAM retention.
 *
 * - Restores system clock dividers
 * - Restores DIO pad configuration before disabling PAD retention feature.
 * - Wait for BLE stack to fully wake up.
 */


/* ----------------------------------------------------------------------------
 * Function      : Device_Param_Prepare(struct app_device_param * param)
 * ----------------------------------------------------------------------------
 * Description   : This function allows the application to overwrite a few BLE
 *                 parameters (BD address and keys) without having to write
 *                 data into RSL10 flash (NVR3). This function is called by the
 *                 stack and it's useful for debugging and testing purposes.
 * Inputs        : - param    - pointer to the parameters to be configured
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */



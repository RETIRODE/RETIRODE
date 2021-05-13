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



#define MEASURE_SIZE   5
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
void Usart_EventCallBack(uint32_t event)
{
	RETIRODE_RMP_UARTEventHandler(event);
}

static void APP_PushData(void)
{
    do
    {
        uint32_t max_data_to_push = RMTS_GetMax_TOFD_PushSize();
        uint32_t data_available = CIRCBUF_GetUsed(&app_env.data_cache);

        if ((max_data_to_push > 0) && (data_available > 0))
        {
            uint8_t buf[4 * 64];
            uint32_t status;
            uint32_t buf_to_write =
                    (max_data_to_push > data_available) ? data_available :
                                                          max_data_to_push;

            if (buf_to_write > 4 * 64)
            {
                buf_to_write = 4 * 64;
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
 * Event handler for the Range Finder BLE service.*/

void APP_RMTS_EventHandler(RMTS_ControlPointOpCode_t opcode,
        const void *p_param)
{
    switch (opcode)
    {
       /* Connected peer device requested continuous image capture.*/
        case RMTS_OP_START_REQ:
        {

        	CIRCBUF_Initialize(app_data_cache_storage, APP_DATA_CACHE_SIZE,
        	        	                    &app_env.data_cache);

        	RETIRODE_RMP_MeasureCommand(MEASURE_SIZE);
        	app_env.measurement_in_progress = true;

            break;
        }

        /* Connected peer device requested to abort any ongoing capture
         * operation.*/
        case RMTS_OP_CANCEL_REQ:
        {
        	if(app_env.measurement_in_progress)
        	{
        		app_env.stop_measurement_command = true;
        	}
            break;
        }

        /* Connected peer device indicates that it received image info and is
         * ready to accept image data.*/

        case RMTS_OP_DATA_TRANSFER_REQ:
        {

            APP_PushData();
            break;
        }

        case RMTS_OP_DATA_SPACE_AVAIL_IND:
		{
			APP_PushData();
			break;
		}

        case RMTS_OP_DATA_TRANSFER_COMPLETED:
		{
			if(app_env.measurement_in_progress)
			{
				if(app_env.stop_measurement_command == false)
				{
					RETIRODE_RMP_MeasureCommand(MEASURE_SIZE);
				}
				else
				{
					app_env.stop_measurement_command = false;
					app_env.measurement_in_progress = false;
				}
			}


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
**/

void APP_ESTS_Push_Query_Data(const RETIRODE_RMP_Query_response_t *p_param)
{

	uint32_t response[3];
	switch (p_param->reg)
	{
		case RETIRODE_RMP_ACTUAL_LASER_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_LASER_VOLTAGE;
			response[1] = ESTS_OP_LASER_VOLTAGE_ACTUAL;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_TARGET_LASER_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_LASER_VOLTAGE;
			response[1] = ESTS_OP_LASER_VOLTAGE_TARGET;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_ACTUAL_BIAS_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_S_BIAS_POWER_VOLTAGE;
			response[1] = ESTS_OP_S_BIAS_POWER_VOLTAGE_ACTUAL;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_TARGET_BIAS_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_S_BIAS_POWER_VOLTAGE;
			response[1] = ESTS_OP_S_BIAS_POWER_VOLTAGE_TARGET;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_PULSE_COUNT_REGISTER:
		{
			response[0] = ESTS_OP_PULSE_COUNT;
			response[1] = ESTS_OP_PULSE_COUNT_VALUE;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_DCD_CONFIG_REGISTER:
		{
			response[0] = ESTS_OP_VOLTAGES_STATUS;
			response[1] = p_param->value;
			response[2] = 0;
			break;
		}
	}

	ESTS_NOTIFY_QUERY_RESPONSE(response);
}

void APP_ESTS_Push_Calibration_Data(const RETIRODE_RMP_CalibrationDataReady_response_t *p_param)
{
	uint32_t response[3];
	switch (p_param->cal)
	{
		case RETIRODE_RMP_CALIBRATION_0ns:
		{
			response[0] = ESTS_OP_CALIBRATE;
			response[1] = ESTS_OP_CALIBRATE_FIRST;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_CALIBRATION_62p5ns:
		{
			response[0] = ESTS_OP_CALIBRATE;
			response[1] = ESTS_OP_CALIBRATE_SECOND;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_CALIBRATION_125ns:
		{
			response[0] = ESTS_OP_CALIBRATE;
			response[1] = ESTS_OP_CALIBRATE_THIRD;
			response[2] = p_param->value;
			break;
		}
		case RETIRODE_RMP_CALIBRATION_DONE:
		{
			break;
		}
	}

	ESTS_NOTIFY_QUERY_RESPONSE(response);
}

/**
 * Event handler for the External Sensors Trigger BLE service.
**/


void APP_ESTS_EventHandler(ESTS_RF_SETTING_ID_t sidx,
        const void *p_param)
{
    switch (sidx)
    {
       /** Connected peer device requested server reset. **/
        case ESTS_OP_SW_RESET:
        {
        	RETIRODE_RMP_SoftwareResetCommand();
            break;
        }
		case ESTS_OP_LASER_VOLTAGE:
		{
			const ETSS_LASER_VOLTAGE_params_t *params = p_param;
			if(params->is_query == true)
			{
				if(params->type == 0x01)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_TARGET_LASER_VOLTAGE_REGISTER);
				}
				if(params->type == 0x02)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_ACTUAL_LASER_VOLTAGE_REGISTER);
				}
			}
			else
			{
				if(params->type == 0x01)
				{
					RETIRODE_RMP_SetLaserPowerTargetVoltateCommand(params->value);
				}
				if(params->type == 0x03)
				{
					RETIRODE_RMP_SetLaserPowerEnabledCommand(params->value);
				}
			}
			break;
		}
		case ESTS_OP_S_BIAS_POWER_VOLTAGE:
		{
			const ETSS_S_BIAS_POWER_VOLTAGE_params_t *params = p_param;
			if(params->is_query == true)
			{
				if(params->type == 0x01)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_TARGET_BIAS_VOLTAGE_REGISTER);
				}
				if(params->type == 0x02)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_ACTUAL_BIAS_VOLTAGE_REGISTER);
				}
			}
			else
			{
				if(params->type == 0x01)
				{
					RETIRODE_RMP_SetPowerBiasTargetVoltateCommand(params->value);
				}
				if(params->type == 0x03)
				{
					RETIRODE_RMP_SetPowerBiasEnabledCommand(params->value);
				}
			}
			break;
		}
		case ESTS_OP_CALIBRATE:
		{
			const ETSS_CALIBRATE_params_t *params = p_param;
			if(params->is_query == true)
			{
				//SEND UART QUERY REQUEST
			}else
			{
				switch(params->type)
				{
					case ESTS_OP_CALIBRATE_FIRST:
					{
						RETIRODE_RMP_CalibrateCommand(RETIRODE_RMP_CALIBRATION_0ns);
						break;
					}
					case ESTS_OP_CALIBRATE_SECOND:
					{
						RETIRODE_RMP_CalibrateCommand(RETIRODE_RMP_CALIBRATION_62p5ns);
						break;
					}
					case ESTS_OP_CALIBRATE_THIRD:
					{
						RETIRODE_RMP_CalibrateCommand(RETIRODE_RMP_CALIBRATION_125ns);
						break;
					}
					case ESTS_OP_CALIBRATE_DONE:
					{
						RETIRODE_RMP_CalibrateCommand(RETIRODE_RMP_CALIBRATION_DONE);
						break;
					}
				}
			}
			break;
		}
		case ESTS_OP_PULSE_COUNT:
		{
			const ETSS_PULSE_COUNT_params_t *params = p_param;
			if(params->is_query == true)
			{
				RETIRODE_RMP_QueryCommand(RETIRODE_RMP_PULSE_COUNT_REGISTER);
			}
			else
			{
				RETIRODE_RMP_SetPulseCountCommand(params->value);
			}
			break;
		}
		case ESTS_OP_VOLTAGES_STATUS:
		{
			const ESTS_VOLTAGES_STATUS_params_t *params = p_param;
			if(params->is_query == true)
			{
				RETIRODE_RMP_QueryCommand(RETIRODE_RMP_DCD_CONFIG_REGISTER);
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

void RETIRODE_RMP_Handler(RETIRODE_RMP_Event_t event,
        						const void *p_param)
{
 	switch(event)
	{
		case RETIRODE_RMP_EVENT_MEASUREMENT_DATA_READY:
		{
			const RETIRODE_RMP_Data_t *data = p_param;
			CIRCBUF_PushBack(data->data, data->size * 4, &app_env.data_cache);
			RMTS_Start_TOFD_Transfer(data->size * 4);
			break;
		}
		case RETIRODE_RMP_EVENT_QUERY_RESPONSE_READY:
		{
			const RETIRODE_RMP_Query_response_t *params = p_param;
			APP_ESTS_Push_Query_Data(params);
			break;
		}
		case RETIRODE_RMP_EVENT_CALIBRATION_DATA_READY:
		{
			const RETIRODE_RMP_CalibrationDataReady_response_t *params = p_param;
			APP_ESTS_Push_Calibration_Data(params);
			break;
		}
		case RETIRODE_RMP_EVENT_ERROR:
		{
			break;
		}
		case RETIRODE_RMP_EVENT_READY:
		{
			break;
		}
	}

	return;
}

int main(void)
{
    /* Initialize the system */
    Device_Initialize();
    CIRCBUF_Initialize(app_data_cache_storage, APP_DATA_CACHE_SIZE,
            	        	                    &app_env.data_cache);

	int i = 0;
	/* Spin loop */
	while (true)
	{
		i++;

		RETIRODE_RMP_MainLoop();

		if(i == 20)
		{
			RETIRODE_RMP_SoftwareResetCommand();
		}

		if(i == 200)
		{
			RETIRODE_RMP_PowerUpCommand();
		}
		/* Refresh the watchdog timer */
		Sys_Watchdog_Refresh();
		Kernel_Schedule();
	}


}

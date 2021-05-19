#include "app.h"
#include <printf.h>

static APP_Environemnt_t app_env = { 0 };

/** Cache for storing of image data before it gets transmitted over BLE. */
static uint8_t app_data_cache_storage[APP_DATA_CACHE_SIZE];


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
        	app_env.stop_measurement_command = false;

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

	uint8_t response[6];
	switch (p_param->reg)
	{
		case RETIRODE_RMP_ACTUAL_LASER_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_LASER_VOLTAGE;
			response[1] = ESTS_OP_LASER_VOLTAGE_ACTUAL;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
			break;
		}
		case RETIRODE_RMP_TARGET_LASER_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_LASER_VOLTAGE;
			response[1] = ESTS_OP_LASER_VOLTAGE_TARGET;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
			break;
		}
		case RETIRODE_RMP_ACTUAL_BIAS_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_S_BIAS_POWER_VOLTAGE;
			response[1] = ESTS_OP_S_BIAS_POWER_VOLTAGE_ACTUAL;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
			break;
		}
		case RETIRODE_RMP_TARGET_BIAS_VOLTAGE_REGISTER:
		{
			response[0] = ESTS_OP_S_BIAS_POWER_VOLTAGE;
			response[1] = ESTS_OP_S_BIAS_POWER_VOLTAGE_TARGET;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
			break;
		}
		case RETIRODE_RMP_PULSE_COUNT_REGISTER:
		{
			response[0] = ESTS_OP_PULSE_COUNT;
			response[1] = ESTS_OP_PULSE_COUNT_VALUE;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
			break;
		}
		case RETIRODE_RMP_DCD_CONFIG_REGISTER:
		{
			response[0] = ESTS_OP_VOLTAGES_STATUS;
			response[1] = ESTS_OP_VOLTAGES_STATUS_VALUE;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
			break;
		}
	}

	ESTS_NOTIFY_QUERY_RESPONSE(response);
}

void APP_ESTS_Push_Calibration_Data(const RETIRODE_RMP_CalibrationDataReady_response_t *p_param)
{
	uint8_t response[6];
	switch (p_param->cal)
	{
		case RETIRODE_RMP_CALIBRATION_0ns:
		{
			response[0] = ESTS_OP_CALIBRATE;
			response[1] = ESTS_OP_CALIBRATE_FIRST;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
			break;
		}
		case RETIRODE_RMP_CALIBRATION_62p5ns:
		{
			response[0] = ESTS_OP_CALIBRATE;
			response[1] = ESTS_OP_CALIBRATE_SECOND;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
			break;
		}
		case RETIRODE_RMP_CALIBRATION_125ns:
		{
			response[0] = ESTS_OP_CALIBRATE;
			response[1] = ESTS_OP_CALIBRATE_THIRD;
			memcpy(response + 2, &p_param->value, sizeof(uint32_t));
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
				if(params->type == ESTS_OP_LASER_VOLTAGE_TARGET)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_TARGET_LASER_VOLTAGE_REGISTER);
				}
				if(params->type == ESTS_OP_LASER_VOLTAGE_ACTUAL)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_ACTUAL_LASER_VOLTAGE_REGISTER);
				}
			}
			else
			{
				if(params->type == ESTS_OP_LASER_VOLTAGE_TARGET)
				{
					RETIRODE_RMP_SetLaserPowerTargetVoltateCommand(params->value);
				}
				if(params->type == ESTS_OP_LASER_VOLTAGE_SWITCH)
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
				if(params->type == ESTS_OP_S_BIAS_POWER_VOLTAGE_TARGET)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_TARGET_BIAS_VOLTAGE_REGISTER);
				}
				if(params->type == ESTS_OP_S_BIAS_POWER_VOLTAGE_ACTUAL)
				{
					RETIRODE_RMP_QueryCommand(RETIRODE_RMP_ACTUAL_BIAS_VOLTAGE_REGISTER);
				}
			}
			else
			{
				if(params->type == ESTS_OP_S_BIAS_POWER_VOLTAGE_TARGET)
				{
					RETIRODE_RMP_SetPowerBiasTargetVoltateCommand(params->value);
				}
				if(params->type == ESTS_OP_S_BIAS_POWER_VOLTAGE_SWITCH)
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
				//NOT SUPPORTED
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
			RMTS_Abort_TOFD_Transfer(RMTS_INFO_ERR_ABORTED_BY_SERVER);
			break;
		}
		case RETIRODE_RMP_EVENT_READY:
		{
			break;
		}
	}

	return;
}


void RMP_Initialize(void)
{
	int i = 0;
	while (true)
	{
		i++;
		RETIRODE_RMP_MainLoop();

		if(i == 5)
		{
			RETIRODE_RMP_SoftwareResetCommand();
		}

		if(i == 50)
		{
			RETIRODE_RMP_PowerUpCommand();
		}

		if(i == 100)
		{
			break;
		}
	}
}

int main(void)
{
    /* Initialize the system */
    Device_Initialize();
    CIRCBUF_Initialize(app_data_cache_storage, APP_DATA_CACHE_SIZE,
            	        	                    &app_env.data_cache);
    //RMP_Initialize();
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


		if(i == 2000)
		{
			RETIRODE_RMP_QueryCommand('R');
		}

		/* Refresh the watchdog timer */
 		Sys_Watchdog_Refresh();
		Kernel_Schedule();
	}


}

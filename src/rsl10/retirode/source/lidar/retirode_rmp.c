/*
 *retirode_lmp.c
 *
 *Created on: Apr 4, 2021
 *     Author: Trojan Martin
 */
/**
 * @file retirode_rmp.c
 *
 * @addtogroup RETIRODE_RMP
 * @{
 *
 * @addtogroup RETIRODE_RMP
 * @{
 */

#include <retirode_rmp.h>
#include <rsl10.h>
static char hex[] = "0123456789ABCDEF";

#define QUERY_STRING 				"%??\r"
#define COMMAND_STRING 				"%$1\r"
#define QUERY(dest, reg) 			ReplaceChar(dest, '%', reg)
#define COMMAND(dest, reg, value)   ReplaceChar(dest, '%', reg); WriteValueToCommand(dest, '$', value);

#define MAX_QUERY_WAIT_COUNT		100
#define MAX_MUEASURE_WAIT_COUNT		20000


static char* ReplaceChar(char* str, char find, char replace);
static void WriteValueToCommand(char* str,char find, uint8_t val);

const float TRIGGER_GENERATOR_TICK = 64e-6f;    // trigger generator time base in HW

const uint16_t TRIGGER_MIN_PERIOD_NATIVE = 15;
const float TRIGGER_MIN_PERIOD = TRIGGER_MIN_PERIOD_NATIVE * TRIGGER_GENERATOR_TICK;

const uint16_t TRIGGER_MAX_PERIOD_NATIVE = 65535;
const float TRIGGER_MAX_PERIOD = TRIGGER_MAX_PERIOD_NATIVE * TRIGGER_GENERATOR_TICK;

// DC-DC controller target voltage limits
const float LASER_MAX_TARGET_VOLTAGE = 16.0;    // maximum setable LASER power supply target voltage
const float LASER_MIN_TARGET_VOLTAGE = 5.0;     // minimum setable LASER power supply target voltage

const float BIAS_MAX_TARGET_VOLTAGE = -45.0;    // maximum setable BIAS power supply target voltage
const float BIAS_MIN_TARGET_VOLTAGE = -32.0;    // minimum setable BIAS power supply target voltage

// DC-DC controller ADC conversion constants
const float R22_UPPER = 100e3;                  // upper sensing resistor for LASER power supply output voltage measurement
const float R27_LOWER = 8200;                   // lower sensing resistor for LASER power supply output voltage measurement
const float LASER_MIN_MEAS_VOLTAGE = 4.0;       // minimum voltage measured at LASER power supply output
const float LASER_MAX_MEAS_VOLTAGE = 17.5;      // maximum voltage measured at LASER power supply output

const float R34_UPPER = 22e3;                   // upper sensing resistor for BIAS power supply output voltage measurement
const float R32_LOWER = 348e3;                  // lower sensing resistor for BIAS power supply output voltage measurement
const float BIAS_MIN_MEAS_VOLTAGE = -30.0;      // minimum voltage measured at BIAS power supply voltage
const float BIAS_MAX_MEAS_VOLTAGE = -48.0;      // maximum voltage measured at BIAS power supply voltage

const float ADC_TAU_LASER = (float) 27.7;       // time constant R25,C25 in ms - reduced for better coverage - tailored for LASER power supply according prototypes
const float ADC_TAU_BIAS = (float) 27.4;        // time constant R25,C25 in ms - reduced for better coverage - tailored for BIAS power supply according prototypes
const float ADC_SUPPLY_VOLTAGE = (float) 3.3;   // ADC comparator supply voltage
const float ADC_OFFSET_LASER = (float) 36.5;    // calculation offset correction for LASER power supply
const float ADC_OFFSET_BIAS = (float) 31.4;     // calculation offset correction for BIAS power supply
const float ADC_LOWEST_COMP_VOLTAGE = (float) 0.55;     // compensating value for calculation of BIAS voltage values
const float ADC_FREQUENCY = 16.0;                       // ADC system clock in MHz

/**
 *Array of internal flags about the state of library and which application
 *issued commands are pending.
 */
struct RETIRODE_RMP_EnvironmentFlags_t
{
	bool cmd_software_reset;

	/**Command to start power up of the RMP was received. */
	bool cmd_power_on;

	/**Command to power off RMP was received. */
	bool cmd_power_off;

	/**Command to measure n samples*/
	uint32_t cmd_measure;

	/** Query register command */
	char cmd_query;

	/** Calibration */
	uint32_t calibration;

	/**Flag to indicate that RMP is not in shutdown mode. */
	bool is_powered;

	/**Flag to indicate that RMP is ready to start next UART transaction. */
	bool is_uart_ready;


};


struct RETIRODE_RMP_CurrentMeasurement_t
{
	/* Holds received RAW data from UART */
	char received_data_buffer[RETIRODE_RMP_DATA_RECEIVED_BUFFER_SIZE];

	/** Holds already processed data */
	float requested_data_buffer[200];

	/** Hold information about current size of processed data */
	uint32_t requested_data_proccesed;

	/** Current requested calibration */
	uint32_t calibration;
};

/** Hold information about current setting command */
struct RETIRODE_RMP_SettingCommand_t
{
	bool low_high;
	char reg;
	char reg2;
	uint16_t value;
};

/** Hold information about current C register state */
struct RETIRODE_RMP_DCD_CONFIG_REGISTER_t
{
   bool laserPowerEnable;
   bool biasPowerEnable;
};

/**
 *Holds all information about current state of RMP
 */
struct RETIRODE_RMP_Eviroment_t
{
	/**Pointer to UART CMSIS-Driver used to communicate with the LIDAR. */
	ARM_DRIVER_USART * uart;

	/**
	 *Flag set from UART Event Handler used to determine status of UART
	 *transaction.
	 *
	 *Some error flags are exposed only using the shared Event Handler,
	 *therefore we store the value given in UART interrupt and process it in
	 *then read / write routines.
	 */
	volatile uint32_t uart_event;

	/**
	 *Current state of RMP operation.
	 *
	 *Used to call appropriate state handler in main loop.
	 */
	RETIRODE_RMP_State_t state;

	/** Flags about RMP state and application issued commands. */
	volatile struct RETIRODE_RMP_EnvironmentFlags_t flag;

	/** Holds information about current measurement. */
	struct RETIRODE_RMP_CurrentMeasurement_t current_measurement;

	/** Current issued command */
	struct RETIRODE_RMP_SettingCommand_t current_command;

	/** State of the main configuration register */
	struct RETIRODE_RMP_DCD_CONFIG_REGISTER_t d_register_state;

	/** Pulse count to be used in measurement. If was not defined before, default value is 100 */
	uint8_t pulse_count;

	/** Holds latest query response */
	char query_response_buffer[4];
	/**
	 *Application assigned event handler used to notify application on
	 *certain events.
	 */
	RETIRODE_RMP_EventHandler_t p_evt_handler;

	uint32_t query_response_waitter;
	uint32_t mueasurement_response_waitter;
};

/**@internal */

/* ----------------------------------------------------------------------------
 * Private Function Forward Declaration
 * ------------------------------------------------------------------------- */

typedef void(*RETIRODE_RMP_StateHandler_t)(void);

static void RETIRODE_RMP_ShutdownStateHandler(void);
static void RETIRODE_RMP_IdleStateHandler(void);
static void RETIRODE_RMP_SettingsStateHandler(void);
static void RETIRODE_RMP_CalibrateStateHandler(void);
static void RETIRODE_RMP_QueryStateHandler(void);
static void RETIRODE_RMP_QueryResponseStateHandler(void);
static void RETIRODE_RMP_MeasureStateHandler(void);
static void RETIRODE_RMP_DataProcessingStateHandler(void);
static void RETIRODE_RMP_MeasurementDataReadyStateHandler(void);

static void RETIRODE_RMP_CalibrateMeasureCommand();
static uint8_t FromRealLaserToNative(float realVoltage);
static float FromNativeLaserToReal(uint8_t nativeVoltage);
static uint8_t FromRealBiasToNative(float realVoltage);
static float FromNativeBiasToReal(uint8_t nativeVoltage);

/* ----------------------------------------------------------------------------
 * Private Global variables Declaration
 * ------------------------------------------------------------------------- */

static struct RETIRODE_RMP_Eviroment_t rmp_env;

static const RETIRODE_RMP_StateHandler_t retirode_lmp_state_handler[RETIRODE_RMP_STATE_MAX] = {
[RETIRODE_RMP_STATE_SHUTDOWN] = RETIRODE_RMP_ShutdownStateHandler,
[RETIRODE_RMP_STATE_IDLE] = RETIRODE_RMP_IdleStateHandler,
[RETIRODE_RMP_STATE_CALIBRATE] = RETIRODE_RMP_CalibrateStateHandler,
[RETIRODE_RMP_STATE_SETTING] = RETIRODE_RMP_SettingsStateHandler,
[RETIRODE_RMP_STATE_QUERY] = RETIRODE_RMP_QueryStateHandler,
[RETIRODE_RMP_STATE_QUERY_RESPONSE] = RETIRODE_RMP_QueryResponseStateHandler,
[RETIRODE_RMP_STATE_MEASURE] = RETIRODE_RMP_MeasureStateHandler,
[RETIRODE_RMP_STATE_MEASURE_DATA_PROCESSING] = RETIRODE_RMP_DataProcessingStateHandler,
[RETIRODE_RMP_STATE_MEASUREMENT_DATA_READY] = RETIRODE_RMP_MeasurementDataReadyStateHandler
};

static char* ReplaceChar(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    *current_pos = replace;
    return str;
}

void convert_to_hex_str(char* str, uint8_t* val, size_t val_count)
{
	for (size_t i = 0; i < val_count; i++)
	{
		str[(i * 2) + 0] = hex[((val[i] & 0xF0) >> 4)];
		str[(i * 2) + 1] = hex[((val[i] & 0x0F) >> 0)];
	}
}

static void WriteValueToCommand(char * str, char find, uint8_t val)
{
	convert_to_hex_str(str + 1, &val, 1);
}

/**
 *Wrapper function for setting of new UART state.
 *
 *Allows to add state change debug messages here if necessary.
 */
static void RETIRODE_RMP_SetState(const RETIRODE_RMP_State_t new_state)
{
	rmp_env.state = new_state;
}

/**
 *Issue a UART write command to Range Finder.
 *
 *These commands are used to issue commands to the Range Finder to either start a
 *specific operation or to query data.
 *
 */
int32_t RETIRODE_RMP_WriteCommand(char *cmd)
{
	int32_t status;
	rmp_env.uart_event = 0;

	//wait for uart if busy
	while (rmp_env.uart->GetStatus().tx_busy);

	status = rmp_env.uart->Send(cmd, 4);

	if (status == ARM_DRIVER_OK)
	{
		//wait for uart
		while (rmp_env.uart->GetStatus().tx_busy);

		if (rmp_env.uart_event == ARM_USART_EVENT_SEND_COMPLETE)
		{
			status = ARM_DRIVER_OK;
		}
		else
		{
			status = ARM_DRIVER_ERROR;
		}
	}
	else
	{
		status = ARM_DRIVER_ERROR;
	}

	return status;
}

static int32_t RETIRODE_RMP_UARTReceive(void *data, uint32_t len)
{
	while(rmp_env.uart->GetStatus().rx_busy);
	return rmp_env.uart->Receive(data, len);
}

static int32_t RETIRODE_RMP_UARTReceiveQueryResponse()
{
	return RETIRODE_RMP_UARTReceive((void *)rmp_env.query_response_buffer, 4);
}

static int32_t RETIRODE_RMP_UARTReceiveMeasurementData(uint8_t len)
{
	return RETIRODE_RMP_UARTReceive((void *)rmp_env.current_measurement.received_data_buffer, len * 4);
}

void RETIRODE_RMP_UARTEventHandler(uint32_t event)
{
	rmp_env.uart_event = event;
}

static void RETIRODE_RMP_SendDataReadyEvent(
		RETIRODE_RMP_Data_t *p_data)
{
    if (rmp_env.p_evt_handler != NULL)
    {
    	rmp_env.p_evt_handler(RETIRODE_RMP_EVENT_MEASUREMENT_DATA_READY, p_data);
    }
}

static void RETIRODE_RMP_SendCalibrationDataReadyEvent(
		RETIRODE_RMP_CalibrationDataReady_response_t *p_data)
{
    if (rmp_env.p_evt_handler != NULL)
    {
    	rmp_env.p_evt_handler(RETIRODE_RMP_EVENT_CALIBRATION_DATA_READY, p_data);
    }
}

static void RETIRODE_RMP_SendQueryResponseReadyEvent(
		RETIRODE_RMP_Query_response_t *p_data)
{
    if (rmp_env.p_evt_handler != NULL)
    {
    	rmp_env.p_evt_handler(RETIRODE_RMP_EVENT_QUERY_RESPONSE_READY, p_data);
    }
}

static void RETIRODE_RMP_SendErrorIndication(RETIRODE_RMP_State_t state,
	RETIRODE_RMP_Err_t error)
{
	RETIRODE_RMP_ErrorEvent_t err_event;

	err_event.state = state;
	err_event.error = error;

	if (rmp_env.p_evt_handler != NULL)
	{
		rmp_env.p_evt_handler(RETIRODE_RMP_EVENT_ERROR, &err_event);
	}
}

static void RETIRODE_RMP_ShutdownStateHandler(void)
{
	if (rmp_env.flag.cmd_power_on)
	{
		rmp_env.flag.is_powered = true;
		rmp_env.flag.cmd_power_on = false;
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
	}
}

static void RETIRODE_RMP_IdleStateHandler(void)
{
	/** Query command */
	if(rmp_env.flag.cmd_query)
	{
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_QUERY);
	}
	/** Setting command */
	else if(rmp_env.current_command.reg)
	{
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_SETTING);
	}
	/** Calibration routine */
	else if(rmp_env.flag.calibration)
	{
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_CALIBRATE);
	}
	/** Measure data */
	else if (rmp_env.flag.cmd_measure)
	{
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_MEASURE);
	}
}

static void RETIRODE_RMP_CalibrateStateHandler(void)
{
	int32_t status = ARM_DRIVER_OK;

	uint16_t commandByte = 0;
	char command[4] = COMMAND_STRING;
	switch(rmp_env.flag.calibration)
	{
		case RETIRODE_RMP_CALIBRATION_0ns:
		{
			commandByte |= 0x04;
			break;
		}
		case RETIRODE_RMP_CALIBRATION_62p5ns:
		{
			commandByte |= 0x08;
			break;
		}
		case RETIRODE_RMP_CALIBRATION_125ns:
		{
			commandByte |= 0x0c;
			break;
		}
		case RETIRODE_RMP_CALIBRATION_DONE:
		{
			//TODO: Add command for enabling, now enabled by default
			//enable laser pulse
			commandByte |= 0x10;
			break;
		}
	}

	COMMAND(command, RETIRODE_RMP_MAIN_CONFIG_REGISTER, commandByte);
	status = RETIRODE_RMP_WriteCommand(command);

	if (status == ARM_DRIVER_OK)
	{
		if(rmp_env.flag.calibration != RETIRODE_RMP_CALIBRATION_DONE)
		{
			RETIRODE_RMP_CalibrateMeasureCommand();
		}
		rmp_env.flag.calibration = 0;
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
	}
	else
	{
		RETIRODE_RMP_SendErrorIndication(RETIRODE_RMP_STATE_IDLE, RETIRODE_RMP_ERR_UART_ERROR);
	}
}

static void RETIRODE_RMP_SettingsStateHandler(void)
{
	int32_t status = ARM_DRIVER_OK;
	int32_t resetStatus = ARM_DRIVER_OK;

	char reset[4] = { rmp_env.current_command.reg, '0','0','\r'};
	resetStatus = RETIRODE_RMP_WriteCommand(reset);

	if(rmp_env.current_command.low_high)
	{
		char command[4] = COMMAND_STRING;
		COMMAND(command, rmp_env.current_command.reg, rmp_env.current_command.value >> 8);
		status = RETIRODE_RMP_WriteCommand(command);

		char command2[4] = COMMAND_STRING;
		COMMAND(command2, rmp_env.current_command.reg2, rmp_env.current_command.value);
		status = RETIRODE_RMP_WriteCommand(command2);
	}
	else
	{
		char command[4] = COMMAND_STRING;
		COMMAND(command, rmp_env.current_command.reg, rmp_env.current_command.value);
		status = RETIRODE_RMP_WriteCommand(command);
	}

	if (resetStatus == ARM_DRIVER_OK && status == ARM_DRIVER_OK)
	{
		rmp_env.current_command.reg = 0;
		rmp_env.current_command.value = 0;
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
	}
	else
	{
		RETIRODE_RMP_SendErrorIndication(RETIRODE_RMP_STATE_IDLE, RETIRODE_RMP_ERR_UART_ERROR);
	}
}

static void RETIRODE_RMP_MeasureStateHandler(void)
{
	int32_t status = ARM_DRIVER_OK;
	int32_t resetStatus  = ARM_DRIVER_OK;
	int32_t receive_status = ARM_DRIVER_OK;

	rmp_env.mueasurement_response_waitter = 0;

	receive_status = RETIRODE_RMP_UARTReceiveMeasurementData(rmp_env.pulse_count);
	char command[4] = COMMAND_STRING;
	COMMAND(command, RETIRODE_RMP_PULSE_COUNT_REGISTER, rmp_env.pulse_count);

	resetStatus = RETIRODE_RMP_WriteCommand("N00\r");
	status = RETIRODE_RMP_WriteCommand(command);

	if (resetStatus == ARM_DRIVER_OK && status == ARM_DRIVER_OK && receive_status == ARM_DRIVER_OK)
	{
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_MEASURE_DATA_PROCESSING);
	}
	else
	{
		RETIRODE_RMP_SendErrorIndication(RETIRODE_RMP_STATE_IDLE, RETIRODE_RMP_ERR_UART_ERROR);
	}
}

static void RETIRODE_RMP_CalculateMeanFromRawData(void)
{
	uint32_t count = 1;
	uint32_t value = 0;
	char *str_end;

	value += strtol(rmp_env.current_measurement.received_data_buffer, &str_end, 16);
	str_end++;

	while(count < rmp_env.pulse_count)
	{
		value += strtol(str_end, NULL, 16);
		str_end += 4;
		count++;
	}

	rmp_env.current_measurement.requested_data_buffer[rmp_env.current_measurement.requested_data_proccesed] = (float)value / count;
}

static void RETIRODE_RMP_DataProcessingStateHandler(void)
{
	if (rmp_env.uart_event == ARM_USART_EVENT_RECEIVE_COMPLETE)
	{
		//calculate mean from every n received data
		RETIRODE_RMP_CalculateMeanFromRawData();

		//after successfully calculated mean from raw data
		rmp_env.current_measurement.requested_data_proccesed += 1;

		//if we need more data, measure another data
		if(rmp_env.current_measurement.requested_data_proccesed < rmp_env.flag.cmd_measure)
		{
			RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_MEASURE);
		}
		else
		{
			RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_MEASUREMENT_DATA_READY);
		}
	}
	else
	{
		rmp_env.mueasurement_response_waitter += 1;

		if(rmp_env.mueasurement_response_waitter >= MAX_MUEASURE_WAIT_COUNT)
		{
			RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
		}
	}
}

static void RETIRODE_RMP_MeasurementDataReadyStateHandler(void)
{
	static RETIRODE_RMP_Data_t measurement;

	if(rmp_env.current_measurement.calibration)
	{
		static RETIRODE_RMP_CalibrationDataReady_response_t calibration;
		calibration.cal =  rmp_env.current_measurement.calibration;
		memcpy(&calibration.value, rmp_env.current_measurement.requested_data_buffer, sizeof(uint32_t));
		RETIRODE_RMP_SendCalibrationDataReadyEvent(&calibration);
		rmp_env.current_measurement.calibration = 0;
	}
	else
	{
		memcpy(measurement.data, rmp_env.current_measurement.requested_data_buffer, rmp_env.current_measurement.requested_data_proccesed * sizeof(uint32_t));
		measurement.size = rmp_env.current_measurement.requested_data_proccesed;
		RETIRODE_RMP_SendDataReadyEvent(&measurement);
	}

	/**Reset values */
	/*Initialize internal structures */
	memset(&rmp_env.current_measurement, 0, sizeof(rmp_env.current_measurement));
	rmp_env.flag.cmd_measure = 0;

	RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
}


/** QUERY COMMAND */
static void RETIRODE_RMP_QueryStateHandler(void)
{
	int32_t status = ARM_DRIVER_OK;
	int32_t receive_status;

	rmp_env.query_response_waitter = 0;
	receive_status = RETIRODE_RMP_UARTReceiveQueryResponse();

	char command[4] = QUERY_STRING;
	QUERY(command, rmp_env.flag.cmd_query);
	status = RETIRODE_RMP_WriteCommand(command);

	if (status == ARM_DRIVER_OK && receive_status == ARM_DRIVER_OK)
	{
		rmp_env.flag.cmd_query = 0;
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_QUERY_RESPONSE);
	}
	else
	{
		RETIRODE_RMP_SendErrorIndication(RETIRODE_RMP_STATE_IDLE, RETIRODE_RMP_ERR_UART_ERROR);
	}
}

/** QUERY RESPONSE */
static void RETIRODE_RMP_QueryResponseStateHandler(void)
{
	if (rmp_env.uart_event == ARM_USART_EVENT_RECEIVE_COMPLETE)
	{
		static RETIRODE_RMP_Query_response_t data;
		data.value = 0;

		float floatResponse = 0;
		uint32_t bitResponse = 0;

		data.reg = rmp_env.query_response_buffer[0];
		uint8_t value = strtol(rmp_env.query_response_buffer + 1, NULL, 16);

		switch(data.reg)
		{
		 	case (RETIRODE_RMP_DCD_CONFIG_REGISTER):
			{
		 		// laser power enabled
		 		if ((value & 0x01) != 0)
		 		{
		 			bitResponse |=  1 << 1;
		 		}
		 		//laser overload
		 		if ((value & 0x04) != 0)
				{
		 			bitResponse |=  1 << 2;
				}
		 		//bias power enabled
		 		if ((value & 0x02) != 0)
				{
		 			bitResponse |=  1 << 3;
				}
		 		//bias  overload
		 		if ((value & 0x08) != 0)
				{
		 			bitResponse |=  1 << 4;
				}
		 		break;
			}
		 	case(RETIRODE_RMP_TARGET_LASER_VOLTAGE_REGISTER):
			{
		 		floatResponse = FromNativeLaserToReal(value);
				break;
			}
			case(RETIRODE_RMP_TARGET_BIAS_VOLTAGE_REGISTER):
			{
				floatResponse = FromNativeBiasToReal(value);
				break;
			}
		 	case(RETIRODE_RMP_ACTUAL_LASER_VOLTAGE_REGISTER):
			{
		 		floatResponse = FromNativeLaserToReal(value);
		 		break;
			}
		 	case(RETIRODE_RMP_ACTUAL_BIAS_VOLTAGE_REGISTER):
			{
 		 		floatResponse = FromNativeBiasToReal(value);
		 		break;
			}
		}

		if(bitResponse != 0)
		{
			memcpy(&data.value, &bitResponse, sizeof(uint32_t));
		}
		else
		{
			memcpy(&data.value, &floatResponse, sizeof(uint32_t));
		}

		RETIRODE_RMP_SendQueryResponseReadyEvent(&data);
	 	RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
	}
	else
	{
		rmp_env.query_response_waitter += 1;

		if(rmp_env.query_response_waitter >= MAX_QUERY_WAIT_COUNT)
		{
			RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
		}
	}
}



int32_t RETIRODE_RMP_Initialize(ARM_DRIVER_USART *uart, RETIRODE_RMP_EventHandler_t handler)
{

	if (uart == NULL || handler == NULL)
	{
		return ARM_DRIVER_ERROR;
	}

	/*Initialize internal structures */
	memset(&rmp_env, 0, sizeof(rmp_env));
	rmp_env.uart = uart;
	rmp_env.p_evt_handler = handler;

	return ARM_DRIVER_OK;
}

bool RETIRODE_RMP_MainLoop(void)
{

	RETIRODE_RMP_State_t entry_state = rmp_env.state;

	if(rmp_env.flag.cmd_software_reset == true)
	{
		RETIRODE_RMP_WriteCommand("R01\r");
		memset(&rmp_env.flag, 0, sizeof(rmp_env.flag));
		memset(&rmp_env.current_command, 0, sizeof(rmp_env.current_command));
		memset(&rmp_env.d_register_state, 0, sizeof(rmp_env.d_register_state));
		memset(&rmp_env.pulse_count, 0, sizeof(rmp_env.pulse_count));
		memset(&rmp_env.query_response_buffer, 0, sizeof(rmp_env.query_response_buffer));
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_SHUTDOWN);

		if(entry_state != RETIRODE_RMP_STATE_SHUTDOWN)
		{
			RETIRODE_RMP_PowerUpCommand();
		}

		rmp_env.flag.cmd_software_reset = false;
	}

	retirode_lmp_state_handler[rmp_env.state]();

 	if (rmp_env.state != entry_state)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void RETIRODE_RMP_QueryCommand(char reg)
{
	rmp_env.flag.cmd_query = reg;
}

void RETIRODE_RMP_SettingLowHighCommand(char reg, char reg2, uint16_t value)
{
	rmp_env.current_command.low_high = true;
	rmp_env.current_command.reg = reg;
	rmp_env.current_command.reg2 = reg2;
	rmp_env.current_command.value = value;
}

void RETIRODE_RMP_SettingCommand(char reg, uint8_t value)
{
	rmp_env.current_command.low_high = false;
	rmp_env.current_command.reg = reg;
	rmp_env.current_command.value = value;
}

void RETIRODE_RMP_MeasureCommand(uint32_t measure_size)
{
	if(rmp_env.pulse_count == 0)
	{
		//default value
		rmp_env.pulse_count = 100;
	}

	rmp_env.flag.cmd_measure = measure_size;
	rmp_env.current_measurement.calibration = 0;
}

static void RETIRODE_RMP_CalibrateMeasureCommand()
{
	RETIRODE_RMP_MeasureCommand(1);
	rmp_env.current_measurement.calibration = rmp_env.flag.calibration;
}


void RETIRODE_RMP_PowerUpCommand(void)
{
	rmp_env.flag.cmd_power_on = true;
}

void RETIRODE_RMP_SetPulseCountCommand(uint8_t count)
{
	rmp_env.pulse_count = count;
}

void RETIRODE_RMP_SoftwareResetCommand()
{
	rmp_env.flag.cmd_software_reset = true;
}

static void RETIRODE_RMP_UpdateDRegister(void)
{
	uint commandByte;
	commandByte = 0;

	if (rmp_env.d_register_state.laserPowerEnable)
		commandByte |= 0x01;

	if (rmp_env.d_register_state.biasPowerEnable)
		commandByte |= 0x02;

	RETIRODE_RMP_SettingCommand(RETIRODE_RMP_DCD_CONFIG_REGISTER, commandByte);
}

void RETIRODE_RMP_SetLaserPowerEnabledCommand(bool enabled)
{
	rmp_env.d_register_state.laserPowerEnable = enabled;
	RETIRODE_RMP_UpdateDRegister();
}

void RETIRODE_RMP_SetPowerBiasEnabledCommand(bool enabled)
{
	rmp_env.d_register_state.biasPowerEnable = enabled;
	RETIRODE_RMP_UpdateDRegister();
}

void RETIRODE_RMP_SetTriggerPeriodCommand(float newPeriod)
{
	uint16_t newNativePeriod;

	float a = TRIGGER_MIN_PERIOD;
	float b = TRIGGER_MAX_PERIOD;
	if (newPeriod >= a && newPeriod <= b)
	{
		// there seems to be some settable input
		newNativePeriod = roundf(newPeriod  / TRIGGER_GENERATOR_TICK);
		RETIRODE_RMP_SettingLowHighCommand(RETIRODE_RMP_TRIGGER_PERIOD_HIGH_BYTE_REGISTER, RETIRODE_RMP_TRIGGER_PERIOD_HIGH_LOW_REGISTER, newNativePeriod);
	}
}

void RETIRODE_RMP_CalibrateCommand(uint8_t calibrate)
{
	rmp_env.flag.calibration = calibrate;
}

static uint8_t FromRealBiasToNative(float realVoltage)
{
    float compVoltage;
    float adcRampTime;

    uint8_t nativeValue;

    // limit real value from bottom
    if (realVoltage < BIAS_MAX_MEAS_VOLTAGE)
        realVoltage = BIAS_MAX_MEAS_VOLTAGE;

    // limit real value from top
    if (realVoltage > BIAS_MIN_MEAS_VOLTAGE)
        realVoltage = BIAS_MIN_MEAS_VOLTAGE;

    // calculate ADC comparator voltage from real laser power supply output voltage
    compVoltage = ADC_SUPPLY_VOLTAGE - R34_UPPER * (ADC_SUPPLY_VOLTAGE - realVoltage - ADC_LOWEST_COMP_VOLTAGE) / (R32_LOWER + R34_UPPER);

    // calculate ADC ramp time from comparator voltage
    adcRampTime = - ADC_TAU_BIAS * log(-((compVoltage / ADC_SUPPLY_VOLTAGE)-1));

    // calculate ADC value from ADC ramp time
    nativeValue = round(ADC_OFFSET_BIAS + (ADC_FREQUENCY * adcRampTime));

    return nativeValue;
}

static float FromNativeBiasToReal(uint8_t nativeVoltage)
{
	float compVoltage;
	float adcRampTime;

	float realValue;

	// limit native value from bottom
	if (nativeVoltage < ADC_OFFSET_BIAS)
		nativeVoltage = ADC_OFFSET_BIAS;

	// calculate ADC ramp time from native value
	adcRampTime = (nativeVoltage - ADC_OFFSET_BIAS) / ADC_FREQUENCY;

	// calculate ADC comparator voltage from ADC ramp time
	compVoltage = ADC_SUPPLY_VOLTAGE * (1 - exp(-adcRampTime / ADC_TAU_BIAS));

	// calculate real voltage value from ADC comparator voltage
	realValue = - ADC_LOWEST_COMP_VOLTAGE - ADC_SUPPLY_VOLTAGE * (R32_LOWER / R34_UPPER) + compVoltage * (1 + (R32_LOWER / R34_UPPER));

	return realValue;
}


/** Convert LASER power voltage from real value to native representation in HW  */
static uint8_t FromRealLaserToNative(float realVoltage)
{
    float compVoltage;
    float adcRampTime;

    uint8_t nativeValue;

    // limit real value from bottom
    if (realVoltage < LASER_MIN_MEAS_VOLTAGE)
        realVoltage = LASER_MIN_MEAS_VOLTAGE;

    // limit real value from top
    if (realVoltage > LASER_MAX_MEAS_VOLTAGE)
        realVoltage = LASER_MAX_MEAS_VOLTAGE;

    // calculate ADC comparator voltage from real laser power supply outpu voltage
    compVoltage = realVoltage / (1 + (R22_UPPER / R27_LOWER));

    //calculate ADC ramp time from comparator voltage
    adcRampTime = - ADC_TAU_LASER * log (-((compVoltage / ADC_SUPPLY_VOLTAGE)-1));

    //calculate ADC value from ADC ramp time
    nativeValue = round(ADC_OFFSET_LASER + (ADC_FREQUENCY * adcRampTime));

    return nativeValue;
}

//* convert LASER power voltage from native representation in HW to real value  */
static float FromNativeLaserToReal(uint8_t nativeVoltage)
{
    float compVoltage;
    float adcRampTime;

    float realValue;

    // limit native value from bottom
    if (nativeVoltage < ADC_OFFSET_LASER)
        nativeVoltage = ADC_OFFSET_LASER;

    // calculate ADC ramp time from native value
    adcRampTime = (nativeVoltage - ADC_OFFSET_LASER) / ADC_FREQUENCY;

    // calculate ADC comparator voltage from ADC ramp time
    compVoltage = ADC_SUPPLY_VOLTAGE * (1 - exp(-adcRampTime / ADC_TAU_LASER));

    // calculate real voltage value from ADC comparator voltage
    realValue = compVoltage * (1 + (R22_UPPER / R27_LOWER));

    return realValue;
}

void RETIRODE_RMP_SetPowerBiasTargetVoltateCommand(uint32_t vol)
{
	float voltage = 0;
	memcpy(&voltage, &vol, sizeof(uint32_t));
	uint8_t targetVoltageNative;

	if (voltage < BIAS_MAX_TARGET_VOLTAGE)
		// required target voltage is below usable range
		voltage = BIAS_MAX_TARGET_VOLTAGE;

	if (voltage > BIAS_MIN_TARGET_VOLTAGE)
		// required target voltage is above usable range
		voltage = BIAS_MIN_TARGET_VOLTAGE;

	// convert real float voltage value to native HW value
	targetVoltageNative = FromRealBiasToNative(voltage);

	RETIRODE_RMP_SettingCommand(RETIRODE_RMP_TARGET_BIAS_VOLTAGE_REGISTER, targetVoltageNative);
}

void RETIRODE_RMP_SetLaserPowerTargetVoltateCommand(uint32_t vol)
{
	 float voltage = 0;
	 memcpy(&voltage, &vol, sizeof(uint32_t));
	 uint8_t targetVoltageNative;

	if (voltage < LASER_MIN_TARGET_VOLTAGE)
		// required target voltage is below usable range
		voltage = LASER_MIN_TARGET_VOLTAGE;

	if (voltage > LASER_MAX_TARGET_VOLTAGE)
		// required target voltage is above usable range
		voltage = LASER_MAX_TARGET_VOLTAGE;

	// convert real float voltage value to native HW value
	targetVoltageNative = FromRealLaserToNative(voltage);

	RETIRODE_RMP_SettingCommand(RETIRODE_RMP_TARGET_LASER_VOLTAGE_REGISTER, targetVoltageNative);
}

/** @} */
/** @} */

/* ----------------------------------------------------------------------------
 * End of File
 * ------------------------------------------------------------------------- */

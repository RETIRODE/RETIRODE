/*
 *retirode_lmp.c
 *
 *Created on: Apr 4, 2021
 *     Author: marti
 */
#include <retirode_rmp.h>
#include <rsl10.h>
#define QUERY_CHAR "%??\r"
#define ACK_CHAR "\r"
#define COMMAND(dest, reg, value) BuildCommand(dest, reg, value)
#define QUERY(dest, reg) ReplaceChar(dest, '%', reg)

static void BuildCommand(char *destination, const char *reg, const char value[]);
static char* ReplaceChar(char* str, char find, char replace);
/**
 *Array of internal flags about the state of library and which application
 *issued commands are pending.
 */
struct RETIRODE_RMP_EnvironmentFlags_t
{
	/**Command to start power up of the RMP was received. */
	bool cmd_power_on;

	/**Command to power off RMP was received. */
	bool cmd_power_off;

	/**Command to measure n samples*/
	uint32_t cmd_measure;

	/** Query register command */
	char cmd_query;

	/**Flag to indicate that RMP is not in shutdown mode. */
	bool is_powered;

	/**Flag to indicate that RMP is ready to start next UART transaction. */
	bool is_uart_ready;
};


struct RETIRODE_RMP_CurrentMeasurement_t
{
	/* Holds received RAW data from UART */
	char received_data_buffer[RETIRODE_RMP_DATA_RECEIVED_BUFFER_SIZE];

	uint32_t pulse_count;

	/** Holds already processed data */
	uint32_t requested_data_buffer[10];

	/** Hold information about current size of processed data */
	uint32_t requested_data_proccesed;
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

	/** Holds latest query response */
	char query_response_buffer[4];
	/**
	 *Application assigned event handler used to notify application on
	 *certain events.
	 */
	RETIRODE_RMP_EventHandler_t p_evt_handler;
};

/**@internal */
typedef void(*RETIRODE_RMP_StateHandler_t)(void);

static void RETIRODE_RMP_ShutdownStateHandler(void);
static void RETIRODE_RMP_PowerUpStateHandler(void);
static void RETIRODE_RMP_IdleStateHandler(void);
static void RETIRODE_RMP_SettingsStateHandler(void);
static void RETIRODE_RMP_QueryStateHandler(void);
static void RETIRODE_RMP_QueryResponseStateHandler(void);
static void RETIRODE_RMP_MeasureStateHandler(void);
static void RETIRODE_RMP_DataProcessingStateHandler(void);
static void RETIRODE_RMP_MeasurementDataReadyStateHandler(void);

static struct RETIRODE_RMP_Eviroment_t rmp_env;

static
const RETIRODE_RMP_StateHandler_t retirode_lmp_state_handler[RETIRODE_RMP_STATE_MAX] = {
[RETIRODE_RMP_STATE_SHUTDOWN] = RETIRODE_RMP_ShutdownStateHandler,
[RETIRODE_RMP_STATE_POWER_UP] = RETIRODE_RMP_PowerUpStateHandler,
[RETIRODE_RMP_STATE_IDLE] = RETIRODE_RMP_IdleStateHandler,
[RETIRODE_RMP_STATE_SETTING] = RETIRODE_RMP_SettingsStateHandler,
[RETIRODE_RMP_STATE_QUERY] = RETIRODE_RMP_QueryStateHandler,
[RETIRODE_RMP_STATE_QUERY_RESPONSE] = RETIRODE_RMP_QueryResponseStateHandler,
[RETIRODE_RMP_STATE_MEASURE] = RETIRODE_RMP_MeasureStateHandler,
[RETIRODE_RMP_STATE_MEASURE_DATA_PROCESSING] = RETIRODE_RMP_DataProcessingStateHandler,
[RETIRODE_RMP_STATE_MEASUREMENT_DATA_READY] = RETIRODE_RMP_MeasurementDataReadyStateHandler
};

/**
 *Function to build command/queries which will be send to LIDAR.
 *
 */
static void BuildCommand(char *destination, const char *reg, const char value[])
{
	//Register
	strncpy(destination, reg, 1);

	//value
	strncpy(destination + 1, value, 2);

	//ack char
	strncpy(destination + 3, ACK_CHAR, 1);
}

static char* ReplaceChar(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    *current_pos = replace;
    return str;
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
 *Issue a UART write command to LIDAR.
 *
 *These commands are used to issue commands to the LIDAR to either start a
 *specific operation or to query data.
 *
 *@param cmd
 *Command as defined in @ref SMARTSHOT_ISP_COMMANDS_GRP.
 *
 *@return
 *ARM_DRIVER_OK - On success. < br>
 *ARM_DRIVER_ERROR - On failure.
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

static int32_t RETIRODE_RMP_UARTReceiveMeasurementData(uint32_t len)
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

static void RETIRODE_RMP_SendQueryResponseReadyEvent(
		RETIRODE_RMP_Query_response_t *p_data)
{
    if (rmp_env.p_evt_handler != NULL)
    {
    	rmp_env.p_evt_handler(RETIRODE_RMP_EVENT_QUERY_RESPONSE_READY, p_data);
    }
}

static void RETIRODE_RMP_SendReadyEvent(
		RETIRODE_RMP_ReadyReason_t *p_data)
{
    if (rmp_env.p_evt_handler != NULL)
    {
    	rmp_env.p_evt_handler(RETIRODE_RMP_EVENT_READY, p_data);
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
		//TODO: Check if lmp is initialized
		rmp_env.flag.is_powered = true;
		rmp_env.flag.cmd_power_on = false;

		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_POWER_UP);
	}
}

static void RETIRODE_RMP_PowerUpStateHandler(void)
{
	int32_t status;
	if (rmp_env.flag.is_powered)
	{
		//TODO: Check if lmp is initialized

		char powerOn_cmd[4];
		COMMAND(powerOn_cmd, D_REGISTER, "03");
		status = RETIRODE_RMP_WriteCommand(powerOn_cmd);

		if (status == ARM_DRIVER_OK)
		{
			rmp_env.flag.cmd_power_on = false;
			rmp_env.flag.is_powered = true;

			RETIRODE_RMP_SendReadyEvent(RETIRODE_RMP_READY_POWER_UP);

			RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
		}
		else
		{
			RETIRODE_RMP_SendErrorIndication(RETIRODE_RMP_STATE_POWER_UP, RETIRODE_RMP_ERR_UART_ERROR);
		}
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
	else if(1 < 0)
	{
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_SETTING);
	}
	/**Measure data */
	else if (rmp_env.flag.cmd_measure)
	{
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_MEASURE);
	}
}

static void RETIRODE_RMP_SettingsStateHandler(void)
{
	int32_t status = ARM_DRIVER_OK;

	char query[4];
	QUERY(query, rmp_env.flag.cmd_query);
	status = RETIRODE_RMP_WriteCommand(query);

	if (status == ARM_DRIVER_OK)
	{
		rmp_env.flag.cmd_measure = 0;
		RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_MEASURE_DATA_PROCESSING);
	}
	else
	{
		RETIRODE_RMP_SendErrorIndication(RETIRODE_RMP_STATE_IDLE, RETIRODE_RMP_ERR_UART_ERROR);
	}
}

static void RETIRODE_RMP_MeasureStateHandler(void)
{
	int32_t status = ARM_DRIVER_OK;
	int32_t receive_status;
	RETIRODE_RMP_WriteCommand("C04\r");
	rmp_env.current_measurement.pulse_count = 100;
	receive_status = RETIRODE_RMP_UARTReceiveMeasurementData(100);
	status = RETIRODE_RMP_WriteCommand("N64\r");

	if (status == ARM_DRIVER_OK && receive_status == ARM_DRIVER_OK)
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

	while(count < rmp_env.current_measurement.pulse_count)
	{
		value += strtol(str_end, NULL, 16);
		str_end += 4;
		count++;
	}

	rmp_env.current_measurement.requested_data_buffer[rmp_env.current_measurement.requested_data_proccesed] = value / count;
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
}

static void RETIRODE_RMP_MeasurementDataReadyStateHandler(void)
{
	static RETIRODE_RMP_Data_t measurement;

	memcpy(measurement.data, rmp_env.current_measurement.received_data_buffer, rmp_env.current_measurement.requested_data_proccesed);
	RETIRODE_RMP_SendDataReadyEvent(&measurement);

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

	receive_status = RETIRODE_RMP_UARTReceiveQueryResponse();

	char command[4] = QUERY_CHAR;
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

		memcpy(&data.response, rmp_env.query_response_buffer, 4);
		RETIRODE_RMP_SendQueryResponseReadyEvent(&data);
	 	RETIRODE_RMP_SetState(RETIRODE_RMP_STATE_IDLE);
	}
}



int32_t RETIRODE_RMP_Initialize(ARM_DRIVER_USART *uart, RETIRODE_RMP_EventHandler_t handler)
{
	//REQUIRE(uart != NULL);
	//REQUIRE(handler != NULL);

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

void RETIRODE_RMP_SettingCommand(char reg, char char1, char char2)
{

}

void RETIRODE_RMP_MeasureCommand(uint32_t measure_size)
{
	rmp_env.flag.cmd_measure = measure_size;
}

void RETIRODE_RMP_PowerUpCommand(void)
{
	rmp_env.flag.cmd_power_on = true;
}

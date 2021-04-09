/*
 *retirode_lmp.c
 *
 *Created on: Apr 4, 2021
 *     Author: marti
 */
#include <retirode_lmp.h>
#include <rsl10.h>
#define QUERY_CHAR "??"
#define ACK_CHAR "\r"
#define COMMAND(dest, reg, value) BuildCommand(dest, reg, value)
#define QUERY(dest, reg) BuildCommand(dest, reg, QUERY_CHAR)

static void BuildCommand(char *destination, const char *reg, const char value[]);
/**
 *Array of internal flags about the state of library and which application
 *issued commands are pending.
 */
struct RETIRODE_LMP_EnvironmentFlags_t
{
	/**Command to start power up of the LMP was received. */
	bool cmd_power_on;

	/**Command to power off LMP was received. */
	bool cmd_power_off;

	/**Command to measure n samples*/
	uint32_t cmd_measure;

	/**Flag to indicate that LMP is not in shutdown mode. */
	bool is_powered;

	/**Flag to indicate that LMP is ready to start next UART transaction. */
	bool is_uart_ready;
};

/**
 *Holds all information about current state of LMP
 */
struct RETIRODE_LMP_Eviroment_t
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
	 *Current state of LMP operation.
	 *
	 *Used to call appropriate state handler in main loop.
	 */
	RETIRODE_LMP_State_t state;

	/**Flags about LMP state and application issued commands. */
	volatile struct RETIRODE_LMP_EnvironmentFlags_t flag;

	/* Hold received data from UART */
	char received_data_buffer[RETIRODE_LMP_DATA_RECEIVED_BUFFER_SIZE];
	/**
	 *Application assigned event handler used to notify application on
	 *certain events.
	 */
	RETIRODE_LMP_EventHandler_t p_evt_handler;
};

/**@internal */
typedef void(*RETIRODE_LMP_StateHandler_t)(void);

static void RETIRODE_LMP_ShutdownStateHandler(void);
static void RETIRODE_LMP_PowerUpStateHandler(void);
static void RETIRODE_LMP_IdleStateHandler(void);
static void RETIRODE_LMP_ConfigurationStateHandler(void);
static void RETIRODE_LMP_DataProcessingStateHandler(void);
static void RETIRODE_LMP_DataReadyStateHandler(void);

static struct RETIRODE_LMP_Eviroment_t lmp_env;

static
const RETIRODE_LMP_StateHandler_t retirode_lmp_state_handler[RETIRODE_LMP_STATE_MAX] = { [RETIRODE_LMP_STATE_SHUTDOWN] = RETIRODE_LMP_ShutdownStateHandler,
[RETIRODE_LMP_STATE_POWER_UP] = RETIRODE_LMP_PowerUpStateHandler,
[RETIRODE_LMP_STATE_IDLE] = RETIRODE_LMP_IdleStateHandler,
[RETIRODE_LMP_STATE_CONFIGURATION] = RETIRODE_LMP_ConfigurationStateHandler,
[RETIRODE_LMP_STATE_DATA_PROCESSING] = RETIRODE_LMP_DataProcessingStateHandler,
[RETIRODE_LMP_STATE_DATA_READY] = RETIRODE_LMP_DataReadyStateHandler
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

/**
 *Wrapper function for setting of new UART state.
 *
 *Allows to add state change debug messages here if necessary.
 */
static void RETIRODE_LMP_SetState(const RETIRODE_LMP_State_t new_state)
{
	lmp_env.state = new_state;
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
int32_t RETIRODE_LMP_WriteCommand(char *cmd)
{
	int32_t status;
	lmp_env.uart_event = 0;

	//wait for uart if busy
	while (lmp_env.uart->GetStatus().tx_busy);

	status = lmp_env.uart->Send(cmd, 4);

	if (status == ARM_DRIVER_OK)
	{
		//wait for uart
		while (lmp_env.uart->GetStatus().tx_busy);

		if (lmp_env.uart_event == ARM_USART_EVENT_SEND_COMPLETE)
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


static int32_t RETIRODE_LMP_UARTReceiveData(uint32_t len)
{
	while(lmp_env.uart->GetStatus().rx_busy);

	return lmp_env.uart->Receive(lmp_env.received_data_buffer,len);
}

void RETIRODE_LMP_UARTEventHandler(uint32_t event)
{
	lmp_env.uart_event = event;
}

static void RETIRODE_LMP_SendDataReadyEvent(
		RETIRODE_LMP_Data_t *p_data)
{
    if (lmp_env.p_evt_handler != NULL)
    {
    	lmp_env.p_evt_handler(RETIRODE_LMP_EVENT_MEASUREMENT_DATA_READY, p_data);
    }
}

static void RETIRODE_LMP_SendReadyEvent(
		RETIRODE_LMP_ReadyReason_t *p_data)
{
    if (lmp_env.p_evt_handler != NULL)
    {
    	lmp_env.p_evt_handler(RETIRODE_LMP_EVENT_READY, p_data);
    }
}

static void RETIRODE_LMP_SendErrorIndication(RETIRODE_LMP_State_t state,
	RETIRODE_LMP_Err_t error)
{
	RETIRODE_LMP_ErrorEvent_t err_event;

	err_event.state = state;
	err_event.error = error;

	if (lmp_env.p_evt_handler != NULL)
	{
		lmp_env.p_evt_handler(RETIRODE_LMP_EVENT_ERROR, &err_event);
	}
}

static void RETIRODE_LMP_ShutdownStateHandler(void)
{
	if (lmp_env.flag.cmd_power_on)
	{
		//TODO: Check if lmp is initialized
		lmp_env.flag.is_powered = true;
		lmp_env.flag.cmd_power_on = false;

		RETIRODE_LMP_SetState(RETIRODE_LMP_STATE_POWER_UP);
	}
}

static void RETIRODE_LMP_PowerUpStateHandler(void)
{
	int32_t status;
	if (lmp_env.flag.cmd_power_on)
	{
		//TODO: Check if lmp is initialized

		char powerOn_cmd[4];
		COMMAND(powerOn_cmd, D_REGISTER, "03");
		status = RETIRODE_LMP_WriteCommand(powerOn_cmd);

		if (status == ARM_DRIVER_OK)
		{
			lmp_env.flag.cmd_power_on = false;
			lmp_env.flag.is_powered = true;

			RETIRODE_LMP_SendReadyEvent(RETIRODE_LMP_READY_POWER_UP);

			RETIRODE_LMP_SetState(RETIRODE_LMP_STATE_IDLE);
		}
		else
		{
			RETIRODE_LMP_SendErrorIndication(RETIRODE_LMP_STATE_POWER_UP, RETIRODE_LMP_ERR_UART_ERROR);
		}
	}
}

static void RETIRODE_LMP_IdleStateHandler(void)
{
	int32_t status = ARM_DRIVER_OK;
	int32_t receive_status;

	if (lmp_env.flag.cmd_measure)
	{
		receive_status = RETIRODE_LMP_UARTReceiveData(lmp_env.flag.cmd_measure);

		status = RETIRODE_LMP_WriteCommand("R??\r");

		if (status == ARM_DRIVER_OK && receive_status == ARM_DRIVER_OK)
		{
			lmp_env.flag.cmd_measure = 0;
			RETIRODE_LMP_SetState(RETIRODE_LMP_STATE_DATA_PROCESSING);
		}
		else
		{
			RETIRODE_LMP_SendErrorIndication(RETIRODE_LMP_STATE_POWER_UP, RETIRODE_LMP_ERR_UART_ERROR);
		}
	}
}


static void RETIRODE_LMP_DataProcessingStateHandler(void)
{
	int32_t status = ARM_DRIVER_OK;
	int32_t receive_status;

	if (lmp_env.uart_event == ARM_USART_EVENT_RECEIVE_COMPLETE)
	{
		//TODO: calculate mean from every n received data

		RETIRODE_LMP_SetState(RETIRODE_LMP_STATE_DATA_READY);
	}
}

static void RETIRODE_LMP_ConfigurationStateHandler(void)
{
}


static void RETIRODE_LMP_DataReadyStateHandler(void)
{
	static RETIRODE_LMP_Data_t measurement;

	memcpy(measurement.data,lmp_env.received_data_buffer,RETIRODE_LMP_MAX_DATA_CHUNK_SIZE);
	RETIRODE_LMP_SendDataReadyEvent(&measurement);

	RETIRODE_LMP_SetState(RETIRODE_LMP_STATE_IDLE);
}

int32_t RETIRODE_LMP_Initialize(ARM_DRIVER_USART *uart, RETIRODE_LMP_EventHandler_t handler)
{
	//REQUIRE(uart != NULL);
	//REQUIRE(handler != NULL);

	if (uart == NULL || handler == NULL)
	{
		return ARM_DRIVER_ERROR;
	}

	/*Initialize internal structures */
	memset(&lmp_env, 0, sizeof(lmp_env));
	lmp_env.uart = uart;
	lmp_env.p_evt_handler = handler;

	return ARM_DRIVER_OK;
}

bool RETIRODE_LMP_MainLoop(void)
{
	//REQUIRE(isp_env.state < SMARTSHOT_ISP_STATE_MAX);

	if ((lmp_env.flag.cmd_power_off == true) &&
		(lmp_env.flag.is_powered == true))
	{
		RETIRODE_LMP_SetState(RETIRODE_LMP_STATE_SHUTDOWN);
	}

	RETIRODE_LMP_State_t entry_state = lmp_env.state;

	retirode_lmp_state_handler[lmp_env.state]();

 	if (lmp_env.state != entry_state)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void RETIRODE_LMP_MeasureCommand(uint32_t measure_size)
{
	lmp_env.flag.cmd_measure = measure_size;
}

void RETIRODE_LMP_PowerUpCommand(void)
{
	if (lmp_env.flag.is_powered == false)
	{
		lmp_env.flag.cmd_power_on = true;
	}
}

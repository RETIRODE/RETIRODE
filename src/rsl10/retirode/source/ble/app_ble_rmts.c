#include <app_ble_rmts_int.h>
#include <msg_handler.h>



static uint8_t RMTS_ControlPointWriteHandler(uint8_t conidx, uint16_t attidx,
        uint16_t handle, uint8_t *to, const uint8_t *from, uint16_t length,
        uint16_t operation);


static RMTS_Environment_t RMTS_env = { 0 };

const struct att_db_desc rmts_att_db[ATT_RMTS_COUNT] =
{
    /* Range measurement transfer service 0 */

    CS_SERVICE_UUID_128(
            ATT_RMTS_SERVICE_0, /* attidx */
            RMTS_SVC_UUID),     /* uuid */

    /* Range measuremen transfer service Control Point Characteristic */

    CS_CHAR_UUID_128(
            ATT_RMTS_CONTROL_POINT_CHAR_0,                         /* attidx_char */
            ATT_RMTS_CONTROL_POINT_VAL_0,                          /* attidx_val */
            RMTS_CHAR_CONTROL_POINT_UUID,                          /* uuid */
            PERM(WRITE_REQ, ENABLE) | PERM(WRITE_COMMAND, ENABLE), /* perm */
            sizeof(RMTS_env.att.cp.value),                         /* length */
            RMTS_env.att.cp.value,                                 /* data */
            RMTS_ControlPointWriteHandler),                        /* callback */

    CS_CHAR_USER_DESC(
            ATT_RMTS_CONTROL_POINT_DESC_0,              /* attidx */
            (sizeof(RMTS_CONTROL_POINT_USER_DESC) - 1), /* length */
            RMTS_CONTROL_POINT_USER_DESC,               /* data */
            NULL),                                      /* callback */

    /* Range Finder Info Characteristic */

    CS_CHAR_UUID_128(
            ATT_RMTS_INFO_CHAR_0, /* attidx_char */
            ATT_RMTS_INFO_VAL_0,  /* attidx_val */
            RMTS_CHAR_INFO_UUID,  /* uuid */
            PERM(NTF, ENABLE),    /* perm */
            0,                    /* length */
            NULL,                 /* data */
            NULL),                /* callback */

    CS_CHAR_CCC(
            ATT_RMTS_INFO_CCC_0,             /* attidx */
            RMTS_env.att.info.ccc,           /* data */
            NULL),                           /* callback */

    CS_CHAR_USER_DESC(
            ATT_RMTS_INFO_DESC_0,                  /* attidx */
            (sizeof(RMTS_TOFD_INFO_USER_DESC) - 1), /* length*/
			RMTS_TOFD_INFO_USER_DESC,               /* data */
            NULL),                                 /* callback */

    /*  Range Finder Measured Data Characteristic */

    CS_CHAR_UUID_128(ATT_RMTS_TOFD_CHAR_0, /* attidx_char */
            ATT_RMTS_TOFD_VAL_0,           /* attidx_val */
            RMTS_CHAR_TOFD_UUID,            /* uuid */
            PERM(NTF, ENABLE),                   /* perm */
            0,                                   /* length */
            NULL,                                /* data */
            NULL),                               /* callback */

    CS_CHAR_CCC(
            ATT_RMTS_TOFD_CCC_0,       /* attidx */
            RMTS_env.att.TOFD.ccc,       /* data */
            NULL),                           /* callback */

    CS_CHAR_USER_DESC(
            ATT_RMTS_TOFD_DESC_0,            /* attidx */
            (sizeof(RMTS_TOFD_USER_DESC) - 1), /* length*/
            RMTS_TOFD_USER_DESC,               /* data */
            NULL),                                 /* callback */
};

static uint32_t RMTS_GetMaxDataOctets(void)
{
    uint32_t max_data_octets = RMTS_env.max_tx_octets
                               - RMTS_NTF_PDU_HEADER_LENGTH
                               - RMTS_NTF_L2CAP_HEADER_LENGTH
                               - RMTS_NTF_ATT_HEADER_LENGTH;

    return max_data_octets;
}

static void RMTS_MsgHandler(ke_msg_id_t const msg_id, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    switch (msg_id)
    {
        case GATTC_CMP_EVT:
        {
            const struct gattc_cmp_evt *p = param;

            if (p->operation == GATTC_NOTIFY)
            {
                uint16_t attidx = RMTS_env.att.attidx_offset
                                  + ATT_RMTS_TOFD_VAL_0;

                /* If the sequence number is the number of MR Data Value
                 * attribute reduce number of pending packets. */
                if (p->seq_num == attidx)
                {
                    RMTS_env.transfer.packets_pending -= 1;

                    if (RMTS_env.transfer.bytes_queued
                        < RMTS_env.transfer.bytes_total)
                    {
                        /* Not all data is queued yet.
                         * Notify application that RMTS is ready to accept mr
                         * data for next packet. */
                        RMTS_env.att.cp.callback(
                                RMTS_OP_DATA_SPACE_AVAIL_IND,
                                NULL);
                    }
                    else
                    {
                        /* Last data notification was transferred. */
                        if (RMTS_env.transfer.packets_pending == 0)
                        {
                        	  RMTS_env.transfer.state = RMTS_STATE_START_REQUEST;
                        	  RMTS_env.att.cp.callback(RMTS_OP_DATA_TRANSFER_COMPLETED,
                        	                                  NULL);

                        }
                    }
                }
            }

            break;
        }

        case GATTC_MTU_CHANGED_IND:
        {
            if (RMTS_env.transfer.state >= RMTS_STATE_TOFD_INFO_PROVIDED)
            {
                /* Update of MTU not allowed while mrd transfer is in
                 * progress.
                 *
                 * 0x16 - CONNECTION TERMINATED BY LOCAL HOST
                 */
                GAPC_DisconnectAll(0x16);
            }

            break;
        }

        case GAPC_CONNECTION_REQ_IND:
        {


            RMTS_env.transfer.state = RMTS_STATE_CONNECTED;

            /* Reset connection related variables. */
            RMTS_env.max_tx_octets = RMTS_MIN_TX_OCTETS;

            break;
        }

        case GAPC_DISCONNECT_IND:
        {


            /* Send abort indication if disconnected during mrd transfer. */
            if (RMTS_env.transfer.state >= RMTS_STATE_TOFD_INFO_PROVIDED)
            {
                RMTS_env.att.cp.callback(RMTS_OP_CANCEL_REQ, NULL);
            }

            RMTS_env.transfer.state = RMTS_STATE_IDLE;
            break;
        }

        case GAPC_LE_PKT_SIZE_IND:
        {

            if (RMTS_env.transfer.state < RMTS_STATE_TOFD_INFO_PROVIDED)
            {
                /* Allow to update DLE parameters when there is no mr data
                 * transfer.
                 */

                const struct gapc_le_pkt_size_ind *p = param;

               RMTS_env.max_tx_octets = p->max_tx_octets;
                //PRINTF("RMTS : Set max_tx_octets=%d\r\n", RMTS_env.max_tx_octets);
            }
            else
            {
                /* Update of DLE parameters not allowed while mrd transfer is
                 * in progress.
                 *
                 * 0x16 - CONNECTION TERMINATED BY LOCAL HOST
                 */
                GAPC_DisconnectAll(0x16);
            }
            break;
        }

        default:
            break;
    }
}


/**
 * Function is called by Control point Handler when Start Command arrives,
 * then ensures that peer is connected and have enabled NTF.
 * Forward request to application handler
 */
static uint8_t RMTS_Start_Data_Measurement_Process()
{
    uint8_t err = -1;

    if (RMTS_env.transfer.state == RMTS_STATE_CONNECTED)
    {
        if (RMTS_env.att.info.ccc[0] == ATT_CCC_START_NTF)
        {
            RMTS_env.transfer.state = RMTS_STATE_START_REQUEST;
            RMTS_env.att.cp.callback(RMTS_OP_START_REQ, NULL);
        }
        else
        {
            err = RMTS_ATT_ERR_NTF_DISABLED;
        }
    }
    else
    {
        err = RMTS_ATT_ERR_PROC_IN_PROGRESS;
    }

    return err;
}

/**
 * Function is called by Control point Handler when Start Transfer Command arrives,
 * then ensures that data info was provided
 * Forward request to application handler
 */
static uint8_t RMTS_Start_Measured_Data_Transfer(uint8_t packet_count)
{
    uint8_t err = 0;

    if (RMTS_env.transfer.state >= RMTS_STATE_TOFD_INFO_PROVIDED)
    {
    	RMTS_env.transfer.state = RMTS_STATE_TOFD_TRANSMISSION;
        RMTS_env.att.cp.callback(RMTS_OP_DATA_TRANSFER_REQ, NULL);
    }
    else
    {
        err = RMTS_ATT_ERR_TOFD_TRANSFER_DISALLOWED;
    }

    return err;
}

/**
 * Function is called by Control point Handler when Stop Command arrives,
 * then ensures that transfer has been started
 * Forward request to handler in application
 *
 */
static uint8_t RMTS_Abort_Data_Measurement_Process(void)
{
    uint8_t status = 0;

    /* Cancel if operation is really ongoing.
     *
     * Silently ignore if there is nothing to cancel.
     */
    if (RMTS_env.transfer.state >= RMTS_STATE_START_REQUEST)
    {
    	RMTS_Abort_TOFD_Transfer(RMTS_INFO_ERR_ABORTED_BY_CLIENT);

        /* Notify application that capture is aborted. */
        RMTS_env.att.cp.callback(RMTS_OP_CANCEL_REQ, NULL);
    }

    return status;
}

static uint8_t RMTS_ControlPointWriteHandler(uint8_t conidx, uint16_t attidx,
        uint16_t handle, uint8_t *to, const uint8_t *from, uint16_t length,
        uint16_t operation)
{
    if (length > RMTS_CONTROL_POINT_VALUE_LENGTH || length == 0)
    {
        return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
    }

    uint8_t status = ATT_ERR_NO_ERROR;

    switch (from[0])
    {
        case RMTS_CONTROL_POINT_OPCODE_START_REQ:
        {
            if (length == 1)
            {
                status = RMTS_Start_Data_Measurement_Process();
            }
            else
            {
                status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
            }
            break;
        }

        case RMTS_CONTROL_POINT_OPCODE_CANCEL_REQ:
        {
            if (length == 1)
            {
                status = RMTS_Abort_Data_Measurement_Process();
            }
            else
            {
                status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
            }
            break;
        }

        case RMTS_CONTROL_POINT_OPCODE_TOFD_TRANSFER_REQ:
        {
            if (length == 1)
            {
                status = RMTS_Start_Measured_Data_Transfer(from[1]);
            }
            else
            {
                status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
            }
            break;
        }

        default:
        {
            status = ATT_ERR_REQUEST_NOT_SUPPORTED;
        }
    }

    return status;
}


static void RMTS_TransmitMRDataNotification(void)
{
	uint16_t attidx = RMTS_env.att.attidx_offset + ATT_RMTS_TOFD_VAL_0;
    uint16_t att_handle = GATTM_GetHandle(attidx);

    GATTC_SendEvtCmd(0, GATTC_NOTIFY, attidx, att_handle,
            RMTS_env.att.TOFD.value_length,
            RMTS_env.att.TOFD.value);

    RMTS_env.att.TOFD.value_length = 0;

   RMTS_env.transfer.packets_pending += 1;
}

int32_t RMTS_Initialize(RMTS_ControlHandler control_event_handler)
{
    int32_t status;

    if (control_event_handler == NULL)
    {
        return RMTS_ERR;
    }

    RMTS_env.att.attidx_offset = 0;
    RMTS_env.att.cp.callback = control_event_handler;
    RMTS_env.att.info.ccc[0] = 0x00;
    RMTS_env.att.info.ccc[1] = 0x00;
    RMTS_env.att.TOFD.ccc[0] = 0x00;
    RMTS_env.att.TOFD.ccc[1] = 0x00;
    RMTS_env.att.TOFD.value_length = 0;

    RMTS_env.transfer.state = RMTS_STATE_IDLE;
    RMTS_env.transfer.bytes_total = 0;

    RMTS_env.max_tx_octets = RMTS_MIN_TX_OCTETS;

    /* Add custom attributes into the attribute database. */
    status = APP_BLE_PeripheralServerAddCustomService(rmts_att_db, ATT_RMTS_COUNT,
            &RMTS_env.att.attidx_offset);
    if (status != 0)
    {
        return RMTS_ERR_INSUFFICIENT_ATT_DB_SIZE;
    }

    /* Listen for specific BLE kernel messages. */
    MsgHandler_Add(GATTC_CMP_EVT, RMTS_MsgHandler);
    MsgHandler_Add(GAPC_CONNECTION_REQ_IND, RMTS_MsgHandler);
    MsgHandler_Add(GAPC_DISCONNECT_IND, RMTS_MsgHandler);
    MsgHandler_Add(GATTC_MTU_CHANGED_IND, RMTS_MsgHandler);
    MsgHandler_Add(GAPC_LE_PKT_SIZE_IND, RMTS_MsgHandler);

    return RMTS_OK;
}

int32_t RMTS_Start_TOFD_Transfer(uint32_t TOFD_size)
{
    int32_t status = RMTS_OK;

    if (TOFD_size > 0)
    {
        if (RMTS_env.transfer.state == RMTS_STATE_START_REQUEST)
        {
            uint16_t attidx = RMTS_env.att.attidx_offset + ATT_RMTS_INFO_VAL_0;
            uint16_t att_handle = GATTM_GetHandle(attidx);
            uint8_t data[RMTS_INFO_TOFD_CAPTURED_LENGTH];

            data[0] = RMTS_INFO_OPCODE_TOFD_CAPTURED_IND;
            memcpy(data + 1, &TOFD_size, sizeof(TOFD_size));

            GATTC_SendEvtCmd(0, GATTC_NOTIFY, attidx, att_handle,
                    RMTS_INFO_TOFD_CAPTURED_LENGTH, data);

            /* Switch to next state to allow data transfers. */
            RMTS_env.transfer.state = RMTS_STATE_TOFD_INFO_PROVIDED;
            RMTS_env.transfer.bytes_total = TOFD_size;
            RMTS_env.transfer.bytes_queued = 0;
            RMTS_env.transfer.packets_pending = 0;
            RMTS_env.att.TOFD.value_length = 0;
        }
        else
        {
            status = RMTS_ERR_NOT_PERMITTED;
        }
    }
    else
    {
        status = RMTS_ERR;
    }

    return status;
}

int32_t RMTS_Abort_TOFD_Transfer(RMTS_InfoErrorCode_t errcode)
{
    int32_t status = RMTS_OK;

    if (RMTS_env.transfer.state >= RMTS_STATE_START_REQUEST)
    {
        uint16_t attidx = RMTS_env.att.attidx_offset + ATT_RMTS_INFO_VAL_0;
        uint16_t att_handle = GATTM_GetHandle(attidx);
        uint8_t data[2];

        data[0] = RMTS_INFO_OPCODE_ERROR_IND;
        data[1] = errcode;

        GATTC_SendEvtCmd(0, GATTC_NOTIFY, attidx, att_handle, 2, data);

        RMTS_env.transfer.state = RMTS_STATE_CONNECTED;
    }
    else
    {
        status = RMTS_ERR_NOT_PERMITTED;
    }

    return status;
}

int32_t RMTS_GetMax_TOFD_PushSize(void)
{
    int32_t avail_bytes;

    if (RMTS_env.transfer.state != RMTS_STATE_TOFD_TRANSMISSION)
    {
        avail_bytes = 0;
    }
    else if (RMTS_env.transfer.packets_pending == RMTS_MAX_PENDING_PACKET_COUNT)
    {
        avail_bytes = 0;
    }
    else
    {
        /* Determine:
         * (A) Number of packets that can be queued.
         * (B) Number of data  that can fit into single packet.
         * (C) Amount of data that is already queued for transmission in next
         *     packet.
         *
         * avail = (A * B) - C
         */
        avail_bytes = ((RMTS_MAX_PENDING_PACKET_COUNT
                       - RMTS_env.transfer.packets_pending)
                      * (RMTS_GetMaxDataOctets() - RMTS_INFO_OFFSET_LENGTH))
                      - (RMTS_env.att.TOFD.value_length
                         - RMTS_INFO_OFFSET_LENGTH);
    }

    return avail_bytes;
}

int32_t RMTS_TOFD_Push(const uint8_t* p_TOFD, const int32_t data_len)
{
    if ((p_TOFD == NULL)
        || (data_len <= 0)
        || (data_len > RMTS_TOFD_MAX_SIZE))
    {
        return RMTS_ERR;
    }

    if (RMTS_env.transfer.state != RMTS_STATE_TOFD_TRANSMISSION)
    {
       return RMTS_ERR_NOT_PERMITTED;
    }

    /* Safe to retype.
     * Pointer is is used as read-only iterator over the data array.
     */
    uint8_t* p_data = (uint8_t*) p_TOFD;
    int32_t bytes_left = data_len;

    while (bytes_left > 0)
    {
        /* Start to fill out new packet if value buffer is clear.
         *
         * Populate notification with first 4 bytes that contain data offset
         * from start of file.
         */
        if (RMTS_env.att.TOFD.value_length == 0)
        {
            memcpy(RMTS_env.att.TOFD.value, &RMTS_env.transfer.bytes_queued,
                    RMTS_INFO_OFFSET_LENGTH);
            RMTS_env.att.TOFD.value_length += RMTS_INFO_OFFSET_LENGTH;
        }

        const uint32_t max_data_octets = RMTS_GetMaxDataOctets();
        const uint32_t avail_space = max_data_octets
                                     - RMTS_env.att.TOFD.value_length;

        if (avail_space >= bytes_left)
        {
            /* All pending data can be fit into single packet. */
            memcpy(RMTS_env.att.TOFD.value + RMTS_env.att.TOFD.value_length,
                    p_data, bytes_left);

            RMTS_env.att.TOFD.value_length += bytes_left;
            RMTS_env.transfer.bytes_queued += bytes_left;
            p_data += bytes_left;
            bytes_left = 0;

            /* Transmit packet if (avail_space == data_len) */
            if (RMTS_env.att.TOFD.value_length >= max_data_octets)
            {
                RMTS_TransmitMRDataNotification();
            }
        }
        else
        {
            /* Pending data must be split into multiple packets.
             *
             * Fill as much data as possible into currently open packet.
             */
            memcpy(RMTS_env.att.TOFD.value + RMTS_env.att.TOFD.value_length,
                    p_data, avail_space);

            RMTS_env.att.TOFD.value_length += avail_space;
            RMTS_env.transfer.bytes_queued += avail_space;
            p_data += avail_space;
            bytes_left -= avail_space;

            RMTS_TransmitMRDataNotification();
        }

        /* Check if EOF was reached. */
        if (RMTS_env.transfer.bytes_queued >= RMTS_env.transfer.bytes_total)
        {
            /* Transmit any remaining image data. */
            if (RMTS_env.att.TOFD.value_length > RMTS_INFO_OFFSET_LENGTH)
            {
            	RMTS_TransmitMRDataNotification();
            }
        }
    }

    return RMTS_OK;
}

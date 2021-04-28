//-----------------------------------------------------------------------------
// Copyright (c) 2019 Semiconductor Components Industries LLC
// (d/b/a "ON Semiconductor").  All rights reserved.
// This software and/or documentation is licensed by ON Semiconductor under
// limited terms and conditions.  The terms and conditions pertaining to the
// software and/or documentation are available at
// http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf ("ON Semiconductor Standard
// Terms and Conditions of Sale, Section 8 Software") and if applicable the
// software license agreement.  Do not use this software and/or documentation
// unless you have carefully read and you agree to the limited terms and
// conditions.  By using this software and/or documentation, you agree to the
// limited terms and conditions.
//-----------------------------------------------------------------------------

#include <app_ble_peripheral_server.h>
#include <app_ble_rmts.h>



static const struct ke_msg_handler appm_default_state[] =
{
    { KE_MSG_DEFAULT_HANDLER, (ke_msg_func_t)MsgHandler_Notify }
};

/* Use the state and event handler definition for all states. */
static const struct ke_state_handler appm_default_handler
    = KE_STATE_HANDLER(appm_default_state);

/* Defines a place holder for all task instance's state */
ke_state_t appm_state[APP_IDX_MAX];

static const struct ke_task_desc TASK_DESC_APP = {
    NULL,       &appm_default_handler,
    appm_state, 0,
    APP_IDX_MAX
};

static const struct gapm_set_dev_config_cmd devConfigCmd =
{
    .operation = GAPM_SET_DEV_CONFIG,
    .role = GAP_ROLE_PERIPHERAL,
    .renew_dur = GAPM_DEFAULT_RENEW_DUR,
    .addr.addr = APP_BD_ADDRESS,
    .irk.key = APP_IRK,
    .addr_type = APP_BD_ADDRESS_TYPE,
#ifdef SECURE_CONNECTION
    .pairing_mode = (GAPM_PAIRING_SEC_CON | GAPM_PAIRING_LEGACY),
#else
    .pairing_mode = GAPM_PAIRING_LEGACY,
#endif
    .gap_start_hdl = GAPM_DEFAULT_GAP_START_HDL,
    .gatt_start_hdl = GAPM_DEFAULT_GATT_START_HDL,
    .att_and_ext_cfg = GAPM_DEFAULT_ATT_CFG,
    .sugg_max_tx_octets = GAPM_DEFAULT_TX_OCT_MAX,
    .sugg_max_tx_time = GAPM_DEFAULT_TX_TIME_MAX,
    .max_mtu = GAPM_DEFAULT_MTU_MAX,
    .max_mps = GAPM_DEFAULT_MPS_MAX,
    .max_nb_lecb = GAPM_DEFAULT_MAX_NB_LECB,
    .audio_cfg = GAPM_DEFAULT_AUDIO_CFG,
    .tx_pref_rates = GAP_RATE_LE_2MBPS,
    .rx_pref_rates = GAP_RATE_LE_2MBPS
};

static struct gapm_start_advertise_cmd advertiseCmd =
{
    .op = {
        .code = GAPM_ADV_UNDIRECT,
        .addr_src = GAPM_STATIC_ADDR,
        .state = 0
    },
    .intv_min = APP_ADV_INT,
    .intv_max = APP_ADV_INT,
    .channel_map = GAPM_DEFAULT_ADV_CHMAP,
    .info.host = {
        .mode = GAP_GEN_DISCOVERABLE,
        .adv_filt_policy = ADV_ALLOW_SCAN_ANY_CON_ANY
        /* ADV_DATA and SCAN_RSP data are set in APP_BLE_Initialize() */
    }
};

struct gap_dev_name_buff
{
    /// name length
    uint16_t length;
    /// name value
    uint8_t value[APP_DEVICE_NAME_LEN];
};

const struct gap_dev_name_buff getDevInfoCfmName =
{
    .length = APP_DEVICE_NAME_LEN,
    .value  = APP_DEVICE_NAME
};

const union gapc_dev_info_val getDevInfoCfmAppearance =
{
    .appearance = APP_DEVICE_APPEARANCE
};

const union gapc_dev_info_val getDevInfoCfmSlvParams =
{
    .slv_params = {APP_PREF_SLV_MIN_CON_INTERVAL,
                   APP_PREF_SLV_MAX_CON_INTERVAL,
                   APP_PREF_SLV_LATENCY,
                   APP_PREF_SLV_SUP_TIMEOUT}
};

const union gapc_dev_info_val* getDevInfoCfm[] =
{
    [GAPC_DEV_NAME] = (const union gapc_dev_info_val*) &getDevInfoCfmName,
    [GAPC_DEV_APPEARANCE] = &getDevInfoCfmAppearance,
    [GAPC_DEV_SLV_PREF_PARAMS] = &getDevInfoCfmSlvParams
};

static APP_BLE_AttDb_t cs_att_env;

static uint8_t registered_kernel_msg_id_count = 0;

/**
 * Set static data to be used as payload of Advertising Data packets and Scan
 * Response packets.
 *
 * Advertising Data:
 *
 * * Flags - Set by BLE stack
 * * Incomplete list of 128-bit Service UUIDs
 *   * Picture Transfer Service UUID
 * * Transmit Power
 * * Advertising Interval
 *
 * Scan Response Data:
 *
 * * Complete Device Name
 * * Developer Specific Data
 *
 * The RSL10 SmartShot mobile applications for Android and iOS filter devices
 * based on advertising and scan response data.
 * At least one of the following conditions must be met for the device to appear
 * in these apps:
 *
 * * AD or SR data contain Complete Device Name with name set to
 *   'smartshot_demo_cam'.
 * * AD contains Incomplete List of 128-bit Services with UUID of Picture
 *   Transfer Service.
 */
static void APP_BLE_SetAdvScanData(void)
{
    bool field_added = false;

    /* Set Advertising Data. */
    advertiseCmd.info.host.adv_data_len = 0;

    /* Add Incomplete list of 128-bit UUID Services with PTS service UUID. */
    uint8_t pts_uuid[16] = RMTS_SVC_UUID;
    field_added = GAPM_AddAdvData(GAP_AD_TYPE_MORE_128_BIT_UUID, pts_uuid, 16,
        advertiseCmd.info.host.adv_data, &advertiseCmd.info.host.adv_data_len);
    ////ASSERT(field_added == true);

    /* Add Transmit Power */
    uint8_t tx_power = OUTPUT_POWER_DBM;
    field_added = GAPM_AddAdvData(GAP_AD_TYPE_TRANSMIT_POWER, &tx_power, 1,
        advertiseCmd.info.host.adv_data, &advertiseCmd.info.host.adv_data_len);
    ////ASSERT(field_added == true);

    /* Add Advertising Interval */
    uint8_t adv_int[2];
    memcpy(adv_int, &advertiseCmd.intv_min, 2);
    field_added = GAPM_AddAdvData(GAP_AD_TYPE_ADV_INTV, adv_int, 2,
        advertiseCmd.info.host.adv_data, &advertiseCmd.info.host.adv_data_len);
    //ASSERT(field_added == true);

    /* FLAGS AD field is added automatically by BLE stack. */
    /* Ensure there is enough space left in AD for stack added Flags field. */
    //ASSERT(advertiseCmd.info.host.adv_data_len <= (GAP_ADV_DATA_LEN - 3));

    /* Set Scan Response data */
    advertiseCmd.info.host.scan_rsp_data_len = 0;

    /* Add Device Name */
    uint8_t devName[] = APP_DEVICE_NAME;
    field_added = GAPM_AddAdvData(GAP_AD_TYPE_COMPLETE_NAME, devName,
        APP_DEVICE_NAME_LEN, advertiseCmd.info.host.scan_rsp_data,
        &advertiseCmd.info.host.scan_rsp_data_len);
    //ASSERT(field_added == true);

    /* Add Developer Specific Data - No data just indicate ON Semiconductor as
     * manufacturer.
     */
    uint8_t manufacturer_id[2] = { 0x62, 0x03 };
    field_added = GAPM_AddAdvData(GAP_AD_TYPE_MANU_SPECIFIC_DATA,
        manufacturer_id, 2, advertiseCmd.info.host.scan_rsp_data,
        &advertiseCmd.info.host.scan_rsp_data_len);
    //ASSERT(field_added == true);
}

static void APP_BLE_SetConnectionCfmParams(uint8_t conidx,
        struct gapc_connection_cfm *cfm)
{
    cfm->svc_changed_ind_enable = 0;
    cfm->ltk_present = false;
#ifdef SECURE_CONNECTION
    cfm->pairing_lvl = GAP_AUTH_REQ_SEC_CON_BOND;
#else
    cfm->auth = GAP_AUTH_REQ_NO_MITM_BOND;
#endif

#if CFG_BOND_LIST_IN_NVR2
    if (GAPC_IsBonded(conidx))
    {
        cfm->ltk_present = true;
        Device_Param_Read(PARAM_ID_CSRK, cfm->lcsrk.key);
        memcpy(cfm->rcsrk.key, GAPC_GetBondInfo(conidx)->CSRK, KEY_LEN);
        cfm->lsign_counter = 0xFFFFFFFF;
        cfm->rsign_counter = 0;
    }
#endif

    //PRINTF("  connectionCfm->ltk_present = %d \n\r", cfm->ltk_present);
}

/* ----------------------------------------------------------------------------
 * Function      : void prvBLE_GAPM_GATTM_Handler(ke_msg_id_t const msg_id,
 *                                     void const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle GAPM/GATTM messages that need application action
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
static void APP_BLE_GAPM_GATTM_Handler(ke_msg_id_t const msg_id,
        void const *param, ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    switch (msg_id)
    {
        case GAPM_CMP_EVT:
        {
            const struct gapm_cmp_evt *p = param;

            /* Reset completed. Apply device configuration. */
            if (p->operation == GAPM_RESET)
            {
                //PRINTF("GAPM_RESET status=0x%x\r\n", p->status);
                GAPM_SetDevConfigCmd(&devConfigCmd);
            }
            /* Device configured. Populate custom attribute database. */
            else if (p->operation == GAPM_SET_DEV_CONFIG
                     && p->status == GAP_ERR_NO_ERROR)
            {
                //PRINTF("GAPM_SET_DEV_CONFIG status=0x%x\r\n", p->status);

                if (cs_att_env.cs_service_count > 0)
                {
                    GATTM_AddAttributeDatabase(cs_att_env.cs_att_db, cs_att_env.cs_att_count);
                }
                else
                {
                    //PRINTF("GAPM_SET_DEV_CONFIG starting advertising\r\n");
                    GAPM_StartAdvertiseCmd(&advertiseCmd); /* Start advertising */
                }
            }
            else if ((p->operation == GAPM_RESOLV_ADDR) && /* IRK not found for address */
            (p->status == GAP_ERR_NOT_FOUND))
            {
                struct gapc_connection_cfm cfm;
                uint8_t conidx = KE_IDX_GET(dest_id);
                APP_BLE_SetConnectionCfmParams(conidx, &cfm);
                GAPC_ConnectionCfm(conidx, &cfm); /* Confirm connection without LTK. */


            }
            else
            {
                //PRINTF("GAPM_CMP_EVT operation=0x%x status=%d\r\n", p->operation, p->status);
            }
        }
            break;

        case GAPM_ADDR_SOLVED_IND: /* Private address resolution was successful */
        {
            //PRINTF("GAPM_ADDR_SOLVED_IND\r\n");

            struct gapc_connection_cfm cfm;
            uint8_t conidx = KE_IDX_GET(dest_id);
            APP_BLE_SetConnectionCfmParams(conidx, &cfm);
            GAPC_ConnectionCfm(conidx, &cfm); /* Send connection confirmation with LTK */
        }
            break;

        case GATTM_ADD_SVC_RSP:
        {


            /* Start advertising if all custom services were added to attribute database. */
            if (GATTM_GetServiceAddedCount() == cs_att_env.cs_service_count)
            {
                //PRINTF("GATTM_ADD_SVC_RSP starting advertising\r\n");
                GAPM_StartAdvertiseCmd(&advertiseCmd); /* Start advertising */
            }
        }
            break;
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void APP_GAPC_MsgHandler(ke_msg_id_t const msg_id,
 *                                     void const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle GAPC messages that need application action
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
static void APP_BLE_GAPC_Handler(ke_msg_id_t const msg_id,
        void const *param, ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);

    switch (msg_id)
    {
        case GAPC_CONNECTION_REQ_IND:
        {
            const struct gapc_connection_req_ind *p = param;


#if CFG_BOND_LIST_IN_NVR2
            if (GAP_IsAddrPrivateResolvable(p->peer_addr.addr,
                        p->peer_addr_type)
                && BondList_Size() > 0)
            {

                GAPM_ResolvAddrCmd(conidx, p->peer_addr.addr, 0, NULL);
            }
            else
#endif
            {
                struct gapc_connection_cfm cfm;
                APP_BLE_SetConnectionCfmParams(conidx, &cfm);
                GAPC_ConnectionCfm(conidx, &cfm); /* Send connection confirmation */
            }
        }
            break;

        case GAPC_DISCONNECT_IND:
        {


            if (GAPC_GetConnectionCount() == 0)
            {
                //PRINTF("GAPC_DISCONNECT_IND starting advertising\r\n");
                GAPM_StartAdvertiseCmd(&advertiseCmd); /* Start advertising */
            }
        }
            break;

        case GAPC_GET_DEV_INFO_REQ_IND:
        {
            const struct gapc_get_dev_info_req_ind *p = param;
            GAPC_GetDevInfoCfm(conidx, p->req, getDevInfoCfm[p->req]);
            //PRINTF("GAPC_GET_DEV_INFO_REQ_IND: req = %d\r\n", p->req);
        }
            break;

        case GAPC_PARAM_UPDATE_REQ_IND:
        {
            const struct gapc_param_update_req_ind *p = param;
            GAPC_ParamUpdateCfm(conidx, true, 0xFFFF, 0xFFFF);

        }
            break;

        case GAPC_PARAM_UPDATED_IND:
        {
            const struct gapc_param_updated_ind *p = param;

        }
            break;

#if CFG_BOND_LIST_IN_NVR2
        case GAPC_ENCRYPT_REQ_IND:
        {
            const struct gapc_encrypt_req_ind *p = param;
            /* Accept request if bond information is valid & EDIV/RAND match */
            bool found = (GAPC_IsBonded(conidx)
                          && p->ediv == GAPC_GetBondInfo(conidx)->EDIV
                          && !memcmp(p->rand_nb.nb,
                                  GAPC_GetBondInfo(conidx)->RAND,
                                  GAP_RAND_NB_LEN));



            GAPC_EncryptCfm(conidx, found, GAPC_GetBondInfo(conidx)->LTK);
        }
            break;

        case GAPC_ENCRYPT_IND:
        {
            //PRINTF("GAPC_ENCRYPT_IND: Link encryption is ON\r\n");
        }
            break;

        case GAPC_BOND_REQ_IND:
        {
            const struct gapc_bond_req_ind *p = param;
            switch (p->request)
            {
                case GAPC_PAIRING_REQ:
                {
                    bool accept = BondList_Size() < APP_BONDLIST_SIZE;
                    union gapc_bond_cfm_data pairingRsp =
                    { .pairing_feat =
                    {
                        .iocap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
                        .oob = GAP_OOB_AUTH_DATA_NOT_PRESENT,
                        .key_size = KEY_LEN,
                        .ikey_dist = (GAP_KDIST_IDKEY | GAP_KDIST_SIGNKEY),
                        .rkey_dist = (GAP_KDIST_ENCKEY
                                      | GAP_KDIST_IDKEY
                                      | GAP_KDIST_SIGNKEY), } };
#ifdef SECURE_CONNECTION
                    if (p->data.auth_req & GAP_AUTH_SEC_CON)
                    {
                        pairingRsp.pairing_feat.auth = GAP_AUTH_REQ_SEC_CON_BOND;
                        pairingRsp.pairing_feat.sec_req = GAP_SEC1_SEC_CON_PAIR_ENC;
                    }
                    else
#endif    /* ifdef SECURE_CONNECTION */
                    {
                        pairingRsp.pairing_feat.auth = GAP_AUTH_REQ_NO_MITM_BOND;
                        pairingRsp.pairing_feat.sec_req = GAP_NO_SEC;
                    }

                    GAPC_BondCfm(conidx, GAPC_PAIRING_RSP, accept, &pairingRsp);
                }
                    break;

                case GAPC_LTK_EXCH: /* Prepare and send random LTK (legacy only) */
                {
                    //PRINTF("GAPC_BOND_REQ_IND / GAPC_LTK_EXCH\r\n");
                    union gapc_bond_cfm_data ltkExch;
                    ltkExch.ltk.ediv = co_rand_hword();
                    for (uint8_t i = 0, i2 = GAP_RAND_NB_LEN;
                            i < GAP_RAND_NB_LEN; i++, i2++)
                    {
                        ltkExch.ltk.randnb.nb[i] = co_rand_byte();
                        ltkExch.ltk.ltk.key[i] = co_rand_byte();
                        ltkExch.ltk.ltk.key[i2] = co_rand_byte();
                    }
                    GAPC_BondCfm(conidx, GAPC_LTK_EXCH, true, &ltkExch); /* Send confirmation */
                }
                    break;

                case GAPC_TK_EXCH: /* Prepare and send TK */
                {
                    //PRINTF("GAPC_BOND_REQ_IND / GAPC_TK_EXCH\r\n");
                    /* IO Capabilities are set to GAP_IO_CAP_NO_INPUT_NO_OUTPUT in this application.
                     * Therefore TK exchange is NOT performed. It is always set to 0 (Just Works algorithm). */
                }
                    break;

                case GAPC_IRK_EXCH:
                {
                    //PRINTF("GAPC_BOND_REQ_IND / GAPC_IRK_EXCH\r\n");
                    union gapc_bond_cfm_data irkExch;
                    memcpy(irkExch.irk.addr.addr.addr,
                            GAPM_GetDeviceConfig()->addr.addr, GAP_BD_ADDR_LEN);
                    irkExch.irk.addr.addr_type = GAPM_GetDeviceConfig()->addr_type;
                    memcpy(irkExch.irk.irk.key, GAPM_GetDeviceConfig()->irk.key,
                            GAP_KEY_LEN);
                    GAPC_BondCfm(conidx, GAPC_IRK_EXCH, true, &irkExch); /* Send confirmation */
                }
                    break;

                case GAPC_CSRK_EXCH:
                {
                    //PRINTF("GAPC_BOND_REQ_IND / GAPC_CSRK_EXCH\r\n");
                    union gapc_bond_cfm_data csrkExch;
                    Device_Param_Read(PARAM_ID_CSRK, csrkExch.csrk.key);
                    GAPC_BondCfm(conidx, GAPC_CSRK_EXCH, true, &csrkExch); /* Send confirmation */
                }
                    break;
            }
        }
            break;

        case GAPC_LE_PKT_SIZE_IND:
        {
            const struct gapc_le_pkt_size_ind *p = param;


        }
            break;

        case GAPC_LE_PHY_IND:
        {
            const struct gapc_le_phy_ind *p = param;

        }
#endif /* CFG_BOND_LIST_IN_NVR2 */
    }
}

/** \brief BLE Kernel message handler for messages from GATTC task. */
static void APP_BLE_GATTC_Handler(ke_msg_id_t const msg_id,
        void const *param, ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    switch (msg_id)
    {
        /* Listen for task completed events.
         *
         * These will be received after notification is sent over air.
         */
        case GATTC_CMP_EVT:
        {
            const struct gattc_cmp_evt *p = param;

            /* Print message on for events with errors. */
            if (p->status != 0)
            {

            }
        }
            break;

        case GATTC_MTU_CHANGED_IND:
        {
            const struct gattc_mtu_changed_ind *p = param;

            //PRINTF("GATTC_MTU_CHANGED_IND mtu=%u\r\n", p->mtu);
        }
            break;
    }
}

void APP_BLE_PeripheralServerInitialize(const uint16_t cs_att_db_size)
{
    /* Check if RF output power is compatible with current calibration settings. */
    //REQUIRE(OUTPUT_POWER_DBM <= RF_TX_POWER_LEVEL_DBM);

   // REQUIRE(cs_att_db_size > 1);

    /* Set radio clock accuracy in ppm */
    BLE_DeviceParam_Set_ClockAccuracy(RADIO_CLOCK_ACCURACY);

    /* Initialize the kernel and Bluetooth stack */
    Kernel_Init(0);
    BLE_InitNoTL(0);

    /* Set radio output power of RF */
    Sys_RFFE_SetTXPower(OUTPUT_POWER_DBM);

    /* Create the application task handler */
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    /* Enable Bluetooth related interrupts */
    NVIC->ISER[1] = (NVIC_BLE_CSCNT_INT_ENABLE      |
                     NVIC_BLE_SLP_INT_ENABLE        |
                     NVIC_BLE_RX_INT_ENABLE         |
                     NVIC_BLE_EVENT_INT_ENABLE      |
                     NVIC_BLE_CRYPT_INT_ENABLE      |
                     NVIC_BLE_ERROR_INT_ENABLE      |
                     NVIC_BLE_GROSSTGTIM_INT_ENABLE |
                     NVIC_BLE_FINETGTIM_INT_ENABLE  |
                     NVIC_BLE_SW_INT_ENABLE);

    /* Add application message handlers to BLE kernel. */
    MsgHandler_Add(TASK_ID_GAPM, APP_BLE_GAPM_GATTM_Handler);
    MsgHandler_Add(GATTM_ADD_SVC_RSP, APP_BLE_GAPM_GATTM_Handler);
    MsgHandler_Add(TASK_ID_GAPC, APP_BLE_GAPC_Handler);
    MsgHandler_Add(TASK_ID_GATTC, APP_BLE_GATTC_Handler);

    /* Set initial advertising data. */
    APP_BLE_SetAdvScanData();

    /* Restart BLE stack to start the initialization process. */
    GAPM_ResetCmd();

    cs_att_env.cs_att_count = 0;
    cs_att_env.cs_service_count = 0;
    cs_att_env.cs_att_db_size = cs_att_db_size;
    cs_att_env.cs_att_db = malloc(cs_att_db_size * sizeof(struct att_db_desc));


}

int32_t APP_BLE_PeripheralServerAddCustomService(const struct att_db_desc *p_atts,
        const uint16_t atts_count, uint16_t *p_attidx_offset)
{


    /* Is there enough space for additional service in the att db? */
    if ((cs_att_env.cs_att_db_size - cs_att_env.cs_att_count) < atts_count)
    {
        return -1;
    }

    /* Store offset that will be applied to all attidx of this service. */
    *p_attidx_offset = cs_att_env.cs_att_count;

    /* Increment number of services in the db */
    cs_att_env.cs_service_count += 1;

    /* First attribute must declare a service. */


    for (uint16_t i = 0; i < atts_count; ++i)
    {
        /* Service attidx must start from 0 and increment without gaps. */

        /* Only first attribute is allowed to declare a service. */


        /* Copy attribute data to internal database. */
        struct att_db_desc *p_att = cs_att_env.cs_att_db + cs_att_env.cs_att_count;
        memcpy(p_att, p_atts + i, sizeof(struct att_db_desc));

        /* Add the attidx offset */
        p_att->att_idx += *p_attidx_offset;

        cs_att_env.cs_att_count += 1;
    }

    return 0;
}

uint16_t APP_BLE_PeripheralServerRegisterKernelMsgIds(uint8_t msg_id_count)
{
    /* Max 256 events can be registered for one task. */


    uint16_t offset = registered_kernel_msg_id_count;
    registered_kernel_msg_id_count += msg_id_count;

    return TASK_FIRST_MSG(TASK_ID_APP) + offset;
}

/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ot_rpc_ids.h>

#include <nrf_rpc/nrf_rpc_serialize.h>
#include <nrf_rpc/nrf_rpc_cbkproxy.h>

#include <openthread/thread.h>
#include <openthread/link.h>

struct active_scan_result {
    uint8_t  ext_addr[OT_EXT_ADDRESS_SIZE];
    uint8_t  nwk_name[OT_NETWORK_NAME_MAX_SIZE + 1];
    uint8_t  ext_pan_id[OT_EXT_PAN_ID_SIZE];
    uint8_t  steering_data[OT_STEERING_DATA_MAX_LENGTH];
    uint16_t pan_id;
    uint16_t joiner_udp_port;
    uint8_t  channel;
    int8_t   rssi;
    uint8_t  lqi;
    uint8_t  version : 4;
             is_native : 1;
             discover : 1;
             is_joinable : 1;
} __attribute__((packed));

static void report_decoding_error(uint8_t cmd_evt_id, void *data)
{
	nrf_rpc_err(-EBADMSG, NRF_RPC_ERR_SRC_RECV, &ot_group, cmd_evt_id, NRF_RPC_PACKET_TYPE_CMD);
}

static void ot_rpc_thread_discover_cb_rpc_handler(const struct nrf_rpc_group *group,
					       struct nrf_rpc_cbor_ctx *ctx, void *handler_data)
{
	otHandleActiveScanResult callback_slot;
    otActiveScanResult aResult;
    struct active_scan_result result;
	void *pkt_data = &result;
	void *context;

    nrf_rpc_decode_buffer(ctx, pkt_data, sizeof(result));
    if (pkt_data) {
        memcpy(aResult.mExtAddress.m8, result.ext_addr, OT_EXT_ADDRESS_SIZE);
        memcpy(aResult.mNetworkName.m8, result.nwk_name, OT_NETWORK_NAME_MAX_SIZE + 1);
        memcpy(aResult.mExtendedPanId.m8, result.ext_pan_id, OT_EXT_PAN_ID_SIZE);
        memcpy(aResult.mSteeringData.m8, result.steering_data, OT_STEERING_DATA_MAX_LENGTH);
        aResult.mPanId = result.pan_id;
        aResult.mJoinerUdpPort = result.joiner_udp_port;
        aResult.mChannel = result.channel;
        aResult.mRssi = result.rssi;
        aResult.mLqi = result.lqi;
        aResult.mVersion = result.version;
        aResult.mIsNative = result.is_native;
        aResult.mDiscover = result.discover;
        aResult.mIsJoinable = result.is_joinable;
    }
	context = (void *)nrf_rpc_decode_uint(ctx);
	callback_slot = (otHandleActiveScanResult)nrf_rpc_decode_callback_call(ctx);

	if (!nrf_rpc_decoding_done_and_check(group, ctx)) {
		goto decoding_error;
	}

	if (callback_slot) {
		callback_slot(pkt_data ? &aResult : NULL, context);
	}

	nrf_rpc_rsp_send_void(group);

	return;

decoding_error:
	report_decoding_error(OT_RPC_CMD_THREAD_DISCOVER_CB, handler_data);
}

NRF_RPC_CBOR_CMD_DECODER(ot_group, ot_rpc_thread_discover_cb, OT_RPC_CMD_THREAD_DISCOVER_CB,
			 ot_rpc_thread_discover_cb_rpc_handler, NULL);

otError otThreadDiscover(otInstance *aInstance, uint32_t aScanChannels, uint16_t aPanId,
			 bool aJoiner, bool aEnableEui64Filtering,
			 otHandleActiveScanResult aCallback, void *aCallbackContext)
{
	struct nrf_rpc_cbor_ctx ctx;
	size_t cbor_buffer_size;
	otError error;

	ARG_UNUSED(aInstance);

	cbor_buffer_size += sizeof(uint32_t) + 1;  /* aScanChannels */
	cbor_buffer_size += sizeof(uint32_t) + 1;  /* aPanId */
	cbor_buffer_size += 1;                     /* aJoiner */
	cbor_buffer_size += 1;                     /* aEnableEui64Filtering */
	cbor_buffer_size += sizeof(uintptr_t) + 1; /* aCallback */
	cbor_buffer_size += sizeof(uint32_t) + 1;  /* aCallbackContext */

	nrf_rpc_encode_uint(&ctx, aScanChannels);
	nrf_rpc_encode_uint(&ctx, aPanId);
	nrf_rpc_encode_bool(&ctx, aJoiner);
	nrf_rpc_encode_bool(&ctx, aEnableEui64Filtering);
	nrf_rpc_encode_callback(&ctx, aCallback);
	nrf_rpc_encode_uint(&ctx, (uintptr_t)aContext);

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_THREAD_DISCOVER, &ctx, ot_rpc_decode_error,
				&error);

	return error;
}

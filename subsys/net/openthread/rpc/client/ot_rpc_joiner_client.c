/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ot_rpc_ids.h>
#include <ot_rpc_types.h>
#include <ot_rpc_common.h>

#include <nrf_rpc_cbor.h>
#include <nrf_rpc/nrf_rpc_serialize.h>
#include <nrf_rpc/nrf_rpc_cbkproxy.h>

#include <openthread/joiner.h>

#define OT_JOINER_MAX_STATE_STRING_LENGTH 16

static char *state_str = (char[OT_JOINER_MAX_STATE_STRING_LENGTH]){};
static otJoinerDiscerner discerner;
static otExtAddress ext_address;

static void ot_rpc_decode_discerner(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
				    void *handler_data)
{
	otJoinerDiscerner *discerner = (otJoinerDiscerner *)handler_data;

	nrf_rpc_decode_buffer(ctx, discerner, sizeof(otJoinerDiscerner));
}

static void ot_rpc_decode_state_string(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
				    void *handler_data)
{
	char *state_string = *(char **)handler_data;

	if (!nrf_rpc_decode_str(ctx, state_string, OT_JOINER_MAX_STATE_STRING_LENGTH)) {
		handler_data = NULL;
	}
}

static void report_decoding_error(uint8_t cmd_evt_id, void *data)
{
	nrf_rpc_err(-EBADMSG, NRF_RPC_ERR_SRC_RECV, &ot_group, cmd_evt_id, NRF_RPC_PACKET_TYPE_CMD);
}

static void ot_rpc_joiner_start_cb_rpc_handler(const struct nrf_rpc_group *group,
					       struct nrf_rpc_cbor_ctx *ctx, void *handler_data)
{
	otJoinerCallback callback_slot;
	otError error;
	void *context;

	error = nrf_rpc_decode_int(ctx);
	context = (void *)nrf_rpc_decode_uint(ctx);
	callback_slot = (otJoinerCallback)nrf_rpc_decode_callback_call(ctx);

	if (!nrf_rpc_decoding_done_and_check(group, ctx)) {
		goto decoding_error;
	}

	if (callback_slot) {
		callback_slot(error, context);
	}

	nrf_rpc_rsp_send_void(group);

	return;

decoding_error:
	report_decoding_error(OT_RPC_CMD_JOINER_START_CB, handler_data);
}

NRF_RPC_CBOR_CMD_DECODER(ot_group, ot_rpc_joiner_start_cb, OT_RPC_CMD_JOINER_START_CB,
			 ot_rpc_joiner_start_cb_rpc_handler, NULL);

otError otJoinerStart(otInstance *aInstance, const char *aPskd, const char *aProvisioningUrl,
		      const char *aVendorName, const char *aVendorModel,
		      const char *aVendorSwVersion, const char *aVendorData,
		      otJoinerCallback aCallback, void *aContext)
{
	struct nrf_rpc_cbor_ctx ctx;
	size_t scratchpad_size = 0;
	size_t cbor_buffer_size = sizeof(size_t);
	const char *string_array[] = {aPskd, aProvisioningUrl, aVendorName,
				      aVendorModel, aVendorSwVersion, aVendorData};
	otError error;

	ARG_UNUSED(aInstance);

	for (size_t i = 0; i < ARRAY_SIZE(string_array); i++) {
		if (string_array[i] != NULL) {
			scratchpad_size += strlen(string_array[i]) + 1;
		}
	}
	scratchpad_size = NRF_RPC_SCRATCHPAD_ALIGN(scratchpad_size);
	cbor_buffer_size += scratchpad_size;
	cbor_buffer_size += sizeof(uintptr_t) + 1; /* aCallback */
	cbor_buffer_size += sizeof(uint32_t) + 1;  /* aContext */

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);

	nrf_rpc_encode_uint(&ctx, scratchpad_size);

	for (size_t i = 0; i < ARRAY_SIZE(string_array); i++) {
		nrf_rpc_encode_str(&ctx, string_array[i],
				   string_array[i] == NULL ? 0 : strlen(string_array[i]));
	}

	nrf_rpc_encode_callback(&ctx, aCallback);
	nrf_rpc_encode_uint(&ctx, (uintptr_t)aContext);

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_START, &ctx, ot_rpc_decode_error,
				&error);

	return error;
}

void otJoinerStop(otInstance *aInstance)
{
	const size_t cbor_buffer_size = 0;
	struct nrf_rpc_cbor_ctx ctx;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_STOP, &ctx, ot_rpc_decode_void, NULL);
}

otJoinerState otJoinerGetState(otInstance *aInstance)
{
	const size_t cbor_buffer_size = 0;
	struct nrf_rpc_cbor_ctx ctx;
	otJoinerState state;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_GET_STATE, &ctx,
				nrf_rpc_rsp_decode_i32, &state);

	return state;
}

const otExtAddress *otJoinerGetId(otInstance *aInstance)
{
	const size_t cbor_buffer_size = 0;
	struct nrf_rpc_cbor_ctx ctx;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_GET_ID, &ctx, ot_rpc_decode_extaddr,
				ext_address.m8);

	return &ext_address;
}

otError otJoinerSetDiscerner(otInstance *aInstance, otJoinerDiscerner *aDiscerner)
{
	const size_t cbor_buffer_size = sizeof(otJoinerDiscerner) + 1;
	struct nrf_rpc_cbor_ctx ctx;
	otError error;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);

	nrf_rpc_encode_buffer(&ctx, aDiscerner, sizeof(otJoinerDiscerner));

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_SET_DISCERNER, &ctx,
				ot_rpc_decode_error, &error);

	return error;
}

const otJoinerDiscerner *otJoinerGetDiscerner(otInstance *aInstance)
{
	const size_t cbor_buffer_size = 0;
	struct nrf_rpc_cbor_ctx ctx;
	otJoinerDiscerner *discerner_ptr = &discerner;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);
	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_GET_DISCERNER, &ctx,
				ot_rpc_decode_discerner, discerner_ptr);

	return discerner_ptr;
}

const char *otJoinerStateToString(otJoinerState aState)
{
	const size_t cbor_buffer_size = sizeof(int) + 1;
	struct nrf_rpc_cbor_ctx ctx;
	char **str_ptr = &state_str;

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);

	nrf_rpc_encode_int(&ctx, aState);

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_GET_STATE_STRING, &ctx,
				ot_rpc_decode_state_string, str_ptr);

	return !str_ptr ? NULL : state_str;
}

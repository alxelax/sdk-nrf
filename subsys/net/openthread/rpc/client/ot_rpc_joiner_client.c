/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ot_rpc_ids.h>
#include <ot_rpc_types.h>

#include <nrf_rpc_cbor.h>

#include <openthread/joiner.h>

/**
 * Represents a Joiner Discerner.
 *
 */
/*typedef struct otJoinerDiscerner
{
    uint64_t mValue;  ///< Discerner value (the lowest `mLength` bits specify the discerner).
    uint8_t  mLength; ///< Length (number of bits) - must be non-zero and at most `OT_JOINER_MAX_DISCERNER_LENGTH`.
} otJoinerDiscerner;*/

/**
 * Pointer is called to notify the completion of a join operation.
 *
 * @param[in]  aError    OT_ERROR_NONE if the join process succeeded.
 *                       OT_ERROR_SECURITY if the join process failed due to security credentials.
 *                       OT_ERROR_NOT_FOUND if no joinable network was discovered.
 *                       OT_ERROR_RESPONSE_TIMEOUT if a response timed out.
 * @param[in]  aContext  A pointer to application-specific context.
 *
 */
typedef void (*otJoinerCallback)(otError aError, void *aContext);

/**
 * Enables the Thread Joiner role.
 *
 * @param[in]  aInstance         A pointer to an OpenThread instance.
 * @param[in]  aPskd             A pointer to the PSKd.
 * @param[in]  aProvisioningUrl  A pointer to the Provisioning URL (may be NULL).
 * @param[in]  aVendorName       A pointer to the Vendor Name (may be NULL).
 * @param[in]  aVendorModel      A pointer to the Vendor Model (may be NULL).
 * @param[in]  aVendorSwVersion  A pointer to the Vendor SW Version (may be NULL).
 * @param[in]  aVendorData       A pointer to the Vendor Data (may be NULL).
 * @param[in]  aCallback         A pointer to a function that is called when the join operation completes.
 * @param[in]  aContext          A pointer to application-specific context.
 *
 * @retval OT_ERROR_NONE              Successfully started the Joiner role.
 * @retval OT_ERROR_BUSY              The previous attempt is still on-going.
 * @retval OT_ERROR_INVALID_ARGS      @p aPskd or @p aProvisioningUrl is invalid.
 * @retval OT_ERROR_INVALID_STATE     The IPv6 stack is not enabled or Thread stack is fully enabled.
 *
 */
otError otJoinerStart(otInstance *aInstance, const char *aPskd, const char *aProvisioningUrl,
		      const char *aVendorName, const char *aVendorModel,
		      const char *aVendorSwVersion, const char *aVendorData,
		      otJoinerCallback aCallback, void *aContext)
{
	const size_t cbor_buffer_size = 10;
	struct nrf_rpc_cbor_ctx ctx;
	otError error;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);
	/* TODO: implement string merging and sending. */
	/* TODO: implement callback & address pseudonymization. */
	zcbor_uint32_put(ctx.zs, (uint32_t)aCallback);
	zcbor_uint32_put(ctx.zs, (uint32_t)aContext);

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_START, &ctx,
				ot_rpc_decode_error, &error);

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

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_GET_STATE, &ctx, ot_rpc_decode_state,
				&state); // TODO: implement state decoding

	return state;
}

const otExtAddress *otJoinerGetId(otInstance *aInstance)
{
	const size_t cbor_buffer_size = 0;
	struct nrf_rpc_cbor_ctx ctx;
	const otExtAddress address;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_GET_ID, &ctx, ot_rpc_decode_address,
				&address); // TODO: implement address decoding

	return &address;
}

/**
 * Sets the Joiner Discerner.
 *
 * The Joiner Discerner is used to calculate the Joiner ID during the Thread Commissioning process. For more
 * information, refer to #otJoinerGetId.
 * @note The Joiner Discerner takes the place of the Joiner EUI-64 during the joiner session of Thread Commissioning.
 *
 * @param[in]   aInstance    A pointer to the OpenThread instance.
 * @param[in]   aDiscerner   A pointer to a Joiner Discerner. If NULL clears any previously set discerner.
 *
 * @retval OT_ERROR_NONE           The Joiner Discerner updated successfully.
 * @retval OT_ERROR_INVALID_ARGS   @p aDiscerner is not valid (specified length is not within valid range).
 * @retval OT_ERROR_INVALID_STATE  There is an ongoing Joining process so Joiner Discerner could not be changed.
 *
 */
otError otJoinerSetDiscerner(otInstance *aInstance, otJoinerDiscerner *aDiscerner)
{
	const size_t cbor_buffer_size = 10;
	struct nrf_rpc_cbor_ctx ctx;
	otError error;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);
	/* TODO: implement discerner sending. */

	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_SET_DISCERNER, &ctx,
				ot_rpc_decode_error, &error);

	return error;
}

/**
 * Gets the Joiner Discerner. For more information, refer to #otJoinerSetDiscerner.
 *
 * @param[in]   aInstance       A pointer to the OpenThread instance.
 *
 * @returns A pointer to Joiner Discerner or NULL if none is set.
 *
 */
const otJoinerDiscerner *otJoinerGetDiscerner(otInstance *aInstance)
{
	const size_t cbor_buffer_size = 10;
	struct nrf_rpc_cbor_ctx ctx;
	const otJoinerDiscerner discerner;

	ARG_UNUSED(aInstance);

	NRF_RPC_CBOR_ALLOC(&ot_group, ctx, cbor_buffer_size);
	nrf_rpc_cbor_cmd_no_err(&ot_group, OT_RPC_CMD_JOINER_GET_DISCERNER, &ctx,
				ot_rpc_decode_discerner, &discerner);

	return &discerner;
}

/**
 * Converts a given joiner state enumeration value to a human-readable string.
 *
 * @param[in] aState   The joiner state.
 *
 * @returns A human-readable string representation of @p aState.
 *
 */
const char *otJoinerStateToString(otJoinerState aState)
{
}

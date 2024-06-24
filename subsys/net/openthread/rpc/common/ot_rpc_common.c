/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/net/openthread.h>

void ot_rpc_decode_error(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			 void *handler_data)
{
	otError *error = handler_data;
	int32_t errorCode;

	*error = zcbor_int32_decode(ctx->zs, &errorCode) ? (otError)errorCode : -EINVAL;
}

void ot_rpc_decode_void(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			void *handler_data)
{
	ARG_UNUSED(group);
	ARG_UNUSED(ctx);
	ARG_UNUSED(handler_data);
}

/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ot_rpc_common.h"

#include <nrf_rpc/nrf_rpc_serialize.h>

#include <openthread/joiner.h>

void ot_rpc_decode_error(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			 void *handler_data)
{
	nrf_rpc_rsp_decode_i32(group, ctx, handler_data);
}

void ot_rpc_decode_extaddr(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			   void *handler_data)
{
	otExtAddress *address = (otExtAddress *)handler_data;

	nrf_rpc_decode_buffer(ctx, address->m8, sizeof(address->m8));
}

void ot_rpc_decode_void(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			void *handler_data)
{
	nrf_rpc_rsp_decode_void(group, ctx, handler_data);
}

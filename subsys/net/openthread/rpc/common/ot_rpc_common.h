/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef OT_RPC_COMMON_H_
#define OT_RPC_COMMON_H_

#include <nrf_rpc_cbor.h>

NRF_RPC_GROUP_DECLARE(ot_group);

void ot_rpc_decode_error(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			 void *handler_data);
void ot_rpc_decode_void(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			void *handler_data);
void ot_rpc_decode_extaddr(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			   void *handler_data);

#endif /* OT_RPC_COMMON_H_ */

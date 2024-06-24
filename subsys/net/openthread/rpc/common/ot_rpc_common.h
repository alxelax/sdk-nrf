/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

NRF_RPC_GROUP_DECLARE(ot_group);

/** @brief Decode OpenThread error code in response to a command.
 * 
 * @param[in] group OpenThread RPC group
 * @param[in] ctx Cbor context
 * @param[inout] handler_data Error code
 */
void ot_rpc_decode_error(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			 void *handler_data);

/** @brief Stub handler for serialized functions that return void.
 * 
 * @param[in] group OpenThread RPC group
 * @param[in] ctx Cbor context
 * @param[inout] handler_data Error code
 */
void ot_rpc_decode_void(const struct nrf_rpc_group *group, struct nrf_rpc_cbor_ctx *ctx,
			void *handler_data);

/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Bluetooth Mesh DFU Target library
 */

#ifndef BT_MESH_DFU_TARGET_LIB_H__
#define BT_MESH_DFU_TARGET_LIB_H__

#include <zephyr/bluetooth/mesh.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_BT_MESH_DFU_TARGET_LIB) || defined(__DOXYGEN__)

/** Firmware Update Server instance owned by the library. */
extern struct bt_mesh_dfu_srv bt_mesh_dfu_target_lib_srv;

/** @brief Firmware Update and BLOB Transfer Server model entries.
 *
 *  Place directly after the last model in a @ref BT_MESH_MODEL_LIST,
 *  without a separating comma. Expands to nothing when
 *  @kconfig{CONFIG_BT_MESH_DFU_TARGET_LIB} is disabled.
 */
#define BT_MESH_DFU_TARGET_LIB_MODELS , BT_MESH_MODEL_DFU_SRV(&bt_mesh_dfu_target_lib_srv)

#else

#define BT_MESH_DFU_TARGET_LIB_MODELS

#endif /* CONFIG_BT_MESH_DFU_TARGET_LIB */

#ifdef __cplusplus
}
#endif

#endif /* BT_MESH_DFU_TARGET_LIB_H__ */

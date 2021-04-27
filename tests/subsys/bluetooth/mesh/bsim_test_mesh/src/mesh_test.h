/** @file
 *  @brief Common functionality for Bluetooth Mesh BabbleSim tests.
 */

/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_TESTS_BLUETOOTH_BSIM_BT_BSIM_TEST_MESH_MESH_TEST_H_
#define ZEPHYR_TESTS_BLUETOOTH_BSIM_BT_BSIM_TEST_MESH_MESH_TEST_H_
#include "kernel.h"

#include "bs_types.h"
#include "bs_tracing.h"
#include "time_machine.h"
#include "bstests.h"

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>

#define FAIL(msg, ...)                                                         \
	do {                                                                   \
		bst_result = Failed;                                           \
		bs_trace_error_time_line(msg "\n", ##__VA_ARGS__);             \
	} while (0)

#define PASS()                                                                 \
	do {                                                                   \
		bst_result = Passed;                                           \
		bs_trace_info_time(1, "%s PASSED\n", __func__);                \
	} while (0)

#define ASSERT_OK(cond, ...)                                                   \
	do {                                                                   \
		int _err = (cond);                                             \
		if (_err) {                                                    \
			bst_result = Failed;                                   \
			bs_trace_error_time_line(                              \
				#cond " failed with error %d\n", _err);        \
		}                                                              \
	} while (0)

struct bt_mesh_test_cfg {
	uint16_t addr;
	uint8_t dev_key[16];
};

extern enum bst_result_t bst_result;
extern const uint8_t test_net_key[16];
extern const uint8_t test_app_key[16];
extern const uint8_t test_va_uuid[16];

void bt_mesh_test_cfg_set(const struct bt_mesh_test_cfg *cfg, int wait_time);
void bt_mesh_test_setup(void);
void bt_mesh_test_timeout(bs_time_t HW_device_time);

void scheduler_action_set_cb(struct bt_mesh_scheduler_srv *srv,
			     struct bt_mesh_msg_ctx *ctx,
			     uint8_t idx,
			     struct bt_mesh_schedule_entry *entry);
#endif /* ZEPHYR_TESTS_BLUETOOTH_BSIM_BT_BSIM_TEST_MESH_MESH_TEST_H_ */

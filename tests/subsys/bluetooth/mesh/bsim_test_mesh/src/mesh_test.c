/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mesh_test.h"

#define LOG_MODULE_NAME mesh_test

#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* Max number of messages that can be pending on RX at the same time */
#define RECV_QUEUE_SIZE 32

const struct bt_mesh_test_cfg *cfg;

static void time_update_cb(struct bt_mesh_time_srv *srv,
			   struct bt_mesh_msg_ctx *ctx,
			   enum bt_mesh_time_update_types type)
{}

static struct bt_mesh_cfg_cli cfg_cli;

static struct bt_mesh_time_srv time_srv = BT_MESH_TIME_SRV_INIT(time_update_cb);
static struct bt_mesh_scheduler_srv scheduler_srv =
	BT_MESH_SCHEDULER_SRV_INIT(scheduler_action_set_cb, &time_srv);
static struct bt_mesh_scheduler_cli scheduler_cli;

static struct bt_mesh_elem elems[] = {
	BT_MESH_ELEM(0, BT_MESH_MODEL_LIST(
		BT_MESH_MODEL_CFG_SRV,
		BT_MESH_MODEL_CFG_CLI(&cfg_cli),
		BT_MESH_MODEL_SCHEDULER_SRV(&scheduler_srv),
		BT_MESH_MODEL_TIME_SRV(&time_srv),
		BT_MESH_MODEL_SCHEDULER_CLI(&scheduler_cli)),
	BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.elem = elems,
	.elem_count = ARRAY_SIZE(elems),
};

const uint8_t test_net_key[16] = { 1, 2, 3 };
const uint8_t test_app_key[16] = { 4, 5, 6 };
const uint8_t test_va_uuid[16] = "Mesh Label UUID";

static void bt_enabled(int err)
{
	static struct bt_mesh_prov prov;
	uint8_t status;

	if (err) {
		FAIL("Bluetooth init failed (err %d)\n", err);
		return;
	}

	LOG_INF("Bluetooth initialized");

	err = bt_mesh_init(&prov, &comp);
	if (err) {
		FAIL("Initializing mesh failed (err %d)", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_mesh_provision(test_net_key, 0, 0, 0, cfg->addr, cfg->dev_key);
	if (err) {
		FAIL("Provisioning failed (err %d)", err);
		return;
	}

	LOG_INF("Mesh initialized");

	/* Self configure */

	err = bt_mesh_cfg_app_key_add(0, cfg->addr, 0, 0, test_app_key,
				      &status);
	if (err || status) {
		FAIL("AppKey add failed (err %d, status %u)", err, status);
		return;
	}

	err = bt_mesh_cfg_net_transmit_set(0, cfg->addr,
					   BT_MESH_TRANSMIT(2, 20), &status);
	if (err || status != BT_MESH_TRANSMIT(2, 20)) {
		FAIL("Net transmit set failed (err %d, status %u)", err,
		     status);
		return;
	}

	/* Scheduler model configuration */
	err = bt_mesh_cfg_mod_app_bind(0, cfg->addr, cfg->addr, 0, BT_MESH_MODEL_ID_SCHEDULER_SRV,
				       &status);
	if (err || status) {
		FAIL("Mod app bind failed (err %d, status %u)", err, status);
		return;
	}

	err = bt_mesh_cfg_mod_app_bind(0, cfg->addr, cfg->addr, 0, BT_MESH_MODEL_ID_SCHEDULER_SETUP_SRV,
				       &status);
	if (err || status) {
		FAIL("Mod app bind failed (err %d, status %u)", err, status);
		return;
	}

	err = bt_mesh_cfg_mod_app_bind(0, cfg->addr, cfg->addr, 0, BT_MESH_MODEL_ID_SCHEDULER_CLI,
				       &status);
	if (err || status) {
		FAIL("Mod app bind failed (err %d, status %u)", err, status);
		return;
	}
}

void bt_mesh_test_setup(void)
{
	int err;

	err = bt_enable(bt_enabled);
	if (err) {
		FAIL("Bluetooth init failed (err %d)", err);
		return;
	}
}

void bt_mesh_test_timeout(bs_time_t HW_device_time)
{
	if (bst_result != Passed) {
		FAIL("Test timeout (not passed after %i seconds)",
		     HW_device_time / USEC_PER_SEC);
	}

	bs_trace_silent_exit(0);
}

void bt_mesh_test_cfg_set(const struct bt_mesh_test_cfg *my_cfg, int wait_time)
{
	bst_ticker_set_next_tick_absolute(wait_time * USEC_PER_SEC);
	bst_result = In_progress;
	cfg = my_cfg;
}


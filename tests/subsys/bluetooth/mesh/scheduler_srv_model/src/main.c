/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ztest.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>

static void scheduler_action_set_cb(struct bt_mesh_scheduler_srv *srv,
				    struct bt_mesh_msg_ctx *ctx,
				    uint8_t idx,
				    struct bt_mesh_schedule_entry *entry);

static struct bt_mesh_time_srv time_srv = BT_MESH_TIME_SRV_INIT(NULL);
static struct bt_mesh_scheduler_srv scheduler_srv =
	BT_MESH_SCHEDULER_SRV_INIT(scheduler_action_set_cb, &time_srv);

static struct bt_mesh_elem elems[] = {
	BT_MESH_ELEM(0, BT_MESH_MODEL_LIST(
		BT_MESH_MODEL_SCHEDULER_SRV(&scheduler_srv),
		BT_MESH_MODEL_TIME_SRV(&time_srv),
	), BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.elem = elems,
	.elem_count = ARRAY_SIZE(elems),
};

/****************** mock section **********************************/
/****************** mock section **********************************/

/****************** callback section ******************************/
static void scheduler_action_set_cb(struct bt_mesh_scheduler_srv *srv,
				    struct bt_mesh_msg_ctx *ctx,
				    uint8_t idx,
				    struct bt_mesh_schedule_entry *entry)
{
}
/****************** callback section ******************************/
int bt_mesh_power_down_enter(void);
int bt_mesh_power_down_leave(void);

static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
	printk("OOB Number: %u\n", number);

	return 0;
}

static void prov_complete(uint16_t net_idx, uint16_t addr)
{
}

static void prov_reset(void)
{
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

static const uint8_t dev_uuid[16] = { 0xdd, 0xdd };

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.output_size = 4,
	.output_actions = BT_MESH_DISPLAY_NUMBER,
	.output_number = output_number,
	.complete = prov_complete,
	.reset = prov_reset,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

static void test_power_down(void)
{
	int err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
	
	if (IS_ENABLED(CONFIG_SETTINGS)) {
			settings_load();
	}

	bt_mesh_power_down_enter();
	bt_mesh_power_down_leave();
}

void test_main(void)
{
	ztest_test_suite(scheduler_srv_test,
			ztest_unit_test(test_power_down)
			 );

	ztest_run_test_suite(scheduler_srv_test);
}

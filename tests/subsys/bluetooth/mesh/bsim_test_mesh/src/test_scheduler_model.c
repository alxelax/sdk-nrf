/*
 * Copyright (c) 2021 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <fs/fs.h>
#include <ff.h>
#include "mesh_test.h"
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>

#define LOG_MODULE_NAME test_scheduler

#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define WAIT_TIME 60 /*seconds*/
#define FATFS_MNTP   "/RAM:"

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t fatfs_mnt = {
	.type = FS_FATFS,
	.mnt_point = FATFS_MNTP,
	.fs_data = &fat_fs,
};

void scheduler_action_set_cb(struct bt_mesh_scheduler_srv *srv,
			     struct bt_mesh_msg_ctx *ctx,
			     uint8_t idx,
			     struct bt_mesh_schedule_entry *entry)
{
}

static const struct bt_mesh_test_cfg scheduler_srv_model_cfg = {
	.addr = 0x0001,
	.dev_key = { 0x01 },
};

static const struct bt_mesh_test_cfg scheduler_cli_model_cfg = {
	.addr = 0x0002,
	.dev_key = { 0x02 },
};

static void mount_settings_area(void)
{
	int err = fs_mount(&fatfs_mnt);
	LOG_INF("Mount point creation status: %i", err);
}

static void test_server_init(void)
{
	LOG_INF("%s", __func__);
}

static void test_client_init(void)
{
	LOG_INF("%s", __func__);
}

static void test_server_check_init(void)
{
	mount_settings_area();
	bt_mesh_test_cfg_set(&scheduler_srv_model_cfg, WAIT_TIME);
	bt_mesh_test_setup();

	k_sleep(K_SECONDS(5));

	PASS();
}

static void test_client_check_init(void)
{
	mount_settings_area();
	bt_mesh_test_cfg_set(&scheduler_cli_model_cfg, WAIT_TIME);
	bt_mesh_test_setup();

	k_sleep(K_SECONDS(5));

	PASS();
}

#define TEST_CASE(role, name, description)                                     \
	{                                                                      \
		.test_id = "scheduler_model_" #role "_" #name,                 \
		.test_descr = description,                                     \
		.test_post_init_f = test_##role##_init,                        \
		.test_tick_f = bt_mesh_test_timeout,                           \
		.test_main_f = test_##role##_##name,                           \
	}

static const struct bst_test_instance test_scheduler[] = {
	TEST_CASE(server, check_init,        "Initialization of the scheduler server model"),
	TEST_CASE(client, check_init,        "Initialization of the scheduler client model"),
	BSTEST_END_MARKER
};

struct bst_test_list *test_scheduler_model_install(struct bst_test_list *tests)
{
	return bst_add_tests(tests, test_scheduler);
}

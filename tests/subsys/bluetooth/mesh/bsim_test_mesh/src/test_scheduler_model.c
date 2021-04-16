/*
 * Copyright (c) 2021 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mesh_test.h"
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>

void scheduler_action_set_cb(struct bt_mesh_scheduler_srv *srv,
			     struct bt_mesh_msg_ctx *ctx,
			     uint8_t idx,
			     struct bt_mesh_schedule_entry *entry)
{
}

static void test_server_init(void)
{
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

static const struct bst_test_instance test_connect[] = {
	TEST_CASE(server, init,        "Initialization if the scheduler model"),
	BSTEST_END_MARKER
};

struct bst_test_list *test_scheduler_model_install(struct bst_test_list *tests)
{
	tests = bst_add_tests(tests, test_connect);
	return tests;
}

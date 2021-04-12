/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ztest.h>
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>

static struct bt_mesh_time_srv time_srv = BT_MESH_TIME_SRV_INIT(NULL);
static struct bt_mesh_scene_srv scene_srv;
static struct bt_mesh_scheduler_srv scheduler_srv =
	BT_MESH_SCHEDULER_SRV_INIT(NULL, &time_srv);

static struct bt_mesh_elem elems[] = {
	BT_MESH_ELEM(0, BT_MESH_MODEL_LIST(
		BT_MESH_MODEL_SCENE_SRV(&scene_srv),
		BT_MESH_MODEL_SCHEDULER_SRV(&scheduler_srv),
		BT_MESH_MODEL_TIME_SRV(&time_srv),
	), BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.elem = elems,
	.elem_count = ARRAY_SIZE(elems),
};

//static void test_setup(void)
//{
//	int err = 0;
//	bt_mesh_model_foreach(mod_init, &err);
//}
//
//static void test_teardown(void)
//{
//}

static void test_scheduler_init(void)
{
	zassert_false(0 == bt_mesh_init(NULL, &comp), "Test fails");
}

void test_main(void)
{
	ztest_test_suite(scheduler_srv_test,
			ztest_unit_test(test_scheduler_init)
			 );

	ztest_run_test_suite(scheduler_srv_test);
}

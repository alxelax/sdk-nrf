/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ztest.h>
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>

static void scheduler_action_set_cb(struct bt_mesh_scheduler_srv *srv,
				    struct bt_mesh_msg_ctx *ctx,
				    uint8_t idx,
				    struct bt_mesh_schedule_entry *entry);

static struct bt_mesh_time_srv time_srv = BT_MESH_TIME_SRV_INIT(NULL);
static struct bt_mesh_scene_srv scene_srv;
static struct bt_mesh_scheduler_srv scheduler_srv =
	BT_MESH_SCHEDULER_SRV_INIT(scheduler_action_set_cb, &time_srv);

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

int bt_mesh_prov_init(const struct bt_mesh_prov *prov_info)
{
	return 0;
}

int bt_mesh_model_extend(struct bt_mesh_model *mod,
			 struct bt_mesh_model *base_mod)
{

	if (mod->id != BT_MESH_MODEL_ID_SCHEDULER_SRV)
	{
		return 0;
	}

	zassert_equal_ptr(scheduler_srv.model, mod,
		"wrong model pointer for extension.");
	zassert_equal_ptr(scheduler_srv.setup_mod, base_mod,
		"wrong base model pointer for extension.");

	return 0;
}

static void scheduler_action_set_cb(struct bt_mesh_scheduler_srv *srv,
				    struct bt_mesh_msg_ctx *ctx,
				    uint8_t idx,
				    struct bt_mesh_schedule_entry *entry)
{
}

static void test_scheduler_init(void)
{
	zassert_true(0 == bt_mesh_init(NULL, &comp), "bt mesh initialization failed.");

	zassert_equal_ptr(scheduler_srv.model, &elems[0].models[1],
			"wrong initialization of the model pointer.");
	zassert_equal_ptr(scheduler_srv.pub.msg, &scheduler_srv.pub_buf,
			"wrong initialization of the publication message pointer.");
	zassert_not_null(scheduler_srv.pub.update,
			"update handler has not been initialized.");
	zassert_equal_ptr(scheduler_srv.pub_buf.data, scheduler_srv.pub_data,
			"wrong initialization of the network buffer.");
	zassert_equal(scheduler_srv.pub_buf.len, ARRAY_SIZE(scheduler_srv.pub_data),
			"wrong initialization of the network buffer length.");
	zassert_equal(scheduler_srv.pub_buf.size, ARRAY_SIZE(scheduler_srv.pub_data),
			"wrong initialization of the network buffer size.");
	zassert_equal(scheduler_srv.status_bitmap, 0,
			"scheduler status bitmap state has not been initialized.");
	zassert_equal(scheduler_srv.idx, BT_MESH_SCHEDULER_ACTION_ENTRY_COUNT,
				"scheduler register index has not been initialized.");

	// todo scheduler register checking
	// todo delayable work initialization checking

	for (int i = 0; i < BT_MESH_SCHEDULER_ACTION_ENTRY_COUNT; i++) {
		zassert_equal(scheduler_srv.sched_tai[i].sec, 0,
			"scheduler tai sec not initialized instance: %i.", i);
		zassert_equal(scheduler_srv.sched_tai[i].subsec, 0,
			"scheduler tai subsec not initialized instance: %i.", i);
	}
}

void test_main(void)
{
	ztest_test_suite(scheduler_srv_test,
			ztest_unit_test(test_scheduler_init)
			 );

	ztest_run_test_suite(scheduler_srv_test);
}

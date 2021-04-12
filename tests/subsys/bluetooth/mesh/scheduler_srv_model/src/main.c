/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ztest.h>
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>

static void test_scheduler_init(void)
{
	// todo
}

void test_main(void)
{
	ztest_test_suite(scheduler_srv_test,
			 ztest_unit_test(test_scheduler_init)
			 );

	ztest_run_test_suite(scheduler_srv_test);
}

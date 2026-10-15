/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/util.h>
#include <zephyr/settings/settings.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/bluetooth/mesh.h>
#include <bootutil/bootutil_public.h>

#include <bluetooth/mesh/dfu_target_lib.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(bt_mesh_dfu_target_lib, CONFIG_BT_MESH_DFU_TARGET_LIB_LOG_LEVEL);

BUILD_ASSERT(IS_ENABLED(CONFIG_SETTINGS), "Image confirmation relies on settings_load()");

static struct bt_mesh_blob_io_flash blob_flash_stream;
static enum bt_mesh_dfu_effect img_effect = BT_MESH_DFU_EFFECT_NONE;
static struct mcuboot_img_sem_ver img_ver;

static struct {
	uint16_t company_id;
	uint8_t major;
	uint8_t minor;
	uint16_t revision;
	uint32_t build_num;
} __packed fwid;

static struct bt_mesh_dfu_img dfu_imgs[] = { {
	.fwid = &fwid,
	.fwid_len = sizeof(fwid),
} };

#if defined(CONFIG_BT_MESH_DFU_METADATA)
static size_t flash_area_size_get(uint8_t area_id)
{
	const struct flash_area *area;
	size_t fa_size;
	int err;

	err = flash_area_open(area_id, &area);
	if (err) {
		return 0;
	}

	fa_size = area->fa_size;
	flash_area_close(area);

	return fa_size;
}

static bool is_firmware_newer(const struct mcuboot_img_sem_ver *new,
			      const struct mcuboot_img_sem_ver *cur)
{
	if (new->major != cur->major) {
		return new->major > cur->major;
	}

	if (new->minor != cur->minor) {
		return new->minor > cur->minor;
	}

	if (new->revision != cur->revision) {
		return new->revision > cur->revision;
	}

	return new->build_num > cur->build_num;
}
#endif /* CONFIG_BT_MESH_DFU_METADATA */

static int dfu_meta_check(struct bt_mesh_dfu_srv *srv,
			  const struct bt_mesh_dfu_img *img,
			  struct net_buf_simple *metadata_raw,
			  enum bt_mesh_dfu_effect *effect)
{
#if defined(CONFIG_BT_MESH_DFU_METADATA)
	struct bt_mesh_dfu_metadata metadata;
	struct mcuboot_img_sem_ver new_ver;
	uint8_t key[16] = {};
	uint32_t hash;
	int err;

	err = bt_mesh_dfu_metadata_decode(metadata_raw, &metadata);
	if (err) {
		LOG_ERR("Unable to decode metadata: %d", err);
		return -EINVAL;
	}

	LOG_INF("Received firmware %u.%u.%u+%u, size %u, core type 0x%x",
		metadata.fw_ver.major, metadata.fw_ver.minor, metadata.fw_ver.revision,
		metadata.fw_ver.build_num, metadata.fw_size, metadata.fw_core_type);

	new_ver.major = metadata.fw_ver.major;
	new_ver.minor = metadata.fw_ver.minor;
	new_ver.revision = metadata.fw_ver.revision;
	new_ver.build_num = metadata.fw_ver.build_num;

	if (!is_firmware_newer(&new_ver, &img_ver)) {
		LOG_WRN("New firmware version is not newer");
		return -EINVAL;
	}

	if (!(metadata.fw_core_type & BT_MESH_DFU_FW_CORE_TYPE_APP)) {
		LOG_WRN("Only application core firmware is supported");
		return -EINVAL;
	}

	if (flash_area_size_get(PARTITION_ID(slot0_partition)) < metadata.fw_size ||
	    flash_area_size_get(PARTITION_ID(slot1_partition)) < metadata.fw_size) {
		LOG_WRN("New firmware does not fit into flash");
		return -EINVAL;
	}

	err = bt_mesh_dfu_metadata_comp_hash_local_get(key, &hash);
	if (err) {
		LOG_ERR("Failed to compute composition data hash: %d", err);
		return -EINVAL;
	}

	img_effect = (hash == metadata.comp_hash) ? BT_MESH_DFU_EFFECT_NONE :
						    BT_MESH_DFU_EFFECT_UNPROV;
#else
	img_effect = BT_MESH_DFU_EFFECT_UNPROV;
#endif /* CONFIG_BT_MESH_DFU_METADATA */

	LOG_INF("Metadata check succeeded, effect: %d", img_effect);

	*effect = img_effect;

	return 0;
}

/* Stale MCUboot trailer in slot1 makes MCUboot reject the swap request. */
static int erase_trailer(void)
{
	struct boot_swap_state state;
	const struct flash_area *fa;
	const struct flash_parameters *fparam;
	const struct device *fdev;
	int err;

	err = boot_read_swap_state_by_id(PARTITION_ID(slot1_partition), &state);
	if (err) {
		return err;
	}

	if (state.magic == BOOT_MAGIC_UNSET) {
		return 0;
	}

	err = flash_area_open(PARTITION_ID(slot1_partition), &fa);
	if (err) {
		return err;
	}

	fdev = flash_area_get_device(fa);
	fparam = flash_get_parameters(fdev);

	if (flash_params_get_erase_cap(fparam) & FLASH_ERASE_C_EXPLICIT) {
		struct flash_pages_info page;

		err = flash_get_page_info_by_offs(fdev, fa->fa_off + fa->fa_size - 1, &page);
		if (!err) {
			err = flash_area_erase(fa, page.start_offset - fa->fa_off, page.size);
		}
	} else {
		err = flash_area_flatten(fa, fa->fa_size - BOOT_MAGIC_ALIGN_SIZE,
					 BOOT_MAGIC_ALIGN_SIZE);
	}

	flash_area_close(fa);

	return err;
}

static int dfu_start(struct bt_mesh_dfu_srv *srv,
		     const struct bt_mesh_dfu_img *img,
		     struct net_buf_simple *metadata,
		     const struct bt_mesh_blob_io **io)
{
	int err;

	LOG_INF("Firmware upload started");

	err = erase_trailer();
	if (err) {
		LOG_ERR("Failed to erase slot1 trailer: %d", err);
		return err;
	}

	*io = &blob_flash_stream.io;

	return 0;
}

static void dfu_end(struct bt_mesh_dfu_srv *srv, const struct bt_mesh_dfu_img *img, bool success)
{
	LOG_INF("Firmware upload %s", success ? "succeeded" : "failed");

	if (!success) {
		return;
	}

	bt_mesh_dfu_srv_verified(srv);
}

static int dfu_recover(struct bt_mesh_dfu_srv *srv,
		       const struct bt_mesh_dfu_img *img,
		       const struct bt_mesh_blob_io **io)
{
	LOG_INF("Recovering the firmware upload");

	if (IS_ENABLED(CONFIG_BT_MESH_DFD_SRV)) {
		return -ENOTSUP;
	}

	*io = &blob_flash_stream.io;

	return 0;
}

static void do_reboot(struct k_work *work)
{
	sys_reboot(SYS_REBOOT_WARM);
}

static K_WORK_DELAYABLE_DEFINE(pending_reboot, do_reboot);

static int dfu_apply(struct bt_mesh_dfu_srv *srv, const struct bt_mesh_dfu_img *img)
{
	int err;

	LOG_INF("Applying the new firmware");

	err = boot_request_upgrade(BOOT_UPGRADE_TEST);
	if (err) {
		LOG_ERR("Failed to request upgrade: %d", err);
		return err;
	}

	if (img_effect == BT_MESH_DFU_EFFECT_UNPROV) {
		bt_mesh_reset();
		/* Let the mesh clear its settings before rebooting. */
		k_work_schedule(&pending_reboot, K_MSEC(1));
	} else {
		sys_reboot(SYS_REBOOT_WARM);
	}

	return 0;
}

static const struct bt_mesh_dfu_srv_cb dfu_handlers = {
	.check = dfu_meta_check,
	.start = dfu_start,
	.end = dfu_end,
	.apply = dfu_apply,
	.recover = dfu_recover,
};

struct bt_mesh_dfu_srv bt_mesh_dfu_target_lib_srv =
	BT_MESH_DFU_SRV_INIT(&dfu_handlers, dfu_imgs, ARRAY_SIZE(dfu_imgs));

static int dfu_target_lib_init(void)
{
	struct mcuboot_img_header img_header;
	int err;

	err = bt_mesh_blob_io_flash_init(&blob_flash_stream, PARTITION_ID(slot1_partition), 0);
	if (err) {
		LOG_ERR("Failed to init BLOB IO Flash module: %d", err);
		return err;
	}

	err = boot_read_bank_header(PARTITION_ID(slot0_partition), &img_header,
				    sizeof(img_header));
	if (err) {
		LOG_ERR("Failed to read image header: %d", err);
		return err;
	}

	img_ver = img_header.h.v1.sem_ver;

	fwid.company_id = sys_cpu_to_le16(CONFIG_BT_COMPANY_ID);
	fwid.major = img_ver.major;
	fwid.minor = img_ver.minor;
	fwid.revision = sys_cpu_to_le16(img_ver.revision);
	fwid.build_num = sys_cpu_to_le32(img_ver.build_num);

	LOG_INF("Current image version: %u.%u.%u+%u", img_ver.major, img_ver.minor,
		img_ver.revision, img_ver.build_num);

	return 0;
}

SYS_INIT(dfu_target_lib_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

/* Runs after settings_load() has restored the DFU Server state. */
static int dfu_target_lib_commit(void)
{
	int err;

	err = boot_write_img_confirmed();
	if (err) {
		LOG_ERR("Failed to confirm image: %d", err);
	}

	bt_mesh_dfu_srv_applied(&bt_mesh_dfu_target_lib_srv);

	return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(bt_mesh_dfu_tgt, "bt_mesh_dfu_tgt", NULL, NULL,
			       dfu_target_lib_commit, NULL);

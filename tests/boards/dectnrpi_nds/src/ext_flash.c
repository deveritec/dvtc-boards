/*
 * Copyright (c) 2026 Deveritec GmbH
 * SPDX-License-Identifier: Apache-2.0
 *
 * External SPI NOR flash (W25Q32, spi3) test for dectnrpi_nds.
 * Uses the partition external_flash_partition for erase/write/read.
 */

#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/ztest.h>

#define EXT_FLASH_SPI_NODE DT_NODELABEL(w25q32)

#define SECTOR_SIZE 4096
#define CHUNK_SIZE  256

/* W25Q32 JEDEC ID: manufacturer 0xEF (Winbond), memory type 0x40, capacity 0x16 (32 Mbit) */
#define JEDEC_MFR  0xEF
#define JEDEC_TYPE 0x40
#define JEDEC_CAP  0x16

static const struct spi_dt_spec ext_flash_spi = SPI_DT_SPEC_GET(
	EXT_FLASH_SPI_NODE, SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB);

static const struct flash_area *ext_fa;

static void *ext_flash_setup(void)
{
	int ret;

	zassert_true(spi_is_ready_dt(&ext_flash_spi), "SPI bus not ready");

	ret = flash_area_open(DT_FIXED_PARTITION_ID(DT_NODELABEL(external_flash_partition)),
			      &ext_fa);
	zassert_equal(0, ret, "flash_area_open(external_flash_partition) failed (%d)", ret);

	zassert_true(ext_fa->fa_size >= SECTOR_SIZE, "partition too small");

	return NULL;
}

ZTEST(ext_flash, test_jedec_id)
{
	uint8_t tx[4] = {0x9F, 0x00, 0x00, 0x00};
	uint8_t rx[4] = {0x00, 0x00, 0x00, 0x00};
	struct spi_buf txb = {.buf = tx, .len = sizeof(tx)};
	struct spi_buf rxb = {.buf = rx, .len = sizeof(rx)};
	struct spi_buf_set txs = {.buffers = &txb, .count = 1};
	struct spi_buf_set rxs = {.buffers = &rxb, .count = 1};
	int ret;

	ret = spi_transceive_dt(&ext_flash_spi, &txs, &rxs);
	zassert_equal(0, ret, "JEDEC ID read failed (%d)", ret);

	printk("JEDEC ID: %02x %02x %02x\n", rx[1], rx[2], rx[3]);

	zassert_equal(JEDEC_MFR, (int)rx[1], "manufacturer ID mismatch");
	zassert_equal(JEDEC_TYPE, (int)rx[2], "memory type mismatch");
	zassert_equal(JEDEC_CAP, (int)rx[3], "capacity mismatch");
}

ZTEST(ext_flash, test_erase_write_read)
{
	static uint8_t wbuf[CHUNK_SIZE];
	static uint8_t rbuf[CHUNK_SIZE];
	int ret;

	ret = flash_area_erase(ext_fa, 0, SECTOR_SIZE);
	zassert_equal(0, ret, "erase failed (%d)", ret);

	for (uint32_t off = 0; off < SECTOR_SIZE; off += CHUNK_SIZE) {
		ret = flash_area_read(ext_fa, off, rbuf, sizeof(rbuf));
		zassert_equal(0, ret, "read after erase failed at 0x%x (%d)", off, ret);

		for (size_t j = 0; j < sizeof(rbuf); j++) {
			zassert_equal(0xFF, (int)rbuf[j], "not erased at 0x%x", off + j);
		}
	}

	for (uint32_t off = 0; off < SECTOR_SIZE; off += CHUNK_SIZE) {
		for (size_t j = 0; j < sizeof(wbuf); j++) {
			wbuf[j] = (uint8_t)((off + j) ^ 0xA5);
		}

		ret = flash_area_write(ext_fa, off, wbuf, sizeof(wbuf));
		zassert_equal(0, ret, "write failed at 0x%x (%d)", off, ret);

		ret = flash_area_read(ext_fa, off, rbuf, sizeof(rbuf));
		zassert_equal(0, ret, "read failed at 0x%x (%d)", off, ret);

		zassert_mem_equal(wbuf, rbuf, sizeof(wbuf), "data mismatch at 0x%x", off);
	}

	printk("ext flash: erase/write/read of %d bytes OK\n", SECTOR_SIZE);
}

ZTEST_SUITE(ext_flash, NULL, ext_flash_setup, NULL, NULL, NULL);
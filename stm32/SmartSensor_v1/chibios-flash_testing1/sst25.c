/**
 * @file       sst25.c
 * @brief      FLASH25 SST25 driver
 * @author     Vladimir Ermakov Copyright (C) 2014.
 * @see        The GNU Public License (GPL) Version 3
 *
 * Based on 25xx.c from ChibiOS-EEPROM written by Timon Wong
 * And sst25.c from NuttX written by Gregory Nutt <gnutt@nuttx.org>
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "sst25.h"

/*
 * Configuration
 */

#if !defined(SST25_FAST_READ) && !defined(SST25_SLOW_READ)
/* Use slow (< 25 MHz) reads by default */
#define SST25_SLOW_READ
#endif

#if defined(SST25_FAST_READ) && defined(SST25_SLOW_READ)
#error "Please select only one read method: FAST or SLOW"
#endif

#if !defined(SST25_SLOW_WRITE) && !defined(SST25_FAST_WRITE)
/* Use byte program (not AAI) by default */
#define SST25_SLOW_WRITE
#endif

#if defined(SST25_FAST_WRITE) && defined(SST25_SLOW_WRITE)
#error "Please select only one write method: FAST (AAI) or SLOW"
#endif

/* Defines */

/* SST25 command set */
#define CMD_READ		0x03
#define CMD_FAST_READ		0x0b
#define CMD_ERASE_4K		0x20
#define CMD_ERASE_32K		0x52
#define CMD_ERASE_64K		0xd8
#define CMD_CHIP_ERASE		0x60 /* or 0xc7 */
#define CMD_BYTE_PROG		0x02
#define CMD_AAI_WORD_PROG	0xad
#define CMD_RDSR		0x05
#define CMD_EWSR		0x50
#define CMD_WRSR		0x01
#define CMD_WREN		0x06
#define CMD_WRDI		0x04
#define CMD_RDID		0x90
#define CMD_JDEC_ID		0x9f
#define CMD_EBSY		0x70
#define CMD_DBSY		0x80

/* SST25 status register bits */
#define STAT_BUSY		(1<<0)
#define STAT_WEL		(1<<1)
#define STAT_BP0		(1<<2)
#define STAT_BP1		(1<<3)
#define STAT_BP2		(1<<4)
#define STAT_BP3		(1<<5)
#define STAT_AAI		(1<<6)
#define STAT_BPL		(1<<7)

#define FLASH_TIMEOUT	MS2ST(10)
#define ERASE_TIMEOUT	MS2ST(100)
#define SST25_PAGESZ	256

/*
 * Supported device table
 */

#define INFO(name_, id_, ps_, es_, nr_)		{ name_, id_, ps_, es_, nr_ }
struct sst25_ll_info {
	const char *name;
	uint32_t jdec_id;
	uint16_t page_size;
	uint16_t erase_size;
	uint32_t nr_pages;
};

static const struct sst25_ll_info sst25_ll_info_table[] = {
	INFO("sst25vf016b", 0xbf2541, SST25_PAGESZ, 4096, 16*1024*1024/8/SST25_PAGESZ),
	INFO("sst25vf032b", 0xbf254a, SST25_PAGESZ, 4096, 32*1024*1024/8/SST25_PAGESZ),
    INFO("m25pe16", 0x208015,256,4096,8192),
};

/*
 * Low level flash interface
 */

/**
 * @brief SPI-Flash transfer function
 * @notapi
 */
static void sst25_ll_transfer(const SST25Config *cfg,
		const uint8_t *txbuf, size_t txlen,
		uint8_t *rxbuf, size_t rxlen)
{
#if SPI_USE_MUTUAL_EXCLUSION
	spiAcquireBus(cfg->spip);
#endif

	spiStart(cfg->spip, cfg->spicfg);
	spiSelect(cfg->spip);
	spiSend(cfg->spip, txlen, txbuf);
	if (rxlen)
		spiReceive(cfg->spip, rxlen, rxbuf);
	spiUnselect(cfg->spip);

#if SPI_USE_MUTUAL_EXCLUSION
	spiReleaseBus(cfg->spip);
#endif
}

/**
 * @brief checks busy flag
 * @notapi
 */
static bool_t sst25_ll_is_busy(const SST25Config *cfg)
{
	uint8_t cmd = CMD_RDSR;
	uint8_t stat;

	sst25_ll_transfer(cfg, &cmd, 1, &stat, 1);
	return !!(stat & STAT_BUSY);
}

/**
 * @brief wait write completion
 * @return CH_FAILED if timeout occurs
 * @notapi
 */
static bool_t sst25_ll_wait_complete(const SST25Config *cfg, systime_t timeout)
{
	systime_t now = chTimeNow();
	while (sst25_ll_is_busy(cfg)) {
		if (chTimeElapsedSince(now) >= timeout)
			return CH_FAILED; /* Timeout */

		chThdYield();
	}

	return CH_SUCCESS;
}

/**
 * @brief write status register (disable block protection)
 * @notapi
 */
static void sst25_ll_wrsr(const SST25Config *cfg, uint8_t sr)
{
	uint8_t cmd[2];

	cmd[0] = CMD_EWSR;
	sst25_ll_transfer(cfg, cmd, 1, NULL, 0);

	cmd[0] = CMD_WRSR;
	cmd[1] = sr;
	sst25_ll_transfer(cfg, cmd, 2, NULL, 0);
}

/**
 * @brief read JDEC ID from device
 * @notapi
 */
static uint32_t sst25_ll_get_jdec_id(const SST25Config *cfg)
{
	uint8_t cmd = CMD_JDEC_ID;
	uint8_t jdec[3];

	/* JDEC: 3 bytes */
	sst25_ll_transfer(cfg, &cmd, 1, jdec, sizeof(jdec));
	return (jdec[0] << 16) | (jdec[1] << 8) | jdec[2];
}

/**
 * @brief prepare command with address
 * @notapi
 */
static void sst25_ll_prepare_cmd(uint8_t *buff, uint8_t cmd, uint32_t addr)
{
	buff[0] = cmd;
	buff[1] = (addr >> 16) & 0xff;
	buff[2] = (addr >> 8) & 0xff;
	buff[3] = addr & 0xff;
}

#ifdef SST25_SLOW_READ
/**
 * @brief Normal read (F_clk < 25 MHz)
 * @notapi
 */
static void sst25_ll_read(const SST25Config *cfg, uint32_t addr,
		uint8_t *buffer, uint32_t nbytes)
{
	uint8_t cmd[4];

	sst25_ll_prepare_cmd(cmd, CMD_READ, addr);
	sst25_ll_transfer(cfg, cmd, sizeof(cmd), buffer, nbytes);
}
#endif /* SST25_SLOW_READ */

#ifdef SST25_FAST_READ
/**
 * @brief Fast read (F_clk < 80 MHz)
 * @notapi
 */
static void sst25_ll_fast_read(const SST25Config *cfg, uint32_t addr,
		uint8_t *buffer, uint32_t nbytes)
{
	uint8_t cmd[5];

	sst25_ll_prepare_cmd(cmd, CMD_FAST_READ, addr);
	cmd[4] = 0xa5; /* dummy byte */
	sst25_ll_transfer(cfg, cmd, sizeof(cmd), buffer, nbytes);
}
#endif /* SST25_FAST_READ */

/**
 * @brief Set/Reset write lock
 * @notapi
 */
static void sst25_ll_wrlock(const SST25Config *cfg, bool_t lock)
{
	uint8_t cmd = (lock)? CMD_WRDI : CMD_WREN;
	sst25_ll_transfer(cfg, &cmd, 1, NULL, 0);
}

/**
 * @brief Enables/Disables SO as hw busy pin
 * @notapi
 */
static void sst25_ll_hw_busy(const SST25Config *cfg, bool_t enable)
{
	uint8_t cmd = (enable)? CMD_EBSY : CMD_DBSY;
	sst25_ll_transfer(cfg, &cmd, 1, NULL, 0);
}

#ifdef SST25_SLOW_WRITE
/**
 * @brief Slow write (one byte per cycle)
 * @return CH_FAILED if timeout occurs
 * @notapi
 */
static bool_t sst25_ll_write_byte(const SST25Config *cfg, uint32_t addr,
		const uint8_t *buffer, uint32_t nbytes)
{
	uint8_t cmd[5];
	bool_t ret;

	for (; nbytes > 0; nbytes--, buffer++, addr++) {
		/* skip bytes equal to erased state */
		if (*buffer == 0xff)
			continue;

		sst25_ll_prepare_cmd(cmd, CMD_BYTE_PROG, addr);
		cmd[4] = *buffer;

		sst25_ll_wrlock(cfg, false);
		sst25_ll_transfer(cfg, cmd, sizeof(cmd), NULL, 0);
		ret = sst25_ll_wait_complete(cfg, FLASH_TIMEOUT);
		sst25_ll_wrlock(cfg, true);

		if (ret == CH_FAILED)
			break;
	}

	return ret;
}
#endif /* SST25_SLOW_WRITE */

#ifdef SST25_FAST_WRITE
/**
 * @brief Fast write (word per cycle)
 * Based on sst25.c mtd driver from NuttX
 *
 * @return CH_FAILED if timeout occurs
 * @notapi
 */
static bool_t sst25_ll_write_word(const SST25Config *cfg, uint32_t addr,
		const uint8_t *buff, uint32_t nbytes)
{
	uint32_t nwords = (nbytes + 1) / 2;
	uint8_t cmd[4];

	while (nwords > 0) {
		/* skip words equal to erased state */
		while (nwords > 0 && buff[0] == 0xff && buff[1] == 0xff) {
			nwords--;
			addr += 2;
			buff += 2;
		}

		if (nwords == 0)
			return CH_SUCCESS; /* all data written */

		sst25_ll_prepare_cmd(cmd, CMD_AAI_WORD_PROG, addr);
		sst25_ll_wrlock(cfg, false);

#if SPI_USE_MUTUAL_EXCLUSION
		spiAcquireBus(cfg->spip);
#endif

		spiStart(cfg->spip, cfg->spicfg);
		spiSelect(cfg->spip);
		spiSend(cfg->spip, sizeof(cmd), cmd);
		spiSend(cfg->spip, 2, buff);
		spiUnselect(cfg->spip);

#if SPI_USE_MUTUAL_EXCLUSION
		spiReleaseBus(cfg->spip);
#endif

		if (sst25_ll_wait_complete(cfg, FLASH_TIMEOUT) == CH_FAILED) {
			sst25_ll_wrlock(cfg, true);
			return CH_FAILED;
		}

		nwords--;
		addr += 2;
		buff += 2;

		/* write 16-bit cunks */
		while (nwords > 0 && (buff[0] != 0xff && buff[1] != 0xff)) {
#if SPI_USE_MUTUAL_EXCLUSION
			spiAcquireBus(cfg->spip);
#endif

			spiStart(cfg->spip, cfg->spicfg);
			spiSelect(cfg->spip);
			spiSend(cfg->spip, 1, cmd); /* CMD_AAI_WORD_PROG */
			spiSend(cfg->spip, 2, buff);
			spiUnselect(cfg->spip);

#if SPI_USE_MUTUAL_EXCLUSION
			spiReleaseBus(cfg->spip);
#endif

			if (sst25_ll_wait_complete(cfg, FLASH_TIMEOUT) == CH_FAILED) {
				sst25_ll_wrlock(cfg, true);
				return CH_FAILED;
			}

			nwords--;
			addr += 2;
			buff += 2;
		}

		sst25_ll_wrlock(cfg, true);
	}

	return CH_SUCCESS;
}
#endif /* SST25_FAST_WRITE */

static bool_t sst25_ll_chip_erase(const SST25Config *cfg)
{
	uint8_t cmd = CMD_CHIP_ERASE;
	bool_t ret;

	sst25_ll_wrlock(cfg, false);
	sst25_ll_transfer(cfg, &cmd, 1, NULL, 0);
	ret = sst25_ll_wait_complete(cfg, ERASE_TIMEOUT);
	sst25_ll_wrlock(cfg, true);
	return ret;
}

static bool_t sst25_ll_erase_block(const SST25Config *cfg, uint32_t addr)
{
	uint8_t cmd[4];
	bool_t ret;

	sst25_ll_prepare_cmd(cmd, CMD_ERASE_4K, addr);
	sst25_ll_wrlock(cfg, false);
	sst25_ll_transfer(cfg, cmd, sizeof(cmd), NULL, 0);
	ret = sst25_ll_wait_complete(cfg, ERASE_TIMEOUT);
	sst25_ll_wrlock(cfg, true);
	return ret;
}

/*
 * VMT functions
 */

/**
 * @brief for unused fields of VMT
 * @notapi
 */
static bool_t sst25_vmt_nop(void *instance __attribute__((unused)))
{
	return CH_SUCCESS;
}

/**
 * @brief probe flash chip
 * Select page/erase/size of chip
 * @api
 */
static bool_t sst25_connect(SST25Driver *inst)
{
	const struct sst25_ll_info *ptbl;

	inst->state = BLK_CONNECTING;
	inst->jdec_id = sst25_ll_get_jdec_id(inst->config);

	for (ptbl = sst25_ll_info_table;
			ptbl < (sst25_ll_info_table + ARRAY_SIZE(sst25_ll_info_table));
			ptbl++)
		if (ptbl->jdec_id == inst->jdec_id) {
			inst->state = BLK_ACTIVE;
			inst->name = ptbl->name;
			inst->page_size = ptbl->page_size;
			inst->erase_size = ptbl->erase_size;
			inst->nr_pages = ptbl->nr_pages;

			/* disable write protection BP[0..3] = 0 */
			sst25_ll_hw_busy(inst->config, false);
			sst25_ll_wrsr(inst->config, 0);

			MTD_INFO("sst25: %s: %u * %u erase: %u, total %u kB",
					mtdGetName(inst),
					inst->page_size, inst->nr_pages, inst->erase_size,
					mtdGetSize(inst) / 1024);
			return CH_SUCCESS;
		}

	inst->state = BLK_STOP;
	MTD_DEBUG("sst25: connection failed: JDEC ID 0x%06x", inst->jdec_id);
	return CH_FAILED;
}

/**
 * @brief read blocks from flash
 * @api
 */
static bool_t sst25_read(SST25Driver *inst, uint32_t startblk,
		uint8_t *buffer, uint32_t n)
{
	startblk += inst->start_page;

	uint32_t addr = startblk * inst->page_size;
	uint32_t nbytes = n * inst->page_size;

	chDbgCheck(inst->state == BLK_ACTIVE, "sst25_read()");
	if (n > inst->nr_pages) {
		MTD_DEBUG("sst25: %s: read oversize (%u)", mtdGetName(inst), n);
		return CH_FAILED;
	}

#ifdef SST25_SLOW_READ
	sst25_ll_read(inst->config, addr, buffer, nbytes);
#else /* SST25_FAST_READ */
	sst25_ll_fast_read(inst->config, addr, buffer, nbytes);
#endif
	return CH_SUCCESS;
}

/**
 * @brief writes blocks to flash
 * @api
 */
static bool_t sst25_write(SST25Driver *inst, uint32_t startblk,
		const uint8_t *buffer, uint32_t n)
{
	startblk += inst->start_page;

	uint32_t addr = startblk * inst->page_size;
	uint32_t nbytes = n * inst->page_size;

	chDbgCheck(inst->state == BLK_ACTIVE, "sst25_write()");
	if (n > inst->nr_pages) {
		MTD_DEBUG("sst25: %s: write oversize (%u)", mtdGetName(inst), n);
		return CH_FAILED;
	}

#ifdef SST25_SLOW_WRITE
	return sst25_ll_write_byte(inst->config, addr, buffer, nbytes);
#else /* SST25_FAST_WRITE */
	return sst25_ll_write_word(inst->config, addr, buffer, nbytes);
#endif
}

/**
 * @brief erase blocks on flash
 * If startblk is 0 and n more than chip capacity then erases whole chip.
 *
 * @param[in] startblk start block number
 * @param[in] n block count (must be equal to erase size, eg. for 4096 es, 256 ps -> n % 4096/256)
 * @api
 */
static bool_t sst25_erase(SST25Driver *inst, uint32_t startblk, uint32_t n)
{
	uint32_t addr;
	uint32_t nblocks;
	bool_t ret = CH_FAILED;

	chDbgCheck(inst->state == BLK_ACTIVE, "sst25_erase()");

	startblk += inst->start_page;
	if (startblk == 0 && n >= inst->nr_pages && inst->parent == NULL) {
		MTD_DEBUG("sst25: %s: perform chip erase", mtdGetName(inst));
		return sst25_ll_chip_erase(inst->config);
	}

	/* for partition erase */
	if (n > inst->nr_pages)
		n = inst->nr_pages;

	MTD_DEBUG("sst25: %s: erase [%u..%u], %u pages", mtdGetName(inst),
			startblk - inst->start_page,
			startblk - inst->start_page + n,
			n);
	chDbgAssert((n % (inst->erase_size / inst->page_size)) == 0,
			"sst25_erase()", "invalid size");

	addr = startblk * inst->page_size;
	nblocks = (n + 1) / (inst->erase_size / inst->page_size);
	for (; nblocks > 0; nblocks--, addr += inst->erase_size) {
		ret = sst25_ll_erase_block(inst->config, addr);
		if (ret == CH_FAILED)
			break;
	}

	return ret;
}

/**
 * @brief Get block device info (page size and noumber of pages)
 * @api
 */
static bool_t sst25_get_info(SST25Driver *inst, BlockDeviceInfo *bdip)
{
	if (inst->state != BLK_ACTIVE)
		return CH_FAILED;

	bdip->blk_size = inst->page_size;
	bdip->blk_num = inst->nr_pages;
	return CH_SUCCESS;
}

static const struct BaseMTDDriverVMT sst25_vmt = {
	.is_inserted = sst25_vmt_nop,
	.is_protected = sst25_vmt_nop,
	.connect = (bool_t (*)(void*)) sst25_connect,
	.disconnect = sst25_vmt_nop,
	.read = (bool_t (*)(void*, uint32_t, uint8_t*, uint32_t)) sst25_read,
	.write = (bool_t (*)(void*, uint32_t, const uint8_t*, uint32_t)) sst25_write,
	.sync = sst25_vmt_nop,
	.get_info = (bool_t (*)(void*, BlockDeviceInfo*)) sst25_get_info,
	.erase = (bool_t (*)(void*, uint32_t, uint32_t)) sst25_erase
};

/*
 * public interface
 */

/**
 * @brief SST25 driver initialization.
 *
 * @init
 */
void sst25Init(void)
{
}

/**
 * @brief Initializes an instance.
 *
 * @init
 */
void sst25ObjectInit(SST25Driver *flp)
{
	chDbgCheck(flp != NULL, "sst25ObjectInit");

	flp->vmt = &sst25_vmt;
	flp->config = NULL;
	flp->parent = NULL;
	flp->state = BLK_STOP;
	flp->jdec_id = 0;
	flp->page_size = 0;
	flp->erase_size = 0;
	flp->nr_pages = 0;
	flp->start_page = 0;
}

/**
 * @brief start flash device
 * @api
 */
void sst25Start(SST25Driver *flp, const SST25Config *cfg)
{
	chDbgCheck((flp != NULL) && (cfg != NULL), "sst25Start");
	chDbgAssert((flp->state == BLK_STOP) || (flp->state == BLK_ACTIVE),
			"sst25Start()", "invalid state");

	flp->config = cfg;
	//flp->state = BLK_ACTIVE;
}

/**
 * @brief stops device
 * @api
 */
void sst25Stop(SST25Driver *flp)
{
	chDbgCheck(flp != NULL, "sst25Stop");
	chDbgAssert((flp->state == BLK_STOP) || (flp->state == BLK_ACTIVE),
			"sst25Start()", "invalid state");

	spiStop(flp->config->spip);
	flp->state = BLK_STOP;
}

/**
 * @brief init partition
 * @api
 */
void sst25InitPartition(SST25Driver *flp, SST25Driver *part_flp, const struct mtd_partition *part_def)
{
	chDbgCheck(flp != NULL, "sst25InitPartition");
	chDbgCheck(part_flp != NULL, "sst25InitPartition");
	chDbgAssert((flp->state == BLK_ACTIVE),
			"sst25InitPartition()", "invalid state");

#define FLP_COPY(field)	(part_flp->field) = (flp->field)
	FLP_COPY(vmt);
	FLP_COPY(config);
	FLP_COPY(state);
	FLP_COPY(page_size);
	FLP_COPY(erase_size);
	FLP_COPY(jdec_id);

	part_flp->name = part_def->name;
	part_flp->parent = flp;
	part_flp->start_page = part_def->start_page;

	part_flp->nr_pages = part_def->nr_pages;
	if (part_flp->nr_pages > flp->nr_pages)
		part_flp->nr_pages = flp->nr_pages - part_def->start_page;

	MTD_INFO("sst25: %s/%s: [%u..%u] %u pages, total %u kB",
			mtdGetName(flp), mtdGetName(part_flp),
			part_flp->start_page,
			part_flp->start_page + part_flp->nr_pages,
			part_flp->nr_pages,
			mtdGetSize(part_flp) / 1024);
}

/**
 * @brief init partitons from table
 * @api
 */
void sst25InitPartitionTable(SST25Driver *flp, const struct sst25_partition *part_defs)
{
	const struct sst25_partition *ptbl = NULL;

	chDbgCheck(flp != NULL, "sst25InitPartitionTable");
	chDbgCheck(part_defs != NULL, "sst25InitPartitionTable");

	for (ptbl = part_defs; ptbl->partp != NULL; ptbl++)
		sst25InitPartition(flp, ptbl->partp, &(ptbl->definition));
}


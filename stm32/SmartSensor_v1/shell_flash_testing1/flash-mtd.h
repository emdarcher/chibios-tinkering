/**
 * @file       flash-mtd.h
 * @brief      FLASH25 main include file
 * @author     Vladimir Ermakov Copyright (C) 2014.
 * @see        The GNU Public License (GPL) Version 3
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

#ifndef FLASH_MTD_H
#define FLASH_MTD_H

/* ChibiOS block */
#include "hal.h"
//#include "mtd_config.h"

/* -*- debugging print -*- */

#ifndef MTD_DEBUG
#define MTD_DEBUG(fmt, arg...)
#endif

#ifndef MTD_INFO
#define MTD_INFO(fmt, arg...)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)	(sizeof(arr)/sizeof(arr[0]))
#endif

/** Base MTD driver methods
 */
#define _base_mtd_driver_methods 					\
	_base_block_device_methods 					\
	bool_t (*erase)(void *instance, uint32_t startblk,		\
			uint32_t n);


/** Base MTD driver data
 */
#define _base_mtd_driver_data						\
	_base_block_device_data						\
	const char *name;						\
	void *parent;							\
	uint16_t page_size;						\
	uint16_t erase_size;						\
	uint32_t nr_pages;						\
	uint32_t start_page;


struct BaseMTDDriverVMT {
	_base_mtd_driver_methods
};

struct mtd_partition {
	const char *name;
	uint32_t start_page;
	uint32_t nr_pages;
};

/* -*- methods -*- */

#define mtdGetEraseSize(flp)	((flp)->erase_size)
#define mtdGetPageSize(flp)	((flp)->page_size)
#define mtdGetSize(flp)		((flp)->page_size * (flp)->nr_pages)
#define mtdGetName(flp)		((flp)->name)
#define mtdErase(flp, sect, n)	((flp)->vmt->erase(flp, sect, n))

#include "sst25.h"

#endif /* FLASH25_H */

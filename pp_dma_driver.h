/*
 *   ioctl and DMA driver for Marvell Packet Processor Bobcat2
 *
 * Based on  Linux 2.6 and later 'parrot' sample device driver
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PP_DMA_DEVICE_H
#define _PP_DMA_DEVICE_H

#define DEVICE_NAME "device"
#define CLASS_NAME "pp_dma"
//#define PARROT_MSG_FIFO_SIZE 1024
//#define PARROT_MSG_FIFO_MAX  128

#define AUTHOR "SB"
#define DESCRIPTION "pp_dma driver for bobcat"
#define VERSION "0.1"

/* We'll use our own macros for printk */
#define dbg(format, arg...) do { if (debug) pr_info(CLASS_NAME ": %s: " format , __FUNCTION__ , ## arg); } while (0)
#define err(format, arg...) pr_err(CLASS_NAME ": " format, ## arg)
#define info(format, arg...) pr_info(CLASS_NAME ": " format, ## arg)
#define warn(format, arg...) pr_warn(CLASS_NAME ": " format, ## arg)



/* Device variables */
static struct device* pp_dma_device = NULL;

#define DEVICE_VAR pp_dma_device

#endif

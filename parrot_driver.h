/*
 * Linux 2.6 and later 'parrot' sample device driver
 *
 * Copyright (c) 2011-2015, Pete Batard <pete@akeo.ie>
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

#ifndef _PARROT_DEVICE_H
#define _PARROT_DEVICE_H

#define DEVICE_NAME "device"
#define CLASS_NAME "parrot"
#define PARROT_MSG_FIFO_SIZE 1024
#define PARROT_MSG_FIFO_MAX  128

#define AUTHOR "Pete Batard <pete@akeo.ie>"
#define DESCRIPTION "'parrot' sample device driver"
#define VERSION "1.2"

#include <linux/ioctl.h>


/* We'll use our own macros for printk */
#define dbg(format, arg...) do { if (debug) pr_info(CLASS_NAME ": %s: " format , __FUNCTION__ , ## arg); } while (0)
#define err(format, arg...) pr_err(CLASS_NAME ": " format, ## arg)
#define info(format, arg...) pr_info(CLASS_NAME ": " format, ## arg)
#define warn(format, arg...) pr_warn(CLASS_NAME ": " format, ## arg)


/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define PARROT_IOC_MAGIC  0x82
/* Please use a different 8-bit number in your code */

#define PARROT_IOCRESET    _IO(PARROT_IOC_MAGIC, 0)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 */

#define PARROT_IOCRGETADDR _IOR(PARROT_IOC_MAGIC,  5, unsigned long int)

#define PARROT_IOC_MAXNR 14


/* #define PARROT_IOCSQUANTUM _IOW(PARROT_IOC_MAGIC,  1, int) */
/* #define PARROT_IOCSQSET    _IOW(PARROT_IOC_MAGIC,  2, unsigned long int) */
/* #define PARROT_IOCTQUANTUM _IO(PARROT_IOC_MAGIC,   3) */
/* #define PARROT_IOCTQSET    _IO(PARROT_IOC_MAGIC,   4) */
/* #define PARROT_IOCGQUANTUM _IOR(PARROT_IOC_MAGIC,  5, unsigned long int) */
/* #define PARROT_IOCGQSET    _IOR(PARROT_IOC_MAGIC,  6, int) */
/* #define PARROT_IOCQQUANTUM _IO(PARROT_IOC_MAGIC,   7) */
/* #define PARROT_IOCQQSET    _IO(PARROT_IOC_MAGIC,   8) */
/* #define PARROT_IOCXQUANTUM _IOWR(PARROT_IOC_MAGIC, 9, int) */
/* #define PARROT_IOCXQSET    _IOWR(PARROT_IOC_MAGIC,10, int) */
/* #define PARROT_IOCHQUANTUM _IO(PARROT_IOC_MAGIC,  11) */
/* #define PARROT_IOCHQSET    _IO(PARROT_IOC_MAGIC,  12) */


#endif

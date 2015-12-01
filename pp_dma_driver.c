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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>

#include <linux/dma-mapping.h>

#include "pp_dma_driver.h"


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define _KERNEL_MODULE
#include "device_ioctl.h"


/* Module information */
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL");



static struct class* pp_dma_class = NULL;

static int pp_dma_major;


/* Flag used with the one_shot mode */
static bool message_read;
/* A mutex will ensure that only one process accesses our device */
static DEFINE_MUTEX(pp_dma_device_mutex);

unsigned long int testme = 0;

/* Module parameters that can be provided on insmod */
static bool debug = true;	/* print extra debug info */
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "enable debug info (default: false)");

static int pp_dma_device_open(struct inode* inode, struct file* filp)
{
	/* Ensure that only one process has access to our device at any one time
 	* For more info on concurrent accesses, see http://lwn.net/images/pdf/LDD3/ch05.pdf */
	if (!mutex_trylock(&pp_dma_device_mutex)) {
		warn("another process is accessing the device\n");
		return -EBUSY;
	}

	message_read = false;
	return 0;
}

static int pp_dma_device_close(struct inode* inode, struct file* filp)
{
	//dbg("");
	mutex_unlock(&pp_dma_device_mutex);
	return 0;
}

static ssize_t pp_dma_device_read(struct file* filp, char __user *buffer, size_t length, loff_t* offset)
{
	int retval;
	unsigned int copied;


	return 0;


	//return retval ? retval : copied;
}

/* The file_operation scructure tells the kernel which device operations are handled.
 * For a list of available file operations, see http://lwn.net/images/pdf/LDD3/ch03.pdf */
static struct file_operations fops = {
        .owner =   THIS_MODULE,
	.read =    pp_dma_device_read, 
	.open =    pp_dma_device_open, 
	.release = pp_dma_device_close, 
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	.unlocked_ioctl = IOCTL_FUNCTION,
	.mmap = op_mmap,
};

/* This sysfs entry resets the FIFO */
static ssize_t sys_reset(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
        dbg("");

        /* Ideally, we would have a mutex around the FIFO, to ensure that we don't reset while in use.
         * To keep this sample simple, and because this is a sysfs operation, we don't do that */
	//        kfifo_reset(&parrot_msg_fifo);
        //parrot_msg_idx_rd = parrot_msg_idx_wr = 0;

        return 0;
}

/* Declare the sysfs entries. The macros create instances of dev_attr_fifo and dev_attr_reset */
static DEVICE_ATTR(reset, S_IRWXU|S_IRWXG, NULL, sys_reset);

/* Module initialization and release */
static int __init pp_dma_module_init(void)
{
	int retval;
	dbg("");

	/* First, see if we can dynamically allocate a major for our device */
	pp_dma_major = register_chrdev(0, DEVICE_NAME, &fops);
	if (pp_dma_major < 0) {
	  printk(KERN_INFO "failed to register device: error %d\n", pp_dma_major);
	  retval = pp_dma_major;
	  goto failed_chrdevreg;
	}else{
	  printk(KERN_INFO "Device registered major=%d \n",pp_dma_major);
	}

	/* We can either tie our device to a bus (existing, or one that we create)
	 * or use a "virtual" device class. For this example, we choose the latter */
	pp_dma_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(pp_dma_class)) {
		err("failed to register device class '%s'\n", CLASS_NAME);
		retval = PTR_ERR(pp_dma_class);
		goto failed_classreg;
	}

	/* With a class, the easiest way to instantiate a device is to call device_create() */
	pp_dma_device = device_create(pp_dma_class, NULL, MKDEV(pp_dma_major, 0), NULL, CLASS_NAME "_" DEVICE_NAME,NULL);
	if (IS_ERR(pp_dma_device)) {
		err("failed to create device '%s_%s'\n", CLASS_NAME, DEVICE_NAME);
		retval = PTR_ERR(pp_dma_device);
		goto failed_devreg;
	}


        retval = device_create_file(parrot_device, &dev_attr_reset);
        if (retval < 0) {
                //warn("failed to create reset /sys endpoint - continuing without\n");
        }


	mutex_init(&pp_dma_device_mutex);


	return 0;

failed_devreg:
	class_destroy(pp_dma_class);
failed_classreg:
	unregister_chrdev(pp_dma_major, DEVICE_NAME);
failed_chrdevreg:
	return -1;
}

static void __exit pp_dma_module_exit(void)
{
	dbg("");

        device_remove_file(parrot_device, &dev_attr_reset);
	device_destroy(pp_dma_class, MKDEV(pp_dma_major, 0));
	class_destroy(pp_dma_class);
	unregister_chrdev(pp_dma_major, DEVICE_NAME);
}

/* Let the kernel know the calls for module init and exit */
module_init(pp_dma_module_init);
module_exit(pp_dma_module_exit);

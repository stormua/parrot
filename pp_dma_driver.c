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
#include <linux/cdev.h>
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


static struct cdev *pp_dma_cdev=NULL;
static struct class* pp_dma_class = NULL;
static dev_t pp_dma_dev;

static int pp_dma_major;

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


/* Module initialization and release */
static int __init pp_dma_module_init(void)
{
  int retval;
  dbg("");

/* Alloc a device region */
  retval = alloc_chrdev_region(&pp_dma_dev, 1, 1, DEVICE_NAME);
  if (retval != 0)          /* error */
    goto failed_cdev;

  printk(KERN_INFO "ppdma dev == %d \n", (int) pp_dma_dev);
  
/* Registring */
  pp_dma_cdev = cdev_alloc();
  if (!pp_dma_cdev) 
    goto failed_cdev;

/* Init it! */
  cdev_init(pp_dma_cdev, &fops); 
    
/* Tell the kernel "hey, I'm exist" */
  retval = cdev_add(pp_dma_cdev, pp_dma_dev, 1);
  if (retval < 0) 
    goto cdev_add_out;

/* class */
  pp_dma_class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(pp_dma_class)) {
    printk(KERN_ERR DEVICE_NAME " cant create class %s\n", CLASS_NAME);
    goto class_err;
  }

/* device */
  pp_dma_device = device_create(pp_dma_class, NULL, pp_dma_dev, NULL, DEVICE_NAME);
  if (IS_ERR(pp_dma_device)) {
    printk(KERN_ERR DEVICE_NAME " cant create device %s\n", DEVICE_NAME);
    goto device_err;
  }
 
  mutex_init(&pp_dma_device_mutex);
 
  
  return 0;
  
 device_err:
  device_destroy(pp_dma_class, pp_dma_dev);
 class_err:
  class_unregister(pp_dma_class);
  class_destroy(pp_dma_class);
 cdev_add_out:
  cdev_del(pp_dma_cdev);
 failed_cdev:
  return -1;
}

static void __exit pp_dma_module_exit(void)
{
	dbg("");
        device_destroy(pp_dma_class, pp_dma_dev);
        class_unregister(pp_dma_class);
        class_destroy(pp_dma_class);
        cdev_del(pp_dma_cdev);
	return;
}

/* Let the kernel know the calls for module init and exit */
module_init(pp_dma_module_init);
module_exit(pp_dma_module_exit);

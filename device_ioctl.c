//#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/slab.h>
//#include <linux/fs.h>
//#include <linux/device.h>
#include <linux/types.h>
//#include <linux/mutex.h>
//#include <linux/kfifo.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>

#include <linux/dma-mapping.h>


#include "parrot_driver.h"
#define _KERNEL_MODULE
#include "device_ioctl.h"

unsigned long kv_addr=0;
dma_addr_t dma_handle;
size_t dma_order=0;
size_t dma_size=0;


#define DEBUG

long device_ioctl(struct file* filp,unsigned int cmd, unsigned long arg)
{
  int err = 0;//, tmp;
  //int retval = 0;
  //unsigned long int *addr;
  

  memory_area mem_arg;
#ifdef DEBUG
  printk(KERN_INFO "Incoming to device ioctl...\n");
#endif

  /* extract the type and number bitfields, and don't decode */
  /*   wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok() */
  if (_IOC_TYPE(cmd) != DEVICE_IOC_MAGIC)
    return -ENOTTY;
  if (_IOC_NR(cmd) > DEVICE_IOC_MAXNR)
    return -ENOTTY;

  /*
   * the direction is a bitmask, and VERIFY_WRITE catches R/W
   * transfers. `Type' is user-oriented, while
   * access_ok is kernel-oriented, so the concept of "read" and
   * "write" is reversed
   */
  if (_IOC_DIR(cmd) & _IOC_READ) {
    err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    //printk(KERN_INFO "Warinig  ioctl can't write data to user space...\n");
  } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
    err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    printk(KERN_INFO "Warning  ioctl can't read data from user space...\n");
  }
  //	if (err)
  //		return -1;

  printk(KERN_INFO "sizeof memory area == %d \n",sizeof(memory_area)); 
  printk(KERN_INFO "offsetof area_order == %d \n", (int)offsetof(memory_area,area_order)); 
  printk(KERN_INFO "offsetof area_size == %d \n", (int)offsetof(memory_area,area_size)); 
  printk(KERN_INFO "offsetof area == %d \n", (int)offsetof(memory_area,area)); 
  printk(KERN_INFO "offsetof ph_area == %d \n", (int)offsetof(memory_area,ph_area)); 



  copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));

  switch(cmd){
  case DEVICE_IOC_GETDMA:

    dma_order=mem_arg.area_order;
#ifdef DEBUG
    printk(KERN_INFO "Ask kernel  for  %d pages", 2<<dma_order);
#endif

    kv_addr=__get_free_pages(0,dma_order);
#ifdef DEBUG
    printk(KERN_INFO "Get from kernel at %lx", (unsigned long)kv_addr);
#endif

    if(kv_addr!=0){
      dma_size=(2<<dma_order)*PAGE_SIZE;
      dma_handle=dma_map_single( DEVICE_VAR ,(void *)kv_addr,dma_size,DMA_BIDIRECTIONAL);
      mem_arg.area=kv_addr;
      mem_arg.area_size=dma_size;
      mem_arg.ph_area=__phys_addr(kv_addr);


#ifdef DEBUG
      printk(KERN_INFO "ioctl asks for %d bytes  kv_addr==0x%lx, saves to 0x%lx", (unsigned int)dma_size, (unsigned long)kv_addr, __phys_addr(kv_addr));
#endif
      copy_to_user((memory_area __user*)arg,&mem_arg,sizeof(memory_area));
    }else{
      printk(KERN_INFO "get NULL from __get_free_pages(0,%d)\n",dma_order);
    }
    break;

  case DEVICE_IOC_CLEARDMA:
    if(mem_arg.area!=0){
#ifdef DEBUG
      printk(KERN_INFO "Try to clear %d pages\n",2<<mem_arg.area_order);
#endif
      
      dma_unmap_single( DEVICE_VAR ,dma_handle,mem_arg.area_size,DMA_BIDIRECTIONAL);
      free_pages((unsigned long)mem_arg.area, mem_arg.area_order);
    }
    break;
  case  DEVICE_IOC_MAPAREA:


    break;
  case  DEVICE_IOC_UNMAPAREA:

    break;
  default:
    return -1;
  }

  return 0;
}


/*
 * open and close: just keep track of how many times the device is
 * mapped, to avoid releasing it.
 */

void scullp_vma_open(struct vm_area_struct *vma)
{
	struct scullp_dev *dev = vma->vm_private_data;

	dev->vmas++;
}

void scullp_vma_close(struct vm_area_struct *vma)
{
	struct scullp_dev *dev = vma->vm_private_data;

	dev->vmas--;
}

/*
 * The nopage method: the core of the file. It retrieves the
 * page required from the scullp device and returns it to the
 * user. The count for the page must be incremented, because
 * it is automatically decremented at page unmap.
 *
 * For this reason, "order" must be zero. Otherwise, only the first
 * page has its count incremented, and the allocating module must
 * release it as a whole block. Therefore, it isn't possible to map
 * pages from a multipage block: when they are unmapped, their count
 * is individually decreased, and would drop to 0.
 */

struct page *device_vma_nopage(struct vm_area_struct *vma,
                                unsigned long address, int *type)
{
	unsigned long offset;
	struct scullp_dev *ptr, *dev = vma->vm_private_data;
	struct page *page = NOPAGE_SIGBUS;
	void *pageptr = NULL; /* default to "missing" */

	down(&dev->sem);
	offset = (address - vma->vm_start) + (vma->vm_pgoff << PAGE_SHIFT);
	if (offset >= dev->size) goto out; /* out of range */

	/*
	 * Now retrieve the scullp device from the list,then the page.
	 * If the device has holes, the process receives a SIGBUS when
	 * accessing the hole.
	 */
	offset >>= PAGE_SHIFT; /* offset is a number of pages */
	for (ptr = dev; ptr && offset >= dev->qset;) {
		ptr = ptr->next;
		offset -= dev->qset;
	}
	if (ptr && ptr->data) pageptr = ptr->data[offset];
	if (!pageptr) goto out; /* hole or end-of-file */
	page = virt_to_page(pageptr);

	/* got it, now increment the count */
	get_page(page);
	if (type)
		*type = VM_FAULT_MINOR;
  out:
	up(&dev->sem);
	return page;
}



struct vm_operations_struct scullp_vm_ops = {
	.open =     scullp_vma_open,
	.close =    scullp_vma_close,
	.nopage =   scullp_vma_nopage,
};


int scullp_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct inode *inode = filp->f_dentry->d_inode;

	/* refuse to map if order is not 0 */
	if (scullp_devices[iminor(inode)].order)
		return -ENODEV;

	/* don't do anything here: "nopage" will set up page table entries */
	vma->vm_ops = &scullp_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = filp->private_data;
	scullp_vma_open(vma);
	return 0;
}




#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
//#include <linux/device.h>
#include <linux/types.h>
//#include <linux/mutex.h>
//#include <linux/kfifo.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>

#include <linux/dma-mapping.h>


#include "ppdma_driver.h"
#define _KERNEL_MODULE
#include "device_ioctl.h"

unsigned long kv_addr=0;
dma_addr_t dma_handle;
size_t dma_order=0;
size_t dma_size=0;




#ifndef VM_RESERVED
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif



struct mmap_info
{
  char *data;
  unsigned long size;
  unsigned int order;
  int reference;      
};



#define DEBUG


/* void mmap_open(struct vm_area_struct *vma) */
/* { */
/*     struct mmap_info *info = (struct mmap_info *)vma->vm_private_data; */
/*     info->reference++; */
/* } */
 
/* void mmap_close(struct vm_area_struct *vma) */
/* { */
/*     struct mmap_info *info = (struct mmap_info *)vma->vm_private_data; */
/*     info->reference--; */
/* } */
 
/* static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf) */
/* { */
/*     struct page *page; */
/*     struct mmap_info *info;     */
     
/*     info = (struct mmap_info *)vma->vm_private_data; */
/*     if (!info->data) */
/*     { */
/*         printk("No data\n"); */
/*         return 0;     */
/*     } */
     
/*     page = virt_to_page(info->data);     */
     
/*     get_page(page); */
/*     vmf->page = page;             */
     
/*     return 0; */
/* } */
 
/* struct vm_operations_struct mmap_vm_ops = */
/* { */
/*     .open =     mmap_open, */
/*     .close =    mmap_close, */
/*     .fault =    mmap_fault,     */
/* }; */


 
/* int op_mmap(struct file *filp, struct vm_area_struct *vma) */
/* { */
/*     vma->vm_ops = &mmap_vm_ops; */
/*     vma->vm_flags |= VM_RESERVED;     */
/*     vma->vm_private_data = filp->private_data; */
/*     mmap_open(vma); */
/*     return 0; */
/* } */

int op_mmap(struct file *filp, struct vm_area_struct *vma)
{
  unsigned long pfn;
  unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
  unsigned long len = vma->vm_end - vma->vm_start;


  struct mmap_info *info;
  info=filp->private_data;

  printk(KERN_ALERT "\nRAGHU: pcie-axi: Inside pcie_MMAP offset==%ld len=%ld ",offset,len);

  if (offset >= PAGE_SIZE)
    return -EINVAL;

  // if (len > (PAGE_SIZE - offset))
  //  return -EINVAL;

  vma->vm_flags |= VM_RESERVED;
  vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

  printk(KERN_ALERT "\nRAGHU: pcie-axi:  mapping %ld bytes of mem at offset %ld\n", len, offset);

  /* need to get the pfn for remap_pfn_range --
     ds->wr_buf_ptr is the virtual pointer returned from pci_alloc_consistent*/

  pfn = virt_to_phys(info->data + offset) >> PAGE_SHIFT;
  /* In Mmap */
  // io_remap_pfn_range(vma, vma->vm_start, (paddr + offset) >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);
  if (remap_pfn_range(vma, vma->vm_start, pfn, len, vma->vm_page_prot)) {
    printk(KERN_ALERT "remap return error");
    return -EAGAIN;
  }

  printk(KERN_ALERT "\nRAGHU: pcie-axi: Exit pcie_MMAP");
  return 0;

}




long device_ioctl(struct file* filp,unsigned int cmd, unsigned long arg)
{
  int err = 0;//, tmp;
  //int retval = 0;
  //unsigned long int *addr;
  unsigned long jiff_tmp;

  memory_area mem_arg;
  struct mmap_info *info;
  char str[80];
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




  switch(cmd){
  case DEVICE_IOC_GETDMA:
    copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));
    dma_order=mem_arg.area_order;
#ifdef DEBUG
    printk(KERN_INFO "Ask kernel  for  %d pages", 1<<dma_order);
#endif
    kv_addr=__get_free_pages(0,dma_order);
#ifdef DEBUG
    printk(KERN_INFO "Get from kernel at %lx", (unsigned long)kv_addr);
#endif
    if(kv_addr!=0){
      dma_size=(1<<dma_order)*PAGE_SIZE;

      info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);    
      info->data = (char *)kv_addr;
      info->size=dma_size;
      info->order=dma_order;
      filp->private_data = info;
      dma_handle=dma_map_single( DEVICE_VAR ,(void *)kv_addr,dma_size,DMA_BIDIRECTIONAL);
      mem_arg.area=kv_addr;
      mem_arg.area_size=dma_size;
      mem_arg.ph_area=__phys_addr(kv_addr);
      //      mem_arg.dma_area=dma_handle;
#ifdef DEBUG
      printk(KERN_INFO "ioctl asks for %d bytes  kv_addr==0x%lx, saves to 0x%lx", (unsigned int)dma_size, (unsigned long)kv_addr, __phys_addr(kv_addr));
#endif
      copy_to_user((memory_area __user*)arg,&mem_arg,sizeof(memory_area));
    }else{
      printk(KERN_INFO "get NULL from __get_free_pages(0,%d)\n",dma_order);
    }
    break;

  case DEVICE_IOC_CLEARDMA:
    copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));
    if(mem_arg.area!=0){
#ifdef DEBUG
      printk(KERN_INFO "Try to clear %d pages\n",1<<mem_arg.area_order);
#endif
      info=filp->private_data;

      dma_unmap_single( DEVICE_VAR, dma_handle, mem_arg.area_size, DMA_BIDIRECTIONAL);
      free_pages(info->data, info->order);
      kfree(info);
      filp->private_data=NULL;
    }
    break;

  case  DEVICE_IOC_MAPAREA:
    //    copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));
    info=filp->private_data;
    strncpy(str,info->data,79);
    printk(KERN_INFO "area==_%s_\n",str);
    break;

  case  DEVICE_IOC_UNMAPAREA:
    copy_from_user(&mem_arg,(memory_area __user *)arg,sizeof(memory_area));
    break;

  case  DEVICE_TEST_DATA:
    info=filp->private_data;
    memcpy(info->data, "hello from kernel this is file: ", 32);
    memcpy(info->data + 32, filp->f_dentry->d_name.name, strlen(filp->f_dentry->d_name.name));
    break;

  case DEVICE_IOC_GETJIFFIES:
    jiff_tmp=jiffies;
    *(unsigned long *)arg=jiff_tmp;
    printk(KERN_INFO "Jiffy == %ld\n",jiff_tmp);
    break;
  default:
    return -1;
  }

  return 0;
}



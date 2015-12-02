

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <stddef.h>
#include <sys/mman.h>

#include "ppdma_driver.h"
#include "device_ioctl.h"


int
main()
{
  int fd; int i;
  unsigned err=0;
  unsigned long int test1=0x12;
  char * address = NULL;

  
   fd=open("/dev/pp_dma_device",O_RDWR);
   if(fd==-1){
     fprintf(stderr, "Cannot open file");
     exit(1);
     
   }
   
   
   printf("Default value of test1 == %ld \n",test1);
   
   err=ioctl(fd, DEVICE_IOC_GETJIFFIES, &test1);
   if(err!=0){
     printf("Get error %d\n",err);
   }
   
   printf("Get jiffy== %ld \n",test1);
   
   
  close(fd);

}


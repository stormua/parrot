

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



#include "parrot_driver.h"


int
main()
{
  int fd;
  int err=0;
  long int test1=0x12;
  memory_area test_dma;

  fd=open("/dev/parrot_device",O_RDONLY);
  if(fd==-1){
    fprintf(stderr, "Cannot open file");
    exit(1);
    
  }

  test_dma.area=&test1;
  test_dma.is_placed=0;
  test_dma.area_size=4*1024*1024;

  printf("Ask for area %ld bytes\n",(unsigned long)test_dma.area_size);

  err=ioctl(fd,PARROT_IOGETDMA, &test_dma);
  if(err!=0){
    printf("Get error %d\n",err);
  }


  printf("Get from kernel log addr 0x%lx phaddr 0x%lx\n",(unsigned long)test_dma.area, (unsigned long)test_dma.ph_area);
  err=ioctl(fd,PARROT_IOCLEARDMA, &test_dma);

  close(fd);

}


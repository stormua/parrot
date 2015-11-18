

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


  fd=open("/dev/parrot_device",O_RDONLY);
  if(fd==-1){
    fprintf(stderr, "Cannot open file");
    exit(1);
    
  }


  err=ioctl(fd,PARROT_IOCRGETADDR, &test1);
  if(err!=0){
    printf("Get error %d\n",err);
  }


  printf("Get addr 0x%lx\n",test1);


  close(fd);

}


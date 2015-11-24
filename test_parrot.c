

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <stddef.h>
#include <sys/mman.h>

#include "parrot_driver.h"
#include "device_ioctl.h"


int
main()
{
  int fd;
  int err=0;
  long int test1=0x12;
  memory_area test_dma;
  
  memset((void *)&test_dma,0,sizeof(memory_area));
  
   fd=open("/dev/parrot_device",O_RDONLY);
   if(fd==-1){
     fprintf(stderr, "Cannot open file");
     exit(1);
     
   }
   
   // test_dma.area=&test1;
   //  test_dma.is_placed=0;
   test_dma.area_order=2;
   
   printf( "sizeof memory area == %ld \n",sizeof(memory_area)); 
   printf( "offsetof area_order == %d \n", (int)offsetof(memory_area,area_order)); 
   printf( "offsetof area_size == %d \n", (int)offsetof(memory_area,area_size)); 
   printf( "offsetof area == %d \n", (int)offsetof(memory_area,area)); 
   printf( "offsetof ph_area == %d \n", (int)offsetof(memory_area,ph_area)); 
   
   
   
   printf("Ask for area %d pages\n",2<<test_dma.area_order);
   
   err=ioctl(fd,DEVICE_IOC_GETDMA, &test_dma);
   if(err!=0){
     printf("Get error %d\n",err);
   }
   
   printf("Get area %d bytes \n",(int)test_dma.area_size);
   
   
   printf("Get from kernel size %d bytes log addr 0x%lx  phys addr == %lx\n", (int)test_dma.area_size, test_dma.area,  test_dma.ph_area);
 
   
   
   int _fdmem; int i;
   int *map = NULL;
   const char memDevice[] = "/dev/mem";
   
/* open /dev/mem and error checking */
   _fdmem = open( memDevice, O_RDWR | O_SYNC );
   
   if (_fdmem < 0){
     printf("Failed to open the /dev/mem !\n");
     return 0;
   }
   else{
     printf("open /dev/mem successfully !\n");
   }

   /* mmap() the opened /dev/mem */
   map= (int *)(mmap(0,test_dma.area_size,PROT_READ|PROT_WRITE,MAP_SHARED,_fdmem,test_dma.ph_area));

   if(map==(void *)-1){
     perror("mmap returns");
   }else{

   fprintf(stderr,"mmaped\n");
   fprintf(stderr, "%x",*map);
   /* use 'map' pointer to access the mapped area! */
   for (i=0;i<10;i++){
     printf("content: 0x%x\n",*(map+i));
   }

   /* unmap the area & error checking */
   if (munmap(map,test_dma.area_size)==-1){
     perror("Error un-mmapping the file");
   }
   }
   /* close the character device */
   close(_fdmem);
























   err=ioctl(fd,DEVICE_IOC_CLEARDMA, &test_dma);

  close(fd);

}


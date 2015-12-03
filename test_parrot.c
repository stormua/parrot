

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <asm/types.h>

#include "ppdma_driver.h"
#include "device_ioctl.h"


#define FILL_MODULE 251

int
main()
{
  int fd; int i;
  unsigned err=0;
  long int test1=0x12;
  memory_area test_dma;
  char * address = NULL;
  
  memset((void *)&test_dma,0,sizeof(memory_area));
  
   fd=open("/dev/pp_dma_device",O_RDWR);
   if(fd==-1){
     fprintf(stderr, "Cannot open file");
     exit(1);
     
   }
   
   // test_dma.area=&test1;
   //  test_dma.is_placed=0;
   test_dma.area_order=10;
   
   printf("Ask for area %d pages\n",1<<test_dma.area_order);
   
   err=ioctl(fd,DEVICE_IOC_GETDMA, &test_dma);
   if(err!=0){
     printf("Get error %d\n",err);
   }
   
   printf("Get area %d bytes \n",(int)test_dma.area_size);
   
   
   printf("Get from kernel size %d bytes log addr 0x%lx  phys addr == %lx\n", (int)test_dma.area_size, test_dma.area,  test_dma.ph_area);
 

   address = mmap(NULL, test_dma.area_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);


   if(address==(void *) -1)
     {
       perror("Cant map addr");
     }else{
   
     fprintf(stderr,"mmaped\n");
     fprintf(stderr, "%x",*address);
     fprintf(stderr, "\n");

     printf("raw\n");
     /* use 'map' pointer to access the mapped area! */
     for (i=0;i<10;i++){
       printf("content: 0x%x\n",*(address+i));
     }

     printf("set\n");
     for(i=0;i<test_dma.area_size;i++){
       int a=i % FILL_MODULE;
       *(address+i)=a;
       printf("to addr+%d write %x read %x\n",i,a, (char)*(address+i)&0xff);
     }
     memcpy(address, "ByeByeBye\0", 10);
     for (i=0;i<10;i++){
       printf("content: 0x%x -- %c \n",*(address+i),*(address+i));
     }

     printf("Size of addr=%d\n",sizeof(*address));
     /* use 'map' pointer to access the mapped area! */
     for (i=0;i<10;i++){
       printf("content: 0x%x\n",*(address+i));
     }

     for(i=0;i<test_dma.area_size;i++){
       if( *(address+i) != (char)(i % FILL_MODULE)){	 
	 fprintf(stdout,"IO error at offset %d 0x%x != 0x%x (%d) \n",i,*(address+i),i % FILL_MODULE, i%FILL_MODULE  );
       }
     }
    
     
     /* err=ioctl(fd,DEVICE_TEST_DATA, &test_dma); */
     /* if(err!=0){ */
     /*   printf("Get error %d\n",err); */
     /* } */

     err=ioctl(fd,DEVICE_IOC_MAPAREA, &test_dma);
     if(err!=0){
       printf("Get error %d\n",err);
     }

     /* for (i=0;i<10;i++){ */
     /*   printf("content: 0x%x %c\n",*(address+i),(char)(*(address+i))  ); */
     /* } */

     /* memcpy(address, "ByeByeBye\0", 10); */
     /* for (i=0;i<10;i++){ */
     /*   printf("content: 0x%x %c\n",*(address+i),(char)(*(address+i))  ); */
     /* } */


     /* err=ioctl(fd,DEVICE_IOC_MAPAREA, &test_dma); */
     /* if(err!=0){ */
     /*   printf("Get error %d\n",err); */
     /* } */

     /* unmap the area & error checking */
     if (munmap(address,test_dma.area_size)==-1){
       perror("Error un-mmapping the file");
     }
   }
   /* close the character device */

   err=ioctl(fd,DEVICE_IOC_CLEARDMA, &test_dma);

  close(fd);

}


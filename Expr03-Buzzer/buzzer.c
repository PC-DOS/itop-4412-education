#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#define MAX 3
void main(void)
{
	int fd,BuzzerOnOffTimes;
	char *buzzer = "/dev/buzzer_ctl";
	BuzzerOnOffTimes = MAX;
	
	printf("buzzer light on and off 5 times \r\n");
	
	if((fd = open(buzzer, O_RDWR|O_NOCTTY|O_NDELAY))<0)
	printf("open %s failed\n",buzzer);   
	else
	{	
    	printf("open %s success\r\n",buzzer);
		while(BuzzerOnOffTimes--)
		{
			printf("ioctl buzzer %d times\n",BuzzerOnOffTimes);
			ioctl(fd,1);	//parameter 2 is cmd ,cmd = 1 buzzer on			
			sleep(1);
			ioctl(fd,0);
			sleep(1);		
		}
    }
	close(fd);
}
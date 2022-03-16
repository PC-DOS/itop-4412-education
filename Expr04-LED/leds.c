#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#define MAX 10
void main(void)
{
	int fd,LedOnOffTimes;
	char gpio[MAX],cmd[MAX];
	char *leds = "/dev/leds";
	LedOnOffTimes = MAX;
	
	printf("leds light on and off 5 times \r\n");

	
	if((fd = open(leds, O_RDWR|O_NOCTTY|O_NDELAY))<0)
	printf("open %s failed\n",leds);   
	else
	{	
    	printf("open %s success\r\n",leds);
		while(LedOnOffTimes--)
		{
			printf("ioctl leds %d times\n",LedOnOffTimes);
			ioctl(fd,0,0);	//parameter 2 is cmd ,cmd = 1 leds on
			ioctl(fd,0,1);
			sleep(1);
			ioctl(fd,1,0);
			ioctl(fd,1,1);
			sleep(1);		
		}
    }
	close(fd);
}

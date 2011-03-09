#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include <linux/input.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define BUTTON_DEV "/dev/input/event1"

int main()
{
	int fd;
	int count;
	struct input_event ev_button;

	if((fd = open(BUTTON_DEV, O_RDONLY)) < 0)
		return -1;

	while(1)
	{
		if((count = read(fd, &ev_button, sizeof(struct input_event))) < 0)
			return -1;
		//sleep();

		if(EV_KEY == ev_button.type) {
			printf("time:%ld.%ld\n",ev_button.time.tv_sec,ev_button.time.tv_usec);
			printf("type:%d, code:%d, value:%d\n", ev_button.type,ev_button.code, ev_button.value);
		}
	}

	
	return 0;
}


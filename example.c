#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<linux/unistd.h>
#include"lookpkt.h"

int main(void)
{
	int opt, ret;
	char buf[1024];
	void * arrays[5];
	int len = 5;

	arrays[1] = buf;

	while(1)
	{
		printf("0. get size, 1. remove one\n");
		printf("2. eth header, 3. net header, 4. tcp header, 5. udp header\n");
		printf("choose one: ");
		scanf("%d", &opt);	
	
		arrays[0] = (void*)opt;	
		memset(buf, 0, 1024);

		if(opt < 0)
			break;
		else if(opt > 5)
			break;

		ret = syscall(__NR_look_pkt, arrays, len);
		switch(opt)
		{
		case GET_SIZE:
			printf("%d\n", ret);
			break;
		default:
			printf("%s\n", buf);
		}
		puts("");
	}
	
	return 0;
}
			

#include <stdio.h>

void init_fn(){
	printf("MY INIT\n");
	
	int ret = sched_fork();
	fprintf(stderr, "%i\n", ret);
	switch(ret){
		case 0:
			printf("CHILD!\n");
			int j;
			while(j<1000000000) j++;
			//sched_switch();
			break;
		default:
			printf("PARENT!\n");
			//int i;
			//while(i<1000000000) i++;
			//sched_switch();
			break;
	}
	switch(sched_fork()){
		case 0:
			printf("CHILD TWO\n");
			int j=0;
			while(j<1000000000) j++;
			break;
		default:
			printf("PARENT TWO\n");
			int i =0;
			while(i<1000000000) i++;
			break;
	}
fprintf(stderr, "NOOO\n");
}

int main(int argc, char const *argv[])
{
	sched_init(init_fn);

	//while(1) ;
	return 0;
}
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/times.h>
#include <signal.h>

#define FIFO "bankFifo"
#define FIFO2 "clientFifo"

sig_atomic_t readF = 0;

void signalHandler(int sig)
{
	readF = 1;
}

int main(int argc, char const *argv[])
{
	//char myfifo[10] = "bankfifo";
	struct sigaction sa;
	sa.sa_handler = &signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		exit(1);
	char * temp;
	int musteriSayisi = strtol(argv[1],&temp,10);
	//int musteriSayisi = 10;
	int father = getpid();
	int money;
	int pid[musteriSayisi];

	int fd2 = open(FIFO2,O_WRONLY);
	for (int i = 0; i < musteriSayisi; ++i)
	{
		pid[i] = fork();
		if (pid[i] == 0)
		{
			break;
		}
	}


	if(getpid() != father)
	{
		int p = getpid();
		write(fd2,&p,sizeof(int));
	}
	unlink(FIFO2);
	close(fd2);
	
	
	int fd = open(FIFO,O_RDONLY);
	if (getpid() != father)
	{
		// fifodan money al.
		while(!readF);
		if(readF)
			read(fd,&money,sizeof(money));
		if (money == -123)
		{
			printf("Musteri %d parasini alamadı. \n", getpid());
			kill(getpid(),SIGKILL);
		}
		else{
			// baska fifoya pid leri yaz. 
			printf("Musteri %d : %d kadar para aldi. :)\n",getpid(),money );
		}
	}	
	else
	{
		while(wait(NULL)>0);

		for (int i = 0; i < musteriSayisi; ++i)
		{
			kill(pid[i],SIGKILL);
		}
	}

	close(fd);

	unlink(FIFO);
	//while(1);
	//printf("pidler %d\n",getpid() );

	return 1;
}
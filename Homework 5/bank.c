#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/times.h>
#include <stdbool.h> 
#include <sys/time.h>

sig_atomic_t closeBank = 0;
sig_atomic_t generateNum = 0;

#define FIFO "bankFifo"
#define FIFO2 "clientFifo"

/*	musteriyi 1500ms beklet. sonra 0 100 arası bir sayi ver. 
	ekrana müsteri pid ve sayıyı bas.
*/

struct MusteriBilet
{
	int para;
	int pid;
};

int bank()
{
	
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

    srand((time_t)ts.tv_nsec);
	int r = rand() % 100;
	if (r <= 0 )
		r = rand() % 100;
	
	//fclose(f);
	return r;
}

int clientPid[100];

static void signalHandler(int sig)
{
	closeBank = 1;
}

void signalHandler2(int sig)
{
	generateNum = 1;
}

int main(int argc, char const *argv[])
{
	FILE * f = fopen("Banka.log","w");
	if (f == NULL)
	{
		printf("error\n");
	}
	struct sigaction sa;
	sa.sa_handler = &signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	int pipeBank[2];
	pipe(pipeBank);

	struct itimerval itv;
	char *temp;
	int musteriCounter=0;
	int TIMEOUT = strtol(argv[1],&temp,10);
	
	int pid[4];
	bool busy[4] = {true, true, true, true};

	itv.it_value.tv_sec = TIMEOUT;
	itv.it_value.tv_usec = 0;
	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = 0;

	if (mkfifo(FIFO,0666) == -1)
	{
		perror("Fifo Error");
		exit(1);
	}

	if (mkfifo(FIFO2,0666) == -1)
	{
		perror("Fifo Error");
		exit(1);
	}

	if (sigaction(SIGALRM, &sa, NULL) == -1)
		exit(1);

	int t=0;
	while(t < 100) 
	{
		clientPid[t] = -1;
		t++;
	}
	fprintf(f, "26 Nisan 2019 tarihinde islem başladı. Bankamız %d saniye hizmet verecek.\n", TIMEOUT);
	fprintf(f, "%s  %s  %s  %s\n", "clientPid", "processNo" ,"Para", "islem bitis zamanı");
	fflush(f);
	int fd2 = open(FIFO2,O_RDONLY);
	while(read(fd2,&clientPid[musteriCounter],sizeof(int)) > 0){
		musteriCounter++;
	}
	
	/*
	for (int i = 0; i < musteriCounter; ++i)
	{
		printf("müşteriler %d \n", clientPid[i]);
	}
*/
	unlink(FIFO2);
	close(fd2);
	

	int fd = open(FIFO,O_WRONLY);

	int father = getpid();
	for (int i = 0; i < 4; ++i)
	{
		pid[i] = fork();
		if (pid[i] == 0)
		{
			break;
		}
	}
	struct MusteriBilet mb;
	if (getpid() == father)
	{
		setitimer(ITIMER_REAL, &itv, 0);
	}
	else{
		struct sigaction sa;
		sa.sa_handler = &signalHandler2;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		struct itimerval itChild;
		itChild.it_value.tv_sec = 1;
		itChild.it_value.tv_usec = 500000;
		itChild.it_interval.tv_sec = 1;
		itChild.it_interval.tv_usec = 1;

		if (sigaction(SIGALRM, &sa, NULL) == -1)
			exit(1);

		setitimer(ITIMER_REAL,&itChild,0);
	}

	int money=0,i=0,processCounter=0,j=0,m=0;
	
	do
	{
		if (generateNum && getpid() != father)
		{
			// process ler random money olusturup pipe a yazacak.
			//int curPid = clientPid[i];
			money = bank();
			mb.para = money;
			mb.pid = getpid();
			//printf("MUSTERİBİLET para %d   pid %d\n", mb[m].para,mb[m].pid);
			
			//write(pipeBank[1],&money,sizeof(money));
			write(pipeBank[1],&mb,sizeof(mb));
			//write(pipeBank[1],&curPid,sizeof(int));
			m++;
			generateNum = 0;
		}

		
		if (getpid() == father)
		{
			if (clientPid[j] > 0)
			{	
				struct MusteriBilet musteriSt;

				read(pipeBank[0],&musteriSt,sizeof(musteriSt));
				money = musteriSt.para;

				int sp = clientPid[j];
				// bankfifo ya yaz.
				int services = musteriSt.pid;
				write(fd,&money,sizeof(money));
				kill(sp, SIGUSR1);

				//printf("MUSTERİBİLET para %d   Servis %d   musteri %d\n", musteriSt.para,musteriSt.pid,clientPid[j]);
				
				//printf("BANKA : Musteri %d %2d parasını aldı :)\n", clientPid[j], money);
				
				getitimer(ITIMER_REAL,&itv);
				double kalanZaman = (TIMEOUT * 1000) - (itv.it_value.tv_sec*1000 + itv.it_value.tv_usec / 1000);
				//printf("gecen süre : %.0lf msec\n", kalanZaman);
				fprintf(f, "%d  %d  %d  %.0lf msec\n", clientPid[j], musteriSt.pid, musteriSt.para, kalanZaman);
				fflush(f);
			}
			j++;
			if (j == 100)
				j = 0;
			
		}


	
		if(closeBank)
		{	// banka kapandi
			int x = -123;
			int c = i;
			while(c < musteriCounter){
				write(fd,&x, sizeof(int));
				c++;
			}
			break;
		}


		processCounter++;
		if (processCounter == 4)
			processCounter = 0;
		i++;
		if (i == musteriCounter)
		{
			i=0;
		}

	}while(1);


	if (getpid() == father)
	{
		for (int i = 0; i < 4; ++i)
		{
			kill(pid[i],SIGKILL);
		}
	}

	fflush(f);
	fprintf(f,"%d saniye dolmustur. %d müsteriye hizmet verdik.\n",TIMEOUT,musteriCounter );
	fflush(f);
	/*
	for (int i = 0,j=0; i < musteriCounter; ++i,++j){
		printf("%d %d\n",pid[j],clientPid[i] );
		if (j == 4)
			j=0;
	}
	*/
	fprintf(f,"musteriye hizmet sundu ....\n");
	fflush(f);

	fclose(f);
	close(fd);
	unlink(FIFO);
	
	return 0;
}



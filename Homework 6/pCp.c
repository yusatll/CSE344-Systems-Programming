#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>

#define MAX 100
/*
	iki tane entry al. 2 dir name. 1deki her dosya ve alt klasör için dosyayı aç ve oku. 
	hedefte o dosyayı oluştur. varsa önceden siler. hata olursa ikisinide kapatıp error yazar. 
	fd ler ve name ler  buffera yaz. buffer kontrolünüde yap thread ler için. producer buffer doldurma
	işini bitirince veya buffer dolunca down flag yap ve kapan.
	
	consumer: her biri thread oluşturur. her consumer bufferdan 1 tane item okur. bunlara kopyalama yapar.
	işi bitince ekrana bitti yazar ve kapar dosyaları. hepsi print ederken karışmaması lazım. 
	buffer boşalıp ama down flag yoksa bitmemiş demektir.
	
	main : pthread_join kullan. consumer sayısı kadar thread ve producer için 1 thread üretir.
	ilk thread den öncee ve iş bitince gettimeofday ile kopyalama süresini hesapla. 
	normal dosyaları ve fifo ları kopyalayacak. kopyalanan dosyaarın kaç tane ve tipleri size ları tut. 
	toplam byte tut. 
*/

/* 	*/
void *producer(void* arg);

/*	reads an item from the buffer, copies the file from the source file
descriptor to the destination file descriptor, closes the files */
void *consumer(void* arg);
bool DONE = -1;

struct FileInfo
{
	int fileDisct;
	int size;	
	char name[10];
	char type[10];
};

struct FileInfo fileArray[MAX];


/* 		FILE QUEUE		*/ 
/* 	https://www.tutorialspoint.com/data_structures_algorithms/queue_program_in_c.htm  */
int front = 0;
int rear = -1;
int itemCount = 0;
char * buffer;

bool isEmpty() {
   return itemCount == 0;
}

bool isFull() {
   return itemCount == MAX;
}

int size() {
   return itemCount;
} 

void insert(struct FileInfo data) {

   if(!isFull()) {
	
	  if(rear == MAX-1) {
		 rear = -1;            
	  }       

	  fileArray[++rear] = data;
	  itemCount++;
   }
}

struct FileInfo removeData() 
{
   struct FileInfo data = fileArray[front++];
	
   if(front == MAX) {
	  front = 0;
   }
	
   itemCount--;
   return data;  
}

sig_atomic_t closeBank = 0;

void signalHandler(int sig)
{
	closeBank = 1;
}

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
int counter = 0;

/* Usage : pCp 10 5 source_dir_path dest_dir_path */
int main(int argc, char const *argv[])
{
	int bufferSize = 10, source = 5;

	struct sigaction sa;
	sa.sa_handler = &signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	char * sourcePath = "Desktop/source";
	char * destPath = "Desktop/hedef";
	char * temp;
	long j=0;

	char *param = (char*)malloc(sizeof(char) * 1024);

	strcat(param, sourcePath);
	strcat(param, " & ");
	strcat(param, destPath);


	if (pthread_mutex_init(&lock, NULL) != 0) 
	{ 
		printf("\n mutex init has failed\n"); 
		return 1; 
	} 

/*
	bufferSize = strtol(argv[1],&temp,10);
	source = strtol(argv[2],&temp,10);
	strcpy(sourcePath, argv[3]);
	strcpy(destPath, argv[4]);
*/

	buffer = (char*)malloc(sizeof(char) * bufferSize);

	pthread_t consumers[source];
	pthread_t producerThread;

	// producer thread created
	pthread_create(&producerThread,NULL,&producer,(char*)param);


	// consumers threads created
	for (j = 0; j < source-1; j++)
		pthread_create(&consumers[j],NULL,&consumer,(void*)j);

	if (sigaction(SIGINT, &sa, NULL) == -1){
		printf("EXIT\n");
		exit(1);
	}

	pthread_join(producerThread,NULL);

	for (j = 0; j < source-1; j++)
		pthread_join(consumers[j],NULL);

	pthread_mutex_destroy(&lock); 
	return 0;
}

void *producer(void* arg)
{
	char * source = strtok(arg , " & ");
	char * dest   = strtok(NULL, " & ");;

	while(!closeBank)
	{
		pthread_mutex_lock(&lock);
		long index = 0;
		struct FileInfo data;

		// index = (long) arg;
		// printf("Yılmaz producer.\n");


		for (int i = 0; i < 500; ++i)
		{
			if (!isFull())
			{
				//printf("producer wait.\n");
				//pthread_cond_wait(&cond_var, &lock);
				printf("producer signal.\n");
				pthread_cond_signal(&cond_var);
				data.size = 10;
				data.fileDisct = i;
				insert(data);
			}
			else
			{
				pthread_cond_wait(&cond_var, &lock);
			}



			if (i == 499)
			{
				DONE = 1;
				printf("producer break \n" );
				//pthread_cond_wait(&cond_var, &lock);
				break;
			}
		}
		pthread_mutex_unlock(&lock); 
	}
	pthread_exit(NULL);

}

void *consumer(void* arg)
{
	while(!closeBank)
	{
		pthread_mutex_lock(&lock);
		struct FileInfo f;
		long index = 0;

		counter++;
		printf("id %lu ",pthread_self());
		index = (long) arg;
		//printf("yılmaz consumer\n");

		if (!isEmpty())
		{
			f = removeData();
			printf("buffer removed : %d %d\n", f.size, f.fileDisct);
		}
		else
		{
			pthread_cond_signal(&cond_var);
			printf("isFull - consumer wait.\n");
			pthread_cond_wait(&cond_var, &lock);
		}

		//if (DONE == 1)	break;		
		pthread_mutex_unlock(&lock); 

	}
	pthread_exit(NULL);
}
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h> 
#include <stdbool.h>
#include <unistd.h> 
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

#define MAX 1024 
#define PORT 8080 
#define SA struct sockaddr_in 

/*	https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/	*/
sem_t mutex; 

pthread_t allThreads[500];

typedef struct sendBox
{
	char filename[30];
	char buffer[MAX];
	int id;
}sendBox;

typedef struct boxArray
{
	sendBox s[20];
	int size;
	char deleted[30];
	int sigFlag;
	char argvbir[100];
}boxArray;

boxArray b;
bool returnAllFiles = false;
int socketCounter = 0;
char path[100];
bool backupTime = false;
char clientPath[100];
FILE * logFile = NULL;

pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER; 

/*	QUEUE 	*/
/*	https://www.tutorialspoint.com/data_structures_algorithms/queue_program_in_c.htm 	*/
int socketIdQueue[100];
int front = 0;
int rear = -1;
int itemCount = 0;
bool isEmpty() {
   return itemCount == 0;
}

bool isFull() {
   return itemCount == MAX;
}

int size() {
   return itemCount;
}  
void insert(int data) {

   if(!isFull()) {
	
      if(rear == MAX-1) {
         rear = -1;            
      }       

      socketIdQueue[++rear] = data;
      itemCount++;
   }
}

int removeSocket() {
   int data = socketIdQueue[front++];
	
   if(front == MAX) {
      front = 0;
   }
	
   itemCount--;
   return data;  
}
/*************************************************************************/


boxArray filereader(int socketID,boxArray rfiles,char * curPath)
{
	// server daki dosyalari okur. socket e gonderir.
	sendBox s;
	DIR *dir;
	struct dirent *dent;
	dir = opendir(curPath);	// klasörü aç.
	printf("Server %s dosyaları clientlara gönderiliyor.\n", curPath);

	FILE * fptr;
	int i=0;
	
	while((dent = readdir(dir)) != NULL)
	{
		bzero(&s,sizeof(s));
		if ((strcmp(dent->d_name, ".") == 0) || (strcmp(dent->d_name, "..") == 0))
		{
			continue;
		}

		char cond[400];
		sprintf(cond,"%s/%s",curPath ,dent->d_name);
		fptr = fopen(cond,"rb");
		fread(s.buffer,sizeof(char)*MAX,1, fptr);
		strcpy(s.filename, dent->d_name);
		fclose(fptr); 
		rfiles.s[i] = s;
		i++;
	}
	rfiles.size = i;

	return rfiles;	
}

char * tokenPath(char * param)
{
	DIR * dir;
	struct dirent * dent;
	dir = opendir(param);
	//printf("param : %s\n",param);

	bzero(clientPath,sizeof(clientPath));
	int j=0;
	for (int i = strlen(param)-2; param[i] != '/'; i--,j++)
	{
		clientPath[j] = param[i];
	}

	char *temp = malloc(sizeof(char) *50);
	int t = j-1;
	int a;
	for (a = 0; a < j; a++,t--)
	{
		temp[a] = clientPath[t]; 

	}
	temp[a] = '\0';
	//printf("token return %s\n", temp);
	return temp;
	//printf("clientPath : %s\n",clientPath );
}

/**************************************************************************************/
// Function designed for chat between client and server. 

bool checkFilenames(char * argumant)
{
	char tempstr[50];
	int j;
	bzero(tempstr,sizeof(tempstr));
	//printf("ARGUMANT : %s\n",argumant );
	for (int i = strlen(argumant)-2, j=0; argumant[i] != '/'; i--,j++)
	{
		tempstr[j] = argumant[i];
		//printf("tempstr : %c\n", tempstr[j] );
	}

	//printf("tempstr : %s\n",tempstr );
	if (strcmp(tempstr, clientPath) == 0){
		//printf("ikiside eşit\n");
		return true;
	}
	return false;
	
}


//void func(int sockfd) 
void * func(void * s)
{ 
	sem_wait(&mutex);
	char curPath[300];
	int socketID = removeSocket();
	//printf("remove socket %d \n", socketID);
	printf("Thread create.\n");
	fprintf(logFile,"Thread create.\n");
	fflush(logFile);
	char filereadpath[100];
	
	bzero(&b,sizeof(b));

	while(1)
	{
		//printf("reading...\n");
		sleep(1);
		read(socketID,&b,sizeof(b));

		
		for (int socketCounter = 0; socketCounter < b.size; ++socketCounter)
		{
			if (b.sigFlag == 333)	// client kapandı.
			{
				printf("SIGNAL HANDLED.\n");
				fprintf(logFile, "Signal Geldi.\n" );
				fflush(logFile);
				// SERVER DAKİ DOSYALARI CLİENT LARA VERİYORUZ.
				//backUp(socketID);
				returnAllFiles = true;
				b.sigFlag = 0;
				//strcpy(clientPath,b.argvbir);
				tokenPath(b.argvbir);	// path in en son klasörünü al.
			}

			if(returnAllFiles && checkFilenames(b.argvbir))
			{
				
				boxArray rfiles;

				//backupTime = true;

				// bütün file ları doldur. ve write yap.

				rfiles = filereader(socketID,rfiles,filereadpath);
				

				//write(socketID,&backupTime,sizeof(backupTime));
				//printf("write backupTime %d\n",backupTime );
				//read(socketID,&backupTime,sizeof(backupTime));
				//printf("read backupTime %d\n",backupTime );

				fprintf(logFile, "Aynı klasöre sahip clientlara tüm dosyalar gönderiliyor.\n" );
				write(socketID,&rfiles,sizeof(rfiles));
				returnAllFiles = false;
			}	
			
			
		//	printf("file name %s counter %d\n", b.s[socketCounter].filename, socketCounter);

			// directory gelirse onun adında mkdir yap.
			/*
			if (strcmp(b.s[socketCounter].buffer,"-/-direcTORY-/-") == 0)
			{
				
				//sprintf(curPath,"%s/%s",path,b.s[socketCounter].filename);
				strcpy(curPath,path);
				strcat(curPath,"/");
				strcat(curPath,b.s[socketCounter].filename);

				printf("curPath : %s\n",curPath );
				mkdir(curPath,0700);
				strcat(path,"/");
				strcat(path,b.s[socketCounter].filename);
				socketCounter++;
			}
			*/

			//mutex lock
			pthread_mutex_lock(&lock);

			char * dirname = tokenPath(b.argvbir);
			//printf("gelen path : %s\n", path);
			printf("dosya: %s\n",dirname );
			strcpy(curPath,path);
			//strcat(curPath,"/");
			
			strcat(curPath,dirname);
			

			strcat(curPath,"/");
			strcpy(filereadpath,curPath);
			mkdir(filereadpath,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			//sprintf(curPath,"%s/%s/",curPath,dirname);
			strcat(curPath,b.s[socketCounter].filename);
			
			printf("path: %s\n",curPath);
			fprintf(logFile, "%s klasöründe %s dosyasi olusturuldu.\n",dirname,curPath );
			fflush(logFile);

			FILE * newfile = fopen(curPath, "wb+");
			
			if(newfile != NULL){
				//printf("filename : %s, id %d\n",b.s[socketCounter].filename, b.s[socketCounter].id );
		//		printf("file : %s\n",b.s[socketCounter].buffer );

				char writeStr[MAX];
				strcpy(writeStr, b.s[socketCounter].buffer);
				//printf("writestr %s\n",writeStr );
				fprintf(newfile, "%s",writeStr );
				fflush(logFile);
		//		printf("2\n");
			}
			else
				printf("exit.\n");

			//printf("1\n");
			char* index = malloc(sizeof(b.deleted));
			index[0] = '\0';
			strcpy(index, b.deleted);
			//printf("index %s\n", index);

			if (index[0] != '\0'){
				//printf("3\n");
				char delPath[50];
				strcpy(delPath,path);
				//strcat(delPath,"/");

				strcat(delPath,index);
				printf("Bu dosya silindi : %s\n",delPath);
				fprintf(logFile,"Bu dosya silindi : %s\n",delPath);
				fflush(logFile);
				remove(delPath);
				//b.deleted = -1;
			}

			//socketCounter++;
			fclose(newfile);

			//mutex unlock
			pthread_mutex_unlock(&lock); 
		}
		//write(socketID,&backupTime,sizeof(backupTime));

	}
	pthread_exit(NULL);
} 
  
// Driver function 
//	BibakBOXServer [directory] [threadPoolSize] [portnumber]
int main(int argc, char const *argv[]) 
{ 
	int poolsize = atoi(argv[2]);
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli;

	if (pthread_mutex_init(&lock, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
	
	logFile = fopen("server.log","a+");

 	strcpy(path,argv[1]);
	//strcpy(path, "/home/yusa/Desktop/serverDosya/");
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else{
		printf("Socket successfully created..\n"); 
		fprintf(logFile,"Socket successfully created..\n"); 
	}
	//bzero(&servaddr, sizeof(servaddr)); 
  	memset(&servaddr,0,sizeof(servaddr));
	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); // atoi(argv[3])
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else{
		printf("Socket successfully binded..\n"); 
		fprintf(logFile,"Socket successfully binded..\n"); 
	}
  
	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else{
		printf("Server listening..\n"); 
		fprintf(logFile,"Server listening..\n"); 
	}
	len = sizeof(cli); 
  


  	int i;
  	sem_init(&mutex, 0, 0);

	for (i = 0; i < poolsize; ++i)
	{
		int * a = &connfd;
		if(pthread_create(&allThreads[i], NULL, func,a)<0){
			perror("Thread couldnot create.");
			return 1;
        }    		
	}

	for (i = 0; i < poolsize; ++i)
	{
		// Accept the data packet from client and verification 
		connfd = accept(sockfd, (struct sockaddr*)&cli, &len); 
		insert(connfd);
		//printf("insert socket %d\n", connfd);
		if (connfd < 0) { 
			printf("server acccept failed...\n"); 
			exit(0); 
		} 
		else{
			printf("server acccept the client...\n"); 
			fprintf(logFile,"server acccept the client...\n"); 
		}
  
		sem_post(&mutex);
		
	}

	for (i = 0; i < poolsize; ++i)
	{
 		pthread_join(allThreads[i],NULL);

    }


	// Function for chatting between client and server 
	//func(connfd); 
  
	// After chatting close the socket 
	sem_destroy(&mutex);
	close(sockfd); 
} 


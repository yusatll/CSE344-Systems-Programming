#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>


#define MAX 1024 
#define PORT 8080 
#define SA struct sockaddr 

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
boxArray checkArray;
int sockfd; 
char PATH[100];

bool backupTime = false;

int readFiles(char * path)
{
	sendBox s;
	int boxCount = 0;
	DIR *dir;
	struct dirent *dent;
	struct stat st;
	dir = opendir(PATH);	// klasörü aç.
	FILE * fptr;
	strcpy(b.argvbir,PATH);

	while((dent = readdir(dir)) != NULL)	// klasörün içini okumaya başla.
	{
		bzero(&s,sizeof(s));
		if ((strcmp(dent->d_name, ".") == 0) || (strcmp(dent->d_name, "..") == 0))
		{
			continue;
		}
		//printf("1\n");
		/*
		if (dent->d_type == DT_DIR)				// directory ise ona farklı işlem yap.
		{
			s.id = -155;								// -155 ise directory dir.
			strcpy(s.buffer,"-/-direcTORY-/-");			// bu varsa directory dir.
			strcpy(s.filename, dent->d_name);
			//printf("2\n");
		}
		*/
		// başka dosya ise içini oku.
		
		//printf("d_name -%s-\n",dent->d_name);
		char cond[400];
		sprintf(cond,"%s/%s",PATH ,dent->d_name);
		fptr = fopen(cond,"rb");		// open file.
		if (fptr == NULL)
			perror("null");

		//s.id = sockfd;
		int sz;								// save socket num
		if (stat(dent->d_name,&st) == 0)
			sz = st.st_size;
		
		if(sz >= 1){
//				if(dent->d_reclen){
			fread(s.buffer,sizeof(char)*1024,1, fptr);		// read file to buffer
		}
		strcpy(s.filename, dent->d_name);				// save filename to struct
		

		//strcpy(b.s[ct].filename , dent->d_name);	// ilk halini al.
		
		fclose(fptr); 
		//printf("buffer   %s\n",s.buffer );
	

		//printf("4\n");
		b.s[boxCount] = s;			// array e al. oluşturulan dosyaları.
		boxCount++;
	}

		//b.size = ct;
	b.size = boxCount;
	closedir(dir);
}


int checkDelete(char * path)
{
	//printf("checkDelete path : %s\n",path);
	DIR * dir = opendir(path);
	int size =0;
	struct dirent *dt;
	while((dt = readdir(dir)) != NULL)
	{
		if ((strcmp(dt->d_name, ".") == 0) || (strcmp(dt->d_name, "..") == 0))
		{}
		else
		{
			//printf("dosya :  %s\n",dt->d_name);
			strcpy(checkArray.s[size].filename , dt->d_name);
			size++;
		}
		
	}
	checkArray.size = size;

	int deleted = -1;
	char * deleteF = malloc(sizeof(char)*50);
	deleteF[0] = '\0';

	for (int i = 0; i < b.size; ++i)
	{
		for (int j = 0; j < checkArray.size; ++j)
		{
			//printf(".%s. - .%s.\n", b.s[i].filename,  checkArray.s[j].filename);
			if (strcmp(b.s[i].filename, checkArray.s[j].filename) == 0)
			{
				deleted = -1;
				break;
			}
			else{
				deleted = i;
			}
			
		}
		if(deleted != -1){
			strcpy(deleteF,b.s[deleted].filename);
			break;
		}

	}

	strcpy(deleteF,b.s[deleted].filename);

	//printf("DELETED : %d  file : %s\n",deleted, deleteF);
	return deleted;
	
}

void removeDeletedFileFromStruct(int index) {

	for (int i = index; i < b.size - 1; ++i)
	{
		b.s[i] = b.s[i + 1];
	}
	b.size--;
}

void printFileName() {
	for (int i = 0; i < b.size; ++i)
	{
		printf("%s\n", b.s[i].filename );
	}
}

void signalHandler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM){   
		printf("Signal Handle edildi. Client exit...\n");
		b.sigFlag = 333;
	}
	write(sockfd, &b, sizeof(b));

	// serverdan read yap. bütün filer ları kendine yaz.
	// senkronizasyon icin template degeri okuyup yaz.
	//read(sockfd,&backupTime,sizeof(backupTime));
	//printf("read backupTime %d\n",backupTime );
	//write(sockfd,&backupTime,sizeof(backupTime));
	//printf("write backupTime %d\n",backupTime );
	// serverdaki tum dosyalari al.
	boxArray sbx;
	bzero(&sbx,sizeof(sbx));
	read(sockfd,&sbx,sizeof(sbx));

	//printf("sbx size %d\n",sbx.size);

	for (int i = 0; i < sbx.size; ++i)
	{
		//printf("gelen filename  %s\n",sbx.s[i].filename );
		char cond[150];
		sprintf(cond,"%s/%s",PATH ,sbx.s[i].filename);
		FILE * newfile = fopen(cond, "wb+");
		if (newfile != NULL)
		{
			char writeStr[MAX];
			strcpy(writeStr, sbx.s[i].buffer);
			fprintf(newfile, "%s",writeStr );
		}
	}


	close(sockfd);
	exit(0);
}



void func(char dirName[50])
{
	sigset_t new_set,old_set;
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &signalHandler;
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}

	sigemptyset (&new_set);
	sigaddset (&new_set, SIGINT);

	//printf("gelendir %s\n",dirName );
	readFiles(PATH);
	while(1)
	{
		// burada sinyal gelirse bekletilecek.
		// işlemler bitince sinyal geri verilecek.
		sigprocmask(SIG_BLOCK, &new_set, &old_set);
		sleep(1);
		
		
		

		if (backupTime == true)
		{
			// serverdan read yap. bütün filer ları kendine yaz.
			
			//read(sockfd,&backupTime,sizeof(backupTime));
			//printf("read backupTime %d\n",backupTime );
			backupTime = false;
			//write(sockfd,&backupTime,sizeof(backupTime));
			//printf("write backupTime %d\n",backupTime );

			read(sockfd, &b, sizeof(b));	
			/*
			for (int i = 0; i < b.size; ++i)
			{
				char cond[60];
				sprintf(cond,"%s/%s",dirName,b.s[i].filename);
				FILE * newfile = fopen(cond, "wb+");
				if (newfile != NULL)
				{
					char writeStr[MAX];
					strcpy(writeStr, b.s[i].buffer);
					fprintf(newfile, "%s",writeStr );
				}
			}
			*/
			
		}

		printFileName();
		
		// değişen file varsa onun adını bulup. arrayin içine ekliyoruz.

		int a = checkDelete(PATH);

		strcpy(b.deleted, b.s[a].filename);

		//b.deleted = a;
		//printf("a :deletedfile    %s\n",a);
		write(sockfd, &b, sizeof(b));		// arrayi sokete gönder.


		if (a != -1){
			//printf("a %s\n",a );

			removeDeletedFileFromStruct(a);
//			strcpy(b.deleted,NULL) ;
			
		}else{
			readFiles(PATH);
		}

		strcpy(b.deleted, "");

		sigprocmask(SIG_SETMASK, &old_set, NULL);

		//read(sockfd,&backupTime,sizeof(backupTime));
	}

}




// BibakBOXClient [dirName] [ip address] [portnumber]
int main(int argc, char const *argv[]) 
{ 
	struct sockaddr_in servaddr, cli; 
	//char dirName[50] = "/home/yusa/Desktop/clientDosya";
	strcpy(PATH ,argv[1]);
	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	//bzero(&servaddr, sizeof(servaddr)); 
	memset(&servaddr,0,sizeof(servaddr));
	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // argv[2]
	servaddr.sin_port = htons(PORT); 
  
	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("Client %d connected to the server..\n",sockfd); 
  
  	func(PATH);
	// close the socket 
	 
} 
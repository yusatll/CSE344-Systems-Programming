#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/file.h>

int checkZ;
/**
* char *path : Takes a path as a parameter.
*
* int pathfun (char *path1) : takes a function as parameter.
* this function use the depth-first search algorithm. and find directory size and total size.
* return total size to main.
*/
int postOrderApply (char *path , int pathfun (char *path1));


/**
*
* @param path : This function takes a path. directory path.
*	this function find stat size given path.
*/
int sizepathfun (char *path);
int calculate(FILE* fd, char * path);

char filename[25] = "151044092sizes.txt";
//char fifofolder[20] = "fifo";
#define fifofolder "fifo2"

int main(int argc, char const *argv[])
{
	char path[100];
	
	//remove(filename);
	// argc == 3 ise exe -z path
	// argc = 2 ise exe path
	
	if(argc == 3)
	{
		if (strcmp(argv[1],"-z") == 0)
		{
			checkZ = 1;
			strcpy(path,argv[2]);
		}
		else{
			perror("Wrong parameter. Only enter -z.");
			return 1;
		}
	}
	else if(argc == 2)
	{
		checkZ = 0;
		if (strcmp(argv[1],"-z") != 0)
			strcpy(path,argv[1]);
	}
	
	//fd = fopen(filename, "a+");
	FILE* fd;
	FILE * pidfile;
	
	int total =0;

	unlink(fifofolder);
	umask(0);
	if(mkfifo(fifofolder, 0666) == -1)
		printf("hata\n");
	fd = fopen(fifofolder, "a+");

	pid_t pid = fork();

	if(pid == (pid_t) 0)
	{
		//close(fds[1]);
		fprintf(fd,"%s \t%s\t\t%s\n","PID","SIZE","PATH");
		fflush(fd);
		
		printf("%s\t%s\t\t%s\n","PID","SIZE","PATH" );
	
		total = postOrderApply(path,sizepathfun);

			
		printf("%d\t", getpid());
		printf("%d\t\t", total);
		printf("%s\n", path);

		int pc = getpid();

		fprintf(fd,"%d\t", getpid());
		fprintf(fd,"%d\t\t", total);
		fprintf(fd,"%s\n", path);
		fflush(fd);
		
		exit(1);
	}
	else
	{	/* PARENT 	*/
		//wait(NULL);
		fclose(fd);
		pidfile = fopen(fifofolder, "r");
		if(pidfile == NULL)
			printf("error\n");


		size_t bufsize = 256;
		char * buffer1 = malloc( bufsize * sizeof(char)); 
		int line=0,ch=0;
		int i=0;

		int id=0;
		int pidArr[100];
		int r=0;

		while( r = (getline(&buffer1, &bufsize,pidfile)) >= 0){
			sscanf(buffer1,"%d",&id);
			//printf("buf %s" ,buffer1);
			pidArr[i] = id;
			i++;
			line++;
		}
		free(buffer1);
		
		//rewind(pidfile);
		char firstline[256];
		
		fgets(firstline, sizeof(firstline), pidfile);
		
		
		int childC = 0;

		for (int i = 0; i < line-1; ++i) 
		{ 
			int j = 0; 
			for (j = 0; j < i; ++j)
			{
				if (pidArr[i] == pidArr[j])
				{
					break; 
				}
			}
			if (i == j)
			{
				++childC;
			}
		}
		
		printf("%d child processes created. Main process is %d\n.",childC, getpid() );
		fprintf(fd,"%d child processes created. Main process is %d.",childC, getpid());
		
	}
	
	unlink(fifofolder);
	fclose(pidfile);
	fclose(fd);

	
	return 0;
}


int postOrderApply (char *path , int pathfun (char *path1))
{
	char longPath[600]; 
	struct dirent *direntPtr;

	DIR *dir = opendir(path);
	int total_size=0;
	int sizeOfDir = 0;
	int sum=0;
	int x = 0;
	int fds[2];
	FILE * fd = fopen(fifofolder, "a+");;
	unlink(fifofolder);
	umask(0);
	mkfifo(fifofolder, 0666);
	pid_t pidC;


	if(!dir)
		return -1;

	while((direntPtr = readdir(dir)) != NULL)
	{
		if(strcmp(direntPtr->d_name, ".") != 0 && strcmp(direntPtr->d_name, "..") != 0)
		{

			strcpy(longPath, path);
			strcat(longPath, "/");
			strcat(longPath, direntPtr->d_name);

			if(direntPtr->d_type == DT_DIR)
			{
				pidC = fork();
				if(pipe(fds) == -1)
					printf("pipe error!\n");

				if(pidC >= 0) 
				{
					if(pidC == 0) 
					{
						strcpy(longPath, path);
						strcat(longPath, "/");
						strcat(longPath, direntPtr->d_name);
				
						//closedir(dir);
						sizeOfDir = postOrderApply(longPath, pathfun);

						if (checkZ == 1)
						{
							close(fds[1]);
							write(fds[0], &sizeOfDir, sizeof(int));
							close(fds[0]);
						}

						fprintf(fd, "%d\t%d\t\t%s\n", getpid(),sizeOfDir, longPath);
						fclose(fd);
						printf("%d\t", getpid());
						printf("%d\t\t", sizeOfDir);
						printf("%s\n", longPath);
						exit(0);
					}
					
				}
				else
				{	
					//wait(NULL);
				}
				
			}
			else if(direntPtr->d_type == DT_FIFO || direntPtr->d_type == DT_LNK)
			{
				printf("%d\t", getpid());
				printf("-1\t\t");
				printf("Special_file_%s\n", direntPtr->d_name);

				fprintf(fd,"%d\t", getpid());
				fprintf(fd,"-1\t\t");
				fprintf(fd,"Special_file_%s\n", direntPtr->d_name);
				fflush(fd);
			}
			else 
			{				
				sum += (pathfun(longPath) / 1024);
			}
			
		}
	}

	while(wait(&x) > 0);


	if(checkZ == 1){
		close(fds[1]);
		read(fds[0],&sum, sizeof(int));	
		close(fds[0]);
	}

	fclose(fd);
	closedir(dir);
	unlink(fifofolder);

	// check -z parameter.
	if(checkZ == 1)
		total_size += sum;
	else 
		return sum;
	return total_size;
}

int sizepathfun (char *path)
{
	struct stat *statPtr;
	//stat(path,&statPtr);
	int size;
	
	statPtr = malloc(sizeof(struct stat));
	
	if(stat(path, statPtr) == -1) {
		return -1;
	}
	else
	{
		size = (int)statPtr->st_size;
	}
	free(statPtr);
	return size;
}

int calculate(FILE* fd, char * path)
{
	int lines=0;
	char ch;
	fseek(fd,0,SEEK_SET);
	rewind(fd);

	//printf("calculate\n");

	while(!feof(fd)){
		ch = fgetc(fd);
		if(ch == '\n'){
			lines++;
		}
	}
	lines--;

	rewind(fd);
	char firstline[255];
	fgets(firstline, sizeof(firstline), fd);
	
	printf("firstline %s\n", firstline);
	int pidArr[lines];
	char sizeArr[1024][lines];
	char pathArr[1024][lines];
	
	char *buffer;
	size_t bufsize = 32;
	buffer = (char*)malloc(bufsize* sizeof(char));

	for (int i = 0; i<lines; ++i)
	{
		
		getline(&buffer,&bufsize,fd);
		sscanf(buffer,"%d%s%s",&pidArr[i],sizeArr[i],pathArr[i]);		
	}

	free(buffer);

	int size = 0,j=0;
	for(j=0; j<lines ;j++)
	{
		if((strcmp(sizeArr[j],"Special") && strcmp(pathArr[j],"file")))
		{
			if(strcmp(pathArr[j],path)>0)	//it is subdirectory.
				sscanf(sizeArr[j],"%d",&size);
		}
	}
	fseek(fd, 0, SEEK_END);
	
	return size;
}
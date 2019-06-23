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


int main(int argc, char const *argv[])
{
	char path[100];
	FILE * fd;
	int total =0;
	remove(filename);
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
	
	fd = fopen(filename, "a+");
	
	pid_t pid = fork();

	if(pid == 0)
	{
		fprintf(fd,"%s \t%s\t\t%s\n","PID","SIZE","PATH");
		fflush(fd);

		printf("%s\t%s\t\t%s\n","PID","SIZE","PATH" );
	
		total = postOrderApply(path,sizepathfun);

		if(checkZ == 1)
			total += calculate(fd,path);
		
		printf("%d\t", getpid());
		printf("%d\t\t", total);
		printf("%s\n", path);

		fprintf(fd,"%d\t", getpid());
		fprintf(fd,"%d\t\t", total);
		fprintf(fd,"%s\n", path);

		fflush(fd);

		exit(1);
	}
	else
	{
		wait(NULL);

		FILE * pidfile = fopen(filename, "r+");
		int line=0,ch=0;;
		while ((ch = fgetc(pidfile)) != EOF)
		{
			if (ch == '\n')
				line++;
		}
		fseek(pidfile,0,SEEK_SET);
		//rewind(pidfile);
		char firstline[256];
		char c = ' ';
		fgets(firstline, sizeof(firstline), pidfile);
		size_t bufsize = 32;
		char * buffer = malloc( bufsize * sizeof(char)); 
		int id=0;
		int pidArr[100];

		
		for (int i = 0; i<line-1;i++)
		{
			getline(&buffer,&bufsize,pidfile);
			sscanf(buffer,"%d",&id);
			pidArr[i] = id;
		}
		
		free(buffer);
		printf("\n");
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
		fprintf(fd,"%d child processes created. Main process is %d.",childC, getpid() );

		fclose(pidfile);
	}
	
	//calculate(fd,path);
	fclose(fd);

	return 0;
}

int postOrderApply (char *path , int pathfun (char *path1))
{
	char longPath[600]; 	// all path is here. 
	struct dirent *direntPtr;
	struct flock lock;
	DIR *dir = opendir(path);
	int  total_size=0;
	int sizeOfDir = 0;
	int sum=0;
	int x = 0;

	FILE * fd = fopen(filename,"a+");
	int fcheck = flock(fileno(fd), LOCK_SH);
	pid_t pidC;
	/*The  opendir()  function  opens a directory stream corresponding to the
		directory name, and returns a pointer to  the  directory  stream.   The
		stream is positioned at the first entry in the directory.*/

	if(!dir)
		return -1;

	while((direntPtr = readdir(dir)) != NULL)
	{
		if(strcmp(direntPtr->d_name, ".") != 0 && strcmp(direntPtr->d_name, "..") != 0)
		{
			
			strcpy(longPath, path);
			strcat(longPath, "/");
			strcat(longPath, direntPtr->d_name);
			
			// if Dirent's type is be Directory we should find its size and add total size.
			if(direntPtr->d_type == DT_DIR)
			{
				pidC = fork();
				if(pidC >= 0) //parent and child giricek.
				{
					if(pidC == 0) // child giricek.
					{
						strcpy(longPath, path);
						strcat(longPath, "/");
						strcat(longPath, direntPtr->d_name);

						closedir(dir);
						
						sizeOfDir = (postOrderApply(longPath, pathfun) + (pathfun(longPath)) / 1024) -4;

						total_size += sizeOfDir;


						if(fcheck == 0)
						{
							if(checkZ == 1)
								sizeOfDir += calculate(fd,longPath);

							flock(fileno(fd), LOCK_UN);
							printf("%d\t", getpid());
							printf("%d\t\t", sizeOfDir);
							printf("%s\n", longPath);

							fprintf(fd,"%d\t", getpid());
							fprintf(fd,"%d\t\t", sizeOfDir);
							fprintf(fd,"%s\n", longPath);

						}
						
						
						fflush(fd);
						
						fcheck = flock(fileno(fd), LOCK_SH);

						//fclose(fd);
						exit(0);
					}
				}
				else
				{
					// parent dosya dolaşmayı bitirince child ları bekle.
					wait(NULL);
				}
				
			}
			else if(direntPtr->d_type == DT_FIFO || direntPtr->d_type == DT_LNK)
			{
				// this time if dirent type is special file fifo or link. we should find them and write their name.
				
				
				/*
				memset(&lock, 0, sizeof(lock));
				lock.l_type = F_WRLCK;
				*/
				if(fcheck == 0)
				{
					flock(fileno(fd), LOCK_UN);
					printf("%d\t", getpid());
					printf("\t\t");
					printf("Special file %s\n", direntPtr->d_name);

					fprintf(fd,"%d\t", getpid());
					fprintf(fd,"\t\t");
					fprintf(fd,"Special file %s\n", direntPtr->d_name);
				}

				
				//fseek(stdin,0,SEEK_END);
				fflush(fd);
				fcheck = flock(fileno(fd), LOCK_UN);
				//lock.l_type = F_UNLCK;
				//fclose(fd);
			}
			else // if it is differet type, we should add its size. if we have -z parameter we add it.
			{				
				sum += (pathfun(longPath) / 1024);
			}
			
		}
	}
	while(wait(&x) > 0);

	fclose(fd);
	closedir(dir);
	// check -z parameter.
	if(checkZ)
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
	//fseek(fd,0,SEEK_SET);
	rewind(fd);
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
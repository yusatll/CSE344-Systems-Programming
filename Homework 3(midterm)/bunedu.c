#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

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


int main(int argc, char const *argv[])
{
	char path[100];
	// argc == 3 ise exe -z path
	// argc = 2 ise exe path

	if(argc == 3)
	{
		if (strcmp(argv[1],"-z") == 0)
		{
			printf("girdi\n");
			checkZ = 1;
			strcpy(path,argv[2]);
		}
		else{
			perror("Wrong parameter. Only enter -z.");
			exit(1);
		}
	}
	else if(argc == 2)
	{
		checkZ = 0;
		if (strcmp(argv[1],"-z") != 0)
			strcpy(path,argv[1]);
	}
	
	int total = postOrderApply(path,sizepathfun);
	printf("total %d\t", total);
	printf("%s\n", path);
	return 0;
}

int postOrderApply (char *path , int pathfun (char *path1))
{
	char longPath[600]; 	// all path is here. 
	struct dirent *direntPtr;
	DIR *dir = opendir(path);
	int  total_size=0;
	int sum=0;
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
				int sizeOfDir = (postOrderApply(longPath, pathfun) + (pathfun(longPath)) / 1024) -4;
				total_size += sizeOfDir;
				printf("%d\t", sizeOfDir);
				printf("%s\n", longPath);
				
			}
			else if(direntPtr->d_type == DT_FIFO || direntPtr->d_type == DT_LNK)
			{
				// this time if dirent type is special file fifo or link. we should find them and write their name.
				printf("Special file %s\n", direntPtr->d_name);
			}
			else // if it is differet type, we should add its size. if we have -z parameter we add it.
			{				
				sum += (pathfun(longPath) / 1024);
				//printf("else type : %c\n", direntPtr->d_type);
			}
			
		}
	}

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
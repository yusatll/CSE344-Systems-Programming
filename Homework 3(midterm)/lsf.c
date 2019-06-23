#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
	char start[5] = "./";
	char filename[1024];

	DIR *dir;
	dir = opendir(start);
	struct dirent *direntPtr;
	struct stat st;
	int dircheck=0;

	while((direntPtr = readdir(dir)) != NULL)
	{
		dircheck = 0;
		strcpy(filename,start);
		strcat(filename,direntPtr->d_name);
		stat(filename, &st);

		if ((strcmp(direntPtr->d_name,".") != 0) && (strcmp(direntPtr->d_name,"..") != 0))
		{
			if(direntPtr->d_type == DT_REG){	// S_ISREG(st.st_mode)
				printf("%1s", "R");
				dircheck = 1;
			}
			else if(direntPtr->d_type == DT_FIFO || direntPtr->d_type == DT_LNK){
				printf("%1s", "S");
				dircheck = 1;
			}
			if (dircheck == 1)
			{
				printf("%1s",(st.st_mode & S_IRUSR) ? "r" : "-");
				printf("%1s",(st.st_mode & S_IWUSR) ? "w" : "-");
				printf("%1s",(st.st_mode & S_IXUSR) ? "x" : "-");
				printf("%1s",(st.st_mode & S_IRGRP) ? "r" : "-");
				printf("%1s",(st.st_mode & S_IWGRP) ? "w" : "-");
				printf("%1s",(st.st_mode & S_IXGRP) ? "x" : "-");
				printf("%1s",(st.st_mode & S_IROTH) ? "r" : "-");
				printf("%1s",(st.st_mode & S_IWOTH) ? "w" : "-");
				printf("%1s",(st.st_mode & S_IXOTH) ? "x" : "-");

				printf("\t%ld  ",st.st_size);
				printf("\t%s\n", direntPtr->d_name);
			}
		}

	}
	closedir(dir);

	return 0;
}

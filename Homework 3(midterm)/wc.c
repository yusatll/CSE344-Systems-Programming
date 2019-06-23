#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#define stdfile STDIN_FILENO

int counter(int f);

int main(int argc, char const *argv[])
{       
	char character;
	int line = 0;
	int file;

	if (argv[1] != NULL)
	{   
		file = open(argv[1], O_RDONLY);
		line = counter(file);
		printf("%d\n",line);
		close(file);
	}
	else
	{
		line = counter(stdfile);
		printf("%d\n",line);
	}

	return 0;
}

int counter(int f)
{
	int lines=0;
	char character;
	while(read(f,&character,1) > 0)
		if(character == '\n')
			lines++;

	return lines;

}
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define BUF_SIZE 4096

int catFunc(int fd);

int main(int argc, char const *argv[])
{
	int file;
	if (argv[1] != NULL)
	{
		file = open(argv[1], O_RDONLY);
		catFunc(file);
		close(file);
	}
	else
	{
		catFunc(STDIN_FILENO);
	}

	return 0;
}

int catFunc(int fd){
	char buffer[BUF_SIZE];
	int readed=0;
	ssize_t written=0;

	while ((readed = read(fd, buffer, sizeof buffer)) > 0) {

		written = 0;

		while (written < readed) {
			ssize_t numWrt = write(STDOUT_FILENO, buffer + written, readed - written);
			printf("\n");
			if (numWrt < 1)
				return -1;
			written += numWrt;
		}

	}

	return 0;
}
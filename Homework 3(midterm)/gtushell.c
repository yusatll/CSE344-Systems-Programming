#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define LINE_LEN 80
#define BUFFER_SIZE 1024

static char commandInput = '\0';
static int buf_chars = 0;
static char commandLine[LINE_LEN];
sig_atomic_t flag = 1;
int fds[2];


struct command_t {
	char *name;
	int countCommand;
	int counter;
	char *commands[64];
};

char pwd();
void help();
void printAlways();
void readCommand(char * commandLine);
void exec_prog(char * t,char * init, int a,struct command_t * command);
int parseCommand(char * commandLine, struct command_t * command);
void changeDirectory(char *path);
void signalHandler(int signo);
struct command_t command;


int main()
{
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &signalHandler;

	char currentPath[BUFFER_SIZE];

	pid_t pidC;

	/* where am i now? its path. */
	getcwd(currentPath, sizeof(currentPath));


	/* for ctrl c handling */
	if((sigaction(SIGTERM, &sa, NULL) == -1) || sigaction(SIGINT, &sa, NULL) == -1)
	{
		printf("signal error.\n");
		exit(1);
	}


	while(flag)
	{
		pipe(fds);

		char temp[BUFFER_SIZE];
		char init[BUFFER_SIZE];

		printAlways();
		//readCommand(commandLine);
		fgets(commandLine,sizeof(commandLine),stdin);
		commandLine[strlen(commandLine) -1] = '\0';

		if(strcmp(commandLine,"\n") == 0) { 
			continue;
		}

		if (flag)
		{
			parseCommand(commandLine,&command);
		}

		if(strcmp(commandLine, "exit") == 0)
		{
			break;
		}
		else if(strcmp(commandLine, "help") == 0)
		{
			help();
		}
		else if (strcmp(commandLine, "pwd") == 0)
		{
			pwd();
		}
		else if (strcmp(commandLine, "cd") == 0)
		{
			changeDirectory(command.commands[1]);
		}
		else if(flag)
		{
			int i,t=0;
			for (i = 0; i <= command.counter; ++i)
			{
					// bu 3 command lerden biri gelmisse | gelmis 
					// olma ihtimalide var. ona bakiyoruz. 
				if (strcmp(command.commands[t], "|") == 0)
					t++;
				if ((strcmp(command.commands[t], "lsf") == 0) || 
					(strcmp(command.commands[t], "cat") == 0) ||
					(strcmp(command.commands[t], "wc") == 0) ||
					(strcmp(command.commands[t], "bunedu") == 0) )
				{
					strcpy(temp,currentPath);
					strcat(temp, "/");
					strcat(temp, command.commands[t]);
				}

				init[0] = '\0';
				if (command.commands[t+1] != NULL)
				{
					if ((strcmp(command.commands[t],"lsf") != 0) && 
						(strcmp(command.commands[t+1],"|") != 0))
					{
						t++;
						strcpy(init,command.commands[t]);
					}
				}


				if(t < command.countCommand)
					t++;
				else
					break;

				pidC = fork();
				if (pidC == 0)	// child
				{
					exec_prog(temp, init, i,&command);
					exit(1);
				}
				else 	// parent
				{
					close(fds[1]);
					wait(NULL);
				}

			}
		}

	}

	while(wait(NULL) > 0);

	if (flag == 0){
		printf("\nSIGTERM is handled.\n");
	}
}

void printAlways() {
	printf("gtu@gtu-gtushell:~$ ");
}

void readCommand(char * commandLine) {
	fgets(commandLine,sizeof(commandLine),stdin);
}

void help()
{
	printf("HELP DOCUMENTATION\n");
	printf("Supported Commends:\n");
	printf(" 1)lsf\n 2)pwd\n 3)cd\n 4)help\n 5)cat\n 6)wc\n 7)bunedu\n 8)exit\n" );
}

void signalHandler(int signumber){

	if (signumber == SIGTERM || signumber == SIGINT){
		flag = 0;
	}
}

int parseCommand(char * commandLine, struct command_t * command) 
{
	char * pch;
	pch = strtok (commandLine," ");
	int i=0;
	command->counter = 0;
	while (pch != NULL) {
		if(strcmp(pch,"|") == 0){
			command->counter++;
		}
		command->commands[i] = pch;
		pch = strtok (NULL, " ");
		i++;
	}

	command->countCommand = i;
	command->commands[++i] = NULL;
}

char pwd(){ 
	char pathname[BUFFER_SIZE];
	getcwd(pathname,sizeof(pathname));
	printf("%s\n",pathname);

}


void changeDirectory(char *path)
{
	char temp[BUFFER_SIZE], current[BUFFER_SIZE];

	strcpy(temp, path);
	if (path[0] != '/')
	{
		getcwd(current,sizeof(current));
		strcat(current,"/");
		strcat(current,temp);
		chdir(current);
	}
	else
		chdir(path);

}

void exec_prog(char * path,char * init, int i, struct command_t * command)
{

	if (command->counter > 0)
	{
		if ((i % 2) == 0)
		{
			close(fds[0]);
			dup2(fds[1], STDOUT_FILENO);
			close(fds[1]);
		}
		else
		{
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO);
			close(fds[0]);
		}
	}

	if (init[0] != '\0'){
		if (execlp(path,path,init,(char*)NULL) == -1){
			printf("Unknown command. Try help .\n");
			exit(EXIT_SUCCESS);
		}
	}
	else{
		if (execlp(path,path,(char*) NULL) == -1){
			printf("Unknown command. Try help .\n");
			exit(EXIT_SUCCESS);
		}
	}
}


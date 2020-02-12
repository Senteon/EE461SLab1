#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>

#define FG 0
#define BG 1

struct jobs
{
	pid_t pid1;
	pid_t pid2;
	pid_t pgid;
	int number;
	int state;
	char * command;
	int pipe;
	struct jobs * next;
};

typedef struct jobs job;

void CHLD_Handler(int sig);
void updateJob(pid_t pid);
void stopJob(pid_t pid);
void destroy(job * x);
void moveToList();

job * head = NULL;
job * tail = NULL;
job * currentFG = NULL;

pid_t yash;
int pipeSignal = -1;
int ampersandSignal = -1;
int currentFGOne = -1;
int currentFGTwo = -1;
int FGDestruction = 0;


int main()
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGCONT, SIG_IGN);

	signal(SIGCHLD, CHLD_Handler);

	char * input;
	yash = getpgid(0);
	while(1)
	{
		if (currentFG != NULL)
		{
			while (((currentFG -> pid1 != -1) || (currentFG -> pid2 != -1)));
		}
		if (FGDestruction == 1)
		{
			currentFG = NULL;
			FGDestruction = 0;
		}
		currentFGOne = 0;
		currentFGTwo = 0;
		input = readline("# ");
		if (input == NULL)
		{
			job * h = head;
			while (h != NULL)
			{
				kill(-1 * (h -> pgid), SIGKILL);
				printf("Killed %i.\n", h -> pgid);
				h = h -> next;
			}
			exit(0);
		}
		if (input == "") continue;
		char * inputCopy = malloc(sizeof(char) * 1000);
		strcpy(inputCopy, input);
		const char delimiter[2] = " ";
		char * token = strtok(input, delimiter);
		char * params[1000];
		char * paramsFile1[1000];
		char * paramsFile2[1000];
		job * f = head;
		while (f != NULL)
		{
			if (f -> pid1 == -1 && f -> pid2 == -1)
			{
				destroy(f);
				break;
			}
			f = f -> next;
		}

		int n = 0;
		while (token != NULL)
		{
			params[n] = token;
			token = strtok(NULL, delimiter);
			n++;
		}
		params[n] = NULL;

		int cpid1 = -1;
		int cpid2 = -1;
		int i = 0;
		int j = 0;
		int k = 0;

		int pipeFound = -1;
		int ampersandFound = -1;
		int foundFG = -1;
		int foundBG = -1;
		int foundJobs = -1;

		int inputIndex1 = -1;
		int outputIndex1 = -1;
		int errorIndex1 = -1;

		int inputIndex2 = -1;
		int outputIndex2 = -1;
		int errorIndex2 = -1;

		if (n == 0)
		{
			continue;
		}

		while (params[i] != NULL)
		{
			if (strcmp(params[i], "<") == 0)
			{
				inputIndex1 = i;
				i++;
			}
			else if (strcmp(params[i], ">") == 0)
			{
				outputIndex1 = i;
				i++;
			}
			else if (strcmp(params[i], "fg") == 0)
			{
				foundFG = 1;
				break;
			}
			else if (strcmp(params[i], "bg") == 0)
			{
				foundBG = 1;
				break;
			}
			else if (strcmp(params[i], "jobs") == 0)
			{
				foundJobs = 1;
				break;
			}
			else if (strcmp(params[i], "2>") == 0)
			{
				errorIndex1 = i;
				i++;
			}
			else if (strcmp(params[i], "&") == 0)
			{
				ampersandFound = 1;
				break;
			}
			else if (strcmp(params[i], "|") == 0)
			{
				pipeFound = 1;
				i++;
				break;
			}
			else
			{
				paramsFile1[j] = params[i];
				j++;
			}
			i++;
		}
		paramsFile1[j] = NULL;

		if (pipeFound == 1)
		{
			while (params[i] != NULL)
			{
				if (strcmp(params[i], "<") == 0)
				{
					inputIndex2 = i;
					i++;
				}
				else if (strcmp(params[i], ">") == 0)
				{
					outputIndex2 = i;
					i++;
				}
				else if (strcmp(params[i], "&") == 0)
				{
					ampersandFound = 1;
					break;
				}
				else if (strcmp(params[i], "2>") == 0)
				{
					errorIndex2 = i;
					i++;
				}
				else
				{
					paramsFile2[k] = params[i];
					k++;
				}
				i++;
			}
			paramsFile2[k] = NULL;
		}

		int pipeFD[2];
		pipe(pipeFD);

		if (foundJobs == 1)
		{
			job * x = head;
			char sign;
			char status[10];
			while (x != NULL)
			{
				if (x -> state == 0) strcpy(status, "Done");
				else if (x -> state == 1) strcpy(status, "Running");
				else if (x -> state == 2) strcpy(status, "Stopped");
				if (x == tail) sign = '+';
				else sign = '-';
				printf("[%i]%c %s		%s\n", x -> number, sign, status, x -> command);
				x = x -> next;
			}
			continue;
		}
		else if (foundFG == 1)
		{
			if (tail != NULL)
			{

			}
		}
		if (pipeFound == 1) pipeSignal = 1;
		else pipeSignal = 0;
		if (ampersandFound == 1) ampersandSignal = 1;
		else ampersandSignal = 0;

		if (ampersandFound == 1)
		{
			if (head == NULL)
			{
				head = malloc(sizeof(struct jobs));
				head -> number = 1;
				head -> state = 1;
				head -> next = NULL;
				head -> command = inputCopy;
				tail = head;
			}
			else
			{
				tail -> next = malloc(sizeof(struct jobs));
				int temp = tail -> number;
				tail = tail -> next;
				tail -> number = ++temp;
				tail -> state = 1;
				tail -> next = NULL;
				tail -> command = inputCopy;
			}
		}
		else
		{
			currentFG = malloc(sizeof(struct jobs));
			currentFG -> number = 1;
			currentFG -> state = 1;
			currentFG -> next = NULL;
			currentFG -> command = inputCopy;
		}

		job ** currentFGChild = &currentFG;

		cpid1 = fork();
		if (cpid1 == 0)
		{
			signal(SIGINT, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);
			signal(SIGTTOU, SIG_DFL);
			signal(SIGCONT, SIG_DFL);
			int inputFD = -1;
			int outputFD = -1;
			int errorFD = -1;
			if (inputIndex1 != -1) inputFD = open(params[inputIndex1 + 1], O_RDONLY);
			if (outputIndex1 != -1)
			{
				outputFD = open(params[outputIndex1 + 1], O_WRONLY);
				if (outputFD == -1) outputFD = creat(params[outputIndex1 + 1], 0644);
			}
			if (errorIndex1 != -1)
			{
				errorFD = open(params[errorIndex1 + 1], O_WRONLY);
				if (errorFD == -1) errorFD = creat(params[errorIndex1 + 1], 0644);
			}
			if (inputFD != -1) dup2(inputFD, STDIN_FILENO);
			if ((outputFD != -1)) dup2(outputFD, STDOUT_FILENO);
			else if (pipeFound == 1) dup2(pipeFD[1], STDOUT_FILENO);
			if (errorFD != -1) dup2(errorFD, STDERR_FILENO);

			close(pipeFD[0]);
			close(pipeFD[1]);
			if (execvp(paramsFile1[0], paramsFile1) == -1)
			{
				if (currentFG != NULL)
				{
					//free((*currentFGChild) -> command);
					//free(*currentFGChild);
					*currentFGChild = NULL;
				}
				kill(getpid(), SIGKILL);
			}
		}

		if ((pipeFound == 1) && (cpid1 != 0))
		{
			cpid2 = fork();
			if (cpid2 == 0)
			{
				signal(SIGINT, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);
				signal(SIGTTOU, SIG_DFL);
				signal(SIGCONT, SIG_DFL);
				int inputFD = -1;
				int outputFD = -1;
				int errorFD = -1;
				if (inputIndex2 != -1) inputFD = open(params[inputIndex2 + 1], O_RDONLY);
				if (outputIndex2 != -1)
				{
					outputFD = open(params[outputIndex2 + 1], O_WRONLY);
					if (outputFD == -1) outputFD = creat(params[outputIndex2 + 1], 0644);
				}
				if (errorIndex2 != -1)
				{
					errorFD = open(params[errorIndex2 + 1], O_WRONLY);
					if (errorFD == -1) errorFD = creat(params[errorIndex2 + 1], 0644);
				}
				if (inputFD != -1) dup2(inputFD, STDIN_FILENO);
				else if (pipeFound == 1) dup2(pipeFD[0], STDIN_FILENO);
				if (outputFD != -1) dup2(outputFD, STDOUT_FILENO);
				if (errorFD != -1) dup2(errorFD, STDERR_FILENO);

				close(pipeFD[0]);
				close(pipeFD[1]);
				if (execvp(paramsFile2[0], paramsFile2) == -1)
				{
					if (currentFG != NULL)
					{
						// free((*currentFGChild) -> command);
						// free(*currentFGChild);
						*currentFGChild = NULL;
					}
					kill(getpid(), SIGKILL);
				}
			}
		}

		close(pipeFD[0]);
		close(pipeFD[1]);

		if (cpid1 != 0 && cpid2 != 0)
		{
			if (cpid1 != -1) setpgid(cpid1, 0);
			if (cpid2 != -1) setpgid(cpid2, getpgid(cpid1));


			if (ampersandFound == 1)
			{
				tail -> pid1 = cpid1;
				tail -> pid2 = cpid2;
				tail -> pgid = getpgid(cpid1);
				tcsetpgrp(STDIN_FILENO, yash);
			}
			else
			{
				currentFG -> pid1 = cpid1;
				currentFG -> pid2 = cpid2;
				currentFG -> pgid = getpgid(cpid1);
				tcsetpgrp(STDIN_FILENO, getpgid(cpid1));
			}
		}
	}
	return 0;
}

void CHLD_Handler(int sig)
{
	int status;
	int pid;
	pid = waitpid(-1, &status, WNOHANG | WCONTINUED | WUNTRACED);
	if (WIFSIGNALED(status))
	{
		if (currentFG != NULL)
		{
			if (pid == currentFG -> pid1)
			{
				currentFG -> pid1 = -1;
				tcsetpgrp(STDIN_FILENO, yash);
			}
			else if (pid == currentFG -> pid2)
			{
				currentFG -> pid2 = -1;
				tcsetpgrp(STDIN_FILENO, yash);
			}
			else
			{
				updateJob(pid);
			}
		}
		else updateJob(pid);
	}
	if (WIFEXITED(status))
	{
		if (currentFG != NULL)
		{
			if (pid == currentFG -> pid1)
			{
				currentFG -> pid1 = -1;
				tcsetpgrp(STDIN_FILENO, yash);
			}
			else if (pid == currentFG -> pid2)
			{
				currentFG -> pid2 = -1;
				tcsetpgrp(STDIN_FILENO, yash);
			}
			else
			{
				updateJob(pid);
			}
		}
		else updateJob(pid);
	}
	if (WIFSTOPPED(status))
	{
		if (currentFG != NULL)
		{
			if (pid == currentFG -> pid1)
			{
				currentFG -> state = 2;
				moveToList();
				tcsetpgrp(STDIN_FILENO, yash);
			}
			else if (pid == currentFG -> pid2)
			{
				currentFG -> state = 2;
				moveToList();
				tcsetpgrp(STDIN_FILENO, yash);
			}
			else
			{
				stopJob(pid);
			}
		}
		else stopJob(pid);
	}
}

void updateJob(pid_t pid)
{
	job * x = head;
	while (x != NULL)
	{
		if (pid == x -> pid1)
		{
			x -> pid1 = -1;
			break;
		}
		else if (pid == x -> pid2)
		{
			x -> pid2 = -1;
			break;
		}
		x = x -> next;
	}
}

void stopJob(pid_t pid)
{
	job * x = head;
	while (x != NULL)
	{
		if (pid == x -> pid1)
		{
			x -> state = 2;
			break;
		}
		else if (pid == x -> pid2)
		{
			x -> state = 2;
			break;
		}
		x = x -> next;
	}
}

void moveToList()
{
	if (head == NULL)
	{
		head = malloc(sizeof(struct jobs));
		head -> number = 1;
		head -> state = currentFG -> state;
		head -> next = NULL;
		tail = head;
		tail -> command = malloc(sizeof(char) * 1000);
		strcpy(tail -> command, currentFG -> command);
		tail -> pid1 = currentFG -> pid1;
		tail -> pid2 = currentFG -> pid2;
		tail -> pgid = currentFG -> pgid;
	}
	else
	{
		tail -> next = malloc(sizeof(struct jobs));
		int temp = tail -> number;
		tail = tail -> next;
		tail -> number = ++temp;
		tail -> state = currentFG -> state;
		tail -> next = NULL;
		tail -> command = malloc(sizeof(char) * 1000);
		strcpy(tail -> command, currentFG -> command);
		tail -> pid1 = currentFG -> pid1;
		tail -> pid2 = currentFG -> pid2;
		tail -> pgid = currentFG -> pgid;
	}
	// free(currentFG -> command);
	// free(currentFG);
	currentFG -> pid1 = -1;
	currentFG -> pid2 = -1;
	FGDestruction = 1;
}
void destroy(job * x)
{
	job * f = head;
	job * prev;
	char sign;
	char status[10];
	strcpy(status, "Done");
	while (f != NULL)
	{
		if (f == x)
		{
			kill(-1 * (f -> pgid), SIGKILL);
			if (f == head)
			{
				if (f -> next == NULL)
				{
					head = NULL;
					tail = NULL;
					sign = '+';
					printf("[%i]%c %s		%s\n", x -> number, sign, status, x -> command);
					// free(f -> command);
					// free(f);
				}
				else
				{
					head = f -> next;
					sign = '-';
					printf("[%i]%c %s		%s\n", x -> number, sign, status, x -> command);
					// free(f -> command);
					// free(f);
				}
			}
			else if (f == tail)
			{
				prev -> next = NULL;
				tail = prev;
				sign = '+';
				printf("[%i]%c %s		%s\n", x -> number, sign, status, x -> command);
				// free(f -> command);
				// free(f);
			}
			else
			{
				prev -> next = f -> next;
				sign = '-';
				printf("[%i]%c %s		%s\n", x -> number, sign, status, x -> command);
				// free(f -> command);
				// free(f);
			}
		}
		prev = f;
		f = f -> next;
	}
}

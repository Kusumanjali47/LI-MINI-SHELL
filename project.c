#include "msh.h"

/*Global variables*/
int p_flag, exit_status, c_flag;
pid_t pid;
jobs *head = NULL;
char string[100];
char command[100];

/*parse function*/
int parse_function(char *buffer, char ***argu, int *arguc, int *array)
{
	int i_index = 0, j_index = 0, k_index = 0, p_count = 0, length;
	char *str = buffer;

	/*compute the length of the command*/
	length = strlen(str);

	/*storing argv position is 0*/
	array[k_index++] = 0;

	/*Running the loop upto length*/
	while (i_index <= length)
	{
		/*for the frst arg*/
		if (j_index == 0)

			/*allocating the memory*/
			*argu = calloc(1, sizeof(char *));

		else

			/*reallocating the memory*/
			*argu = realloc(*argu, (j_index + 1) * sizeof(char *));

		/*checking for space and null character*/
		if (buffer[i_index] == ' ' || buffer[i_index] == '\0')
		{
			/*storing the null character*/
			buffer[i_index] = '\0';

			/*checking for the pipe*/
			if ((strcmp(str, "|")) == 0)
			{
				/*incrementing the count*/
				p_count++;

				/*storing the position*/
				array[k_index++] = j_index + 1;

				/*storing the null*/
				*(*argu + j_index) = NULL;
			}	
			else
			{
				/*allocating the memory*/
				*(*argu + j_index) = malloc(strlen(str) + 1);

				/*copying the string*/
				strcpy(*(*argu + j_index), str);
			}
			/*increment index*/
			j_index++;

			/*storing the next address*/
			str = buffer + i_index + 1;
		}
		/*incremt the index*/
		i_index++;
	}

	*arguc = j_index;

	/*reallocating the memory*/
	*argu = realloc(*argu, (j_index + 1) * sizeof(char *));

	/*storing the null*/
	*(*argu + j_index) = NULL;

	/*returning the count*/
	return p_count;
}



/*Function for builtin functions*/
int calling_system_calls(char **argu, char *buff, int *array)
{
	/*checking for the pwd*/
	if (strcmp(argu[0], "pwd") == 0)
	{
		/*Invoking the system call*/
		getcwd(buff, 100);
		printf("%s\n", buff);
	}

	/*checking for the mkdir to create the directory*/
	else if (strcmp(argu[0], "mkdir") == 0)
	{
		/*Invoking the system call*/
		mkdir(buff + 6, 0777);
	}
	/*Removing the directory*/
	else if (strcmp(argu[0], "rmdir") == 0)
	{
		/*Invoking the system call*/
		rmdir(buff + 6);
	}
	/*changing the directory*/
	else if (strcmp(argu[0], "cd") == 0)
	{
		if (argu[1])
		{
			chdir(buff + 3);
		}
		else 
		{
			chdir("/home/emertxe/");
		}
	}
	else
		return 0;

	return 1;
}


/*Function for builtin functions*/
int check_echo(char **argu, int *array, char **env)
{
	int i_index = 0;

	/*checking for the echo*/
	if (strcmp(argu[0], "echo") == 0)
	{
		if (argu[1] == NULL)
			printf("\n");
		
		/*printing the pid*/
		else if (strcmp(argu[1], "$$") == 0)
			printf("%d\n", getpid());
		
		/*printing the status*/
		else if (strcmp(argu[1], "$?") == 0 || strcmp(argu[1], "$#") == 0)
			printf("%d\n", WEXITSTATUS(exit_status));
		
		/*printing the bash path*/
		else if (strcmp(argu[1], "$SHELL") == 0)
			printf("%s\n", string);
		
		else
			return 0;

		return 1;
	}
	return 0;
}


/*Function for new Prompt*/
int new_prompt(char *buff, char *newshell)
{
	/*creating the new prompt*/
	if (strncmp(buff, "PS1=", 4) == 0)
	{
		if (buff[4] != ' ' && buff[4] != '\t')
			/*copying the new prompt*/
			strcpy(newshell, buff + 4);
		else
			printf("Invalid command\n");
		return 1;
	}
	return 0;
}


/*Function for back to prompt*/
int back_to_prompt(char *buff)
{
	/*if the flag is not set and strlen is 0*/
	if (p_flag || strlen(buff) == 0)
	{
		if (p_flag)
		{
			/*printing the new line*/
			printf("\n");
			p_flag = 0;
		}
		return 1;
	}
	return 0;
}


/*Function for checking the priority*/
void maintain_priority(void)
{
	jobs *ptr = head, *last = head, *ptr_1 = head;

	/*If the only one node is present*/
	if (ptr->next == NULL)
	{
		ptr->Priority = '+';
		return;
	}

	/*Running the loop untill the last node*/
	while (ptr->next != NULL)
	{
		/*clearing the priority*/
		ptr->Priority = ' ';
		ptr = ptr->next;
	}

	/*clearing the last node*/
	ptr->Priority = ' ';

	/*assigning ptr to the last*/
	last = ptr;

	/*checking for the first stopped state*/
	while (ptr->prev != NULL && (strcmp(ptr->state, "Stopped") != 0))
		ptr = ptr->prev;

	if (ptr != NULL)
	{
		/*Assigning the first priority*/
		ptr->Priority = '+';

		ptr = ptr->prev;

		/*checking for the second stopped state*/
		while (ptr_1->prev != NULL && strcmp(ptr->state, "Stopped") != 0)
			ptr = ptr->prev;


		if (ptr != NULL)
			/*Assigning the second priority*/
			ptr->Priority = '-';
		else
		{
			ptr_1 = last;

			/*If the second stop is not present and checking for the second running*/
			while (ptr_1->prev != NULL && strcmp(ptr_1->state, "Running") != 0)
				ptr_1 = ptr_1->prev;

			if (ptr_1 != NULL)
				/*Assigning the second priority*/
				ptr_1->Priority = '-';

		}
	}
	/*If there is no stopped states*/
	else
	{
		/*checking for the first running*/
		while (last->prev != NULL && (strcmp(last->state, "Running") != 0))
			last = last->prev;

		if (last != NULL)
		{
			/*Assigning the first priority*/
			last->Priority = '+';

			last = last->prev;

			/*checking for the second running*/
			while (last->prev != NULL && (strcmp(last->state, "Running") != 0))
				last = last->prev;

			if (last != NULL)
				/*Assigning the second priority*/
				last->Priority = '-';
		}
	}
}


/*Function for inserting the jobs in the list*/
void insertJob(char *string)
{
	/*If the list is empty*/
	if (head == NULL)
	{
		/*allocating the memory*/
		head = malloc(sizeof(jobs));

		/*process number*/
		head->Pno = 1;

		/*pid*/
		head->pid = pid;

		/*assigning next and prev as null*/
		head->next = NULL;
		head->prev = NULL;

		/*process name*/
		strcpy(head->Pname, command);

		/*process state*/
		strcpy(head->state, string);
		
		/*checking if the process is in the running state or not*/
		if (strcmp(head->state, "Running") == 0)
			//if it is a background process storing &
			strcat(head->Pname, "  &");

		/*priority*/
		head->Priority = '+';
	}
	else
	{
		jobs *last;	
		last = head;

		/*Running upto the last node*/
		while (last->next != NULL)
			last = last->next;

		/*allocating the memory*/
		last->next = malloc(sizeof(jobs));


		last->next->prev = last;
		last->next->next = NULL;
		
		/*process number*/
		last->next->Pno = (last->Pno) + 1;

		/*process name*/
		strcpy(last->next->Pname, command);

		/*process state*/
		strcpy(last->next->state, string);
		
		/*checking if the process is in the running state or not*/
		if (strcmp(last->next->state, "Running") == 0)
			//if it is a background process storing &
			strcat(last->next->Pname, "  &");

		/*pid*/
		last->next->pid = pid;

		/*priority*/
		last->next->Priority = ' ';

		/*Maintaining the priority*/
		maintain_priority();

	}
}


/*Function for deleting the job*/
void del_job(pid_t pid)
{
	/*If the no process is present*/
	if (head == NULL)
		printf("NO Process is available\n");
	else
	{
		jobs *ptr;
		ptr = head;

		/*Running until the pid found*/
		while (ptr != NULL && ptr->pid != pid)
			ptr = ptr->next;

		if (ptr == NULL)
			printf("process is not found\n");

		/*updating*/
		if (ptr != head)
			ptr->prev->next = ptr->next;

		else
			head = (head)->next;

		/*freeing the memory of that deleted job*/
		free(ptr);
	}

	/*Maintaining the priority*/
	if (head != NULL)
		maintain_priority();
}


/*Function for printing the jobs*/
void print_jobs(void)
{
	/*If the list is empty and return it*/
	if (head == NULL)
		return;
	else
	{
		jobs *ptr;
		ptr = head;

		/*Running upto the last node*/
		while (ptr != NULL)
		{
			/*printing the job details*/
			printf("[%d]%c\t%s\t\t\t%s\n", ptr->Pno, ptr->Priority, ptr->state, ptr->Pname);
			ptr = ptr->next;
		}

		/*updating the ptr*/
		ptr = head;

		/*checking for the exit node*/
		while (ptr != NULL)
		{
			if (strncmp(ptr->state, "Exit", 4) == 0)
			{
				/*deleting the job*/
				del_job(ptr->pid);
			}
			ptr = ptr->next;
		}
	}
}


/*Function for the fg bg process*/
int fg_bg(void)
{
	jobs *ptr = head;
	int length, i_index;

	/*if the fg command is passed*/
	if (strcmp(command, "fg") == 0)
	{
		/*checking if the jobs or available or not*/
		if (ptr == NULL)
			printf("fg: No jobs available.\n");
		else
		{
			/*checking for the first priority job*/
			while (ptr != NULL && ptr->Priority != '+')
				ptr = ptr->next;

			if (ptr != NULL)
			{
				/*clearing the flag*/
				c_flag = 0;

				/*updating the pid*/
				pid = ptr->pid;

				/*computing the length of the process name*/
				length = strlen(ptr->Pname);

				/*checking for the back ground process*/
				if (ptr->Pname[length - 1] != '&' && strcmp(command, "fg") == 0)
				{
					strcpy(ptr->state, "Running");
					printf("%s\n", ptr->Pname);
				}
				else
				{
					strcpy(ptr->state, "Running");

					for (i_index = 0; i_index < length - 1; i_index++)
					{
						printf("%c", ptr->Pname[i_index]);
					}
					printf("\n");
				}

				/*passing the continue signal to the pid*/
				kill(ptr->pid, SIGCONT);

				//waiting the process to complete
				while (!c_flag);

				/*clearing the flag*/
				c_flag = 0;
			}
		}
	}

	/*checking for the bg command*/
	else if (strcmp(command, "bg") == 0)
	{
		/*Handling the error if the job list is empty*/
		if (head == NULL)
			printf("bg: No Back Ground jobs.\n");

		/*If the list is not empty*/
		if (head != NULL)
		{
			/*checking for the first priority process*/
			while (ptr != NULL && ptr->Priority != '+')
				ptr = ptr->next;

			if (ptr != NULL)
			{
				/*updating the status and process name*/
				strcpy(ptr->state, "Running");
				strcat(ptr->Pname, "  &");

				/*printing the job details*/
				printf("[%d]%c\t\t%s\n", ptr->Pno, ptr->Priority, ptr->Pname);

				/*passing the signal to the pid*/
				kill(ptr->pid, SIGCONT);
			}
			else
			{
				printf("bg : No Back Ground jobs.\n");
			}
		}
	}
	/*checking for the jobs command*/
	else if (strcmp(command, "jobs") == 0)
		print_jobs();
	else
		return 0;

	return 1;
}



/*Function for the child handler*/
void child_signal_handler(int signum, siginfo_t *sinfo, void *context)
{
	jobs *ptr_1 = head;
	char dest[20];

	/*if the child process is exited*/
	if (sinfo->si_code == CLD_EXITED)
	{
		/*printing the status into the buffer*/
		sprintf(dest, "%d", sinfo->si_status);

		/*checking for the pid*/
		while (ptr_1 != NULL && ptr_1->pid != pid)
			ptr_1 = ptr_1->next;

		if (ptr_1 != NULL)
		{
			/*updating the status*/
			strcpy(ptr_1->state, "Exit  ");
			strcat(ptr_1->state, dest);
		}

		/*setting the flag*/
		c_flag = 1;
	}

	/*if the child process is killed*/
	else if (sinfo->si_code == CLD_KILLED)
	{
		/*checking for the pid*/
		while (ptr_1 != NULL && ptr_1->pid != pid)
			ptr_1 = ptr_1->next;

		if (ptr_1 != NULL)
			/*deleting the jobs from the jobs list*/
			del_job(ptr_1->pid);

		/*setting the flag*/
		c_flag = 1;
	}

	/*if the child process is stopped*/
	else if (sinfo->si_code == CLD_STOPPED)
	{
		/*checking for the pid*/
		while (ptr_1 != NULL && ptr_1->pid != pid)
			ptr_1 = ptr_1->next;

		if (ptr_1 != NULL)
			/*updating the status*/
			strcpy(ptr_1->state, "Stopped");
		else
		{
			/*Inserting the jobs as stopped state*/
			insertJob("Stopped");
			ptr_1 = head;

			/*Running upto the last node*/
			while (ptr_1->next != NULL)
				ptr_1 = ptr_1->next;
		}

		/*printing the job details*/
		printf("[%d]%c\t%s\t\t\t%s\n", ptr_1->Pno, ptr_1->Priority, ptr_1->state, ptr_1->Pname);

		/*setting the flag*/
		c_flag = 1;
	}

	/*if the child process is continued after from the stopped state*/
	else if (sinfo->si_code == CLD_CONTINUED)
	{
		/*claering the flag*/
		c_flag = 0;

		ptr_1 = head;

		/*Running upto the last node*/
		while (ptr_1 != NULL && ptr_1->pid != sinfo->si_pid)
			ptr_1 = ptr_1->next;
	}
}

/*signal handler for  the cntrl + c signal*/
void signal_handler(int signum)
{
	/*checking for the pid*/
	if (pid != 0)
	{
		/*passing the signal to the pid to the kill process*/
		kill(pid, SIGKILL);
		p_flag = 1;
	}
	else
	{
		/*clearing the flags*/
		p_flag = 0;
	}
	printf("\n");
}

/*signal handler for the cntrl + z signal*/
void q_signal_handler(int signum)
{
	/*passing the signal to the pid*/
	kill(pid, SIGSTOP);
	printf("\n");
}


/*Main Function*/
int main(int argc, char **argv, char **env)
{
	int no_pipe;
	char shell[100] = "MiniShell";
	int index_array[100], i_index = 0;	
	*argv = NULL;
	jobs *ptr = head;

	/*invoking the function call to store my shell path*/
	getcwd(string, 100);


	/*Declaring the variables*/
	struct sigaction act_1, act_2, act_3;

	/*claering the memory*/
	memset(&act_1, 0, sizeof(act_1));
	memset(&act_2, 0, sizeof(act_2));
	memset(&act_3, 0, sizeof(act_3));

	act_1.sa_handler = signal_handler;

	act_3.sa_flags = SA_SIGINFO;
	act_3.sa_sigaction = child_signal_handler;

	/*passing the signals*/
	sigaction(SIGINT, &act_1, NULL);
	sigaction(SIGCHLD, &act_3, NULL);

	printf(CYAN "\n\n**************************************************\n\n" RESET);
	printf(RED "\n\nWELCOME TO MINI SHELL\n\n" RESET);
	printf(CYAN "\n\n**************************************************\n\n" RESET);

	while (1)
	{
		/*ignoring the sigstop signal*/
		act_2.sa_handler = SIG_IGN;
		sigaction(SIGTSTP, &act_2, NULL);

		/*filling the memory with 0*/
		memset(&command, 0, sizeof(command));

		/*reading the command*/
		printf(MAGENTA "<%s>$" GREEN, shell);
		scanf("%[^\n]", command);
		__fpurge(stdin);

		/*To exit the program*/
		if (strcmp(command, "exit") == 0)
			break;

		/*Assigning the sigstop signal handler*/
		act_2.sa_handler = q_signal_handler;
		sigaction(SIGTSTP, &act_2, NULL);

		/*invoking the back to prompt and new prompt and fg_bg process functions*/
		if(back_to_prompt(command) || new_prompt(command, shell) || fg_bg())
			continue;


		/*calling the parse function and storing the number of pipes*/
		no_pipe = parse_function(command, &argv, &argc, index_array);

		/*decaring the pipe*/
		int p[no_pipe][2];

		/*if no pipe are passed in the command*/
		if (no_pipe == 0)
		{
			/*Invoking the built in commands function*/
			if (calling_system_calls(argv, command, index_array) || check_echo(argv, index_array, env))
				continue;

			/*creating the child*/
			pid = fork();

			switch (pid)
			{
				/*on error return -1*/
				case -1:

					printf("Fork() : Failed\n");
					exit(0);

					/*on creating the child it returns 0 itself*/
				case 0:

					/*if the process is back ground process*/
					if (strcmp(argv[argc - 1],  "&") == 0)
					{
						argv[argc - 1] = NULL;
						
						/*Ignoring the sigstop and sigint signals*/
						act_1.sa_handler = SIG_IGN;
						sigaction(SIGINT, &act_1, NULL);
						act_2.sa_handler = SIG_IGN;
						sigaction(SIGTSTP, &act_2, NULL);

						/*executing the command*/	
						if ((execvp(argv[index_array[0]], argv + index_array[0])) == -1)
						{
							printf("Invalid Command\n");
							exit(-1);
						}
					}
					else
					{
						/*executing the command*/	
						if ((execvp(argv[index_array[0]], argv + index_array[0])) == -1)
						{
							printf("Invalid Command\n");
							exit(-2);
						}
					}
					exit(1);

					/*on creating the child it returns child pid to the parent process*/
				default:

					/*comapring the if the process is back ground process or not*/
					if (strcmp(argv[argc - 1], "&") == 0)
					{
						/*insert the job*/
						insertJob("Running");
						ptr = head;

						/*Running untill the last node*/
						while (ptr->next != NULL)
							ptr = ptr->next;

						/*printing the details*/
						if (ptr != NULL)
							printf("[%d] %d\n", ptr->Pno, ptr->pid);

						/*setting process id as 0*/
						pid = 0;

					}
					else
					{
						/*waiting untill the process completes*/
						while (!c_flag);

						/*clearing the flag*/
						c_flag = 0;
					}
			}
		}
		else
		{
			dup2(0, 77);

			/*for loop to run upto the no of pipes*/
			for (i_index = 0; i_index < no_pipe + 1; i_index++)
			{
				/*creating the pipes and handling the errors*/
				if (i_index != no_pipe && pipe(p[i_index]) == -1)
				{
					perror("pipe");
					exit(1);
				}

				/*creating the child*/
				pid = fork();

				switch (pid)
				{
					//on error return -1
					case -1:
						printf("Fork() : Failed\n");
						exit(0);

						/*on creating the child, returns 0 itself*/		
					case 0:

						/*if the index is not equal to no_pipe*/
						if (i_index != no_pipe)
						{
							/*closing the read end*/
							close(p[i_index][0]);

							/*writing the output into the write end*/
							dup2(p[i_index][1], 1);
						}

						/*executing the commnad*/
						execvp(argv[index_array[i_index]], argv + index_array[i_index]);
						exit(0);

					default:

						if (i_index != no_pipe)
						{
							/*closing the read end*/
							close(p[i_index][1]);

							/*readirecting the stdin to the read end*/
							dup2(p[i_index][0], 0);
						}
						else
							dup2(77, 0);
				}
			}
		}
		fflush(stdout);
		printf(RESET);
	}

	printf(BLUE "\n\nMINI SHELL CLOSED\n\n" RESET);

	return 0;
}


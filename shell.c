// Unix shell project

// Header files
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

// Global variables
char *output_redirection_file;
char *output_redirection_file_2;
int redirection_flag = 0;
int redirection_flag_2 = 0;
int redirection_error = 0;
char *cmd_exec[100];
char error_message[30] = "An error has occurred\n";

// This function gets the user input
int getUserInput(char *input)
{
	// char* buffer;
	// size_t bufsize = 32;
	// size_t characters;
	// buffer = (char*)malloc(bufsize * sizeof(char));
	// if (buffer == NULL) {
	// 	write(STDERR_FILENO, error_message, strlen(error_message));
	// 	exit(1);
	// }
	// printf("witsshell> ");
	// characters = getline(&buffer, &bufsize, stdin);
	// if (strlen(buffer) != 0) {
	// 	buffer[strcspn(buffer, "\n")] = 0;
	// 	strcpy(input, buffer);
	// 	return 0;
	// }
	// else {
	// 	return 1;
	// }

	char *buffer;

	buffer = readline("witsshell> ");
	if (strlen(buffer) != 0)
	{
		add_history(buffer);
		strcpy(input, buffer);
		return 0;
	}
	else
	{
		return 1;
	}
}

// This function is used to execute commands as well as handle redirection
void executeCommands(char **args)
{
	pid_t pid = fork();

	if (pid == -1)
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(0);
	}
	else if (pid == 0)
	{
		if (redirection_flag == 1)
		{
			if (redirection_flag_2 == 1)
			{
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
			else
			{
				if (output_redirection_file == NULL)
				{
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
				else
				{
					int output_fd = open(output_redirection_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if (output_fd == -1)
					{
						write(STDERR_FILENO, error_message, strlen(error_message));
						redirection_flag = 0;
					}
					else
					{

						dup2(output_fd, 1);
						if (execvp(args[0], args) < 0)
						{
							write(STDERR_FILENO, error_message, strlen(error_message));
						}
						close(output_fd);
						redirection_flag = 0;
					}
				}
			}
		}
		else if (execvp(args[0], args) < 0)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		exit(0);
	}
	else
	{
		wait(NULL);
		return;
	}
}

// This function is used to execute piped commands
void executePipedCommands(char **args, char **piped_args)
{
	int pipes[2];
	pid_t p1, p2;

	if (pipe(pipes) < 0)
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}
	p1 = fork();
	if (p1 < 0)
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(0);
	}

	if (p1 == 0)
	{

		if (execvp(args[0], args) < 0)
		{
			exit(0);
		}
	}
	else
	{
		p2 = fork();
		if (p2 < 0)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(0);
		}
		if (p2 == 0)
		{
			if (execvp(piped_args[0], piped_args) < 0)
			{
				exit(0);
			}
		}
		else
		{
			wait(NULL);
			wait(NULL);
		}
	}
}

// This function handles the required built-in commands
int builtInCommands(char **args)
{
	int which_command = 0;
	char *built_in_commands[3];

	built_in_commands[0] = "exit";
	built_in_commands[1] = "cd";
	built_in_commands[2] = "path";

	for (int i = 0; i < 3; i++)
	{
		if (strcmp(args[0], built_in_commands[i]) == 0)
		{
			which_command = i + 1;
			break;
		}
	}

	if (which_command == 1)
	{
		if (args[1] != NULL)
		{
			// Nothing to be done
		}
		else
		{
			exit(0);
		}
	}
	else if (which_command == 2)
	{
		if (args[1] == NULL)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		else
		{
			chdir(args[1]);
			if (chdir(args[1]) == -1)
			{
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(0);
			}
		}
	}
	else if (which_command == 3)
	{
		if (args[1] == NULL)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		else
		{
			chdir(args[1]);
			if (chdir(args[1]) == -1)
			{
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(0);
			}
		}
	}

	return 0;
}

// this function looks for the '&' character in the input
int findPipeChar(char *input, char **piped_input)
{
	int i;
	for (i = 0; i < 2; i++)
	{
		piped_input[i] = strsep(&input, "&");
		if (piped_input[i] == NULL)
			break;
	}

	if (piped_input[1] == NULL)
		return 0;
	else
	{
		return 1;
	}
}

// this function removes unnecessary spaces from the input
void removeWhiteSpace(char *input, char **args)
{
	for (int i = 0; i < 50; i++)
	{
		args[i] = strsep(&input, " ");
		if (args[i] == NULL)
			break;
		if (strlen(args[i]) == 0)
			i--;
	}
}

// This function processes user input
int parseString(char *input, char **args, char **piped_args)
{

	// used for redirection
	if (strchr(input, '>'))
	{
		char *tmp[100];
		char *error[100];
		tmp[0] = strtok(input, ">");
		tmp[1] = strtok(NULL, ">");
		tmp[2] = strtok(NULL, ">");
		if (tmp[2] != NULL)
		{
			output_redirection_file_2 = tmp[2];
			redirection_flag_2 = 1;
		}
		error[1] = strtok(tmp[1], " ");
		error[2] = strtok(NULL, " ");
		if (error[2] != NULL)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
			return 0;
		}
		output_redirection_file = tmp[1];
		removeWhiteSpace(tmp[0], args);
		redirection_flag = 1;
		return 1;
	}

	char *piped_input[2];
	int isPiped = 0;

	isPiped = findPipeChar(input, piped_input);

	if (isPiped)
	{
		removeWhiteSpace(piped_input[0], args);
		removeWhiteSpace(piped_input[1], piped_args);
	}
	else
	{
		removeWhiteSpace(input, args);
	}

	if (builtInCommands(args))
	{
		return 0;
	}
	else
	{
		return 1 + isPiped;
	}
}

int main(int MainArgc, char *MainArgv[])
{
	char user_input[500];
	char *args[50];
	char *piped_args[50];
	int execution_flag = 0;

	if (MainArgc == 1)
	{
		while (1)
		{
			if (getUserInput(user_input))
			{
				continue;
			}
			if (strcmp(user_input, "&") == 0)
			{
				// write(STDERR_FILENO, error_message, strlen(error_message));
			}
			else
			{
				execution_flag = parseString(user_input, args, piped_args);

				if (execution_flag == 1)
					executeCommands(args);

				if (execution_flag == 2)
					executePipedCommands(args, piped_args);
			}
		}
	}
	else if (MainArgc > 2)
	{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	else
	{

		char *buffer;
		size_t bufsize = 32;
		size_t characters;

		FILE *input_file;

		input_file = fopen(MainArgv[1], "r");
		if (input_file == NULL)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
		}

		characters = getline(&buffer, &bufsize, input_file);

		ssize_t len;

		buffer[strcspn(buffer, "\n")] = 0;
		strcpy(user_input, buffer);

		if (strcmp(user_input, "&") == 0)
		{
			// write(STDERR_FILENO, error_message, strlen(error_message));
		}
		else
		{

			execution_flag = parseString(user_input, args, piped_args);
			if (execution_flag == 1)
			{
				executeCommands(args);
			}
			if (execution_flag == 2)
			{
				executePipedCommands(args, piped_args);
			}
		}

		while ((len = getline(&buffer, &bufsize, input_file)) != -1)
		{
			if (strlen(buffer) != 0)
			{
				buffer[strcspn(buffer, "\n")] = 0;
				strcpy(user_input, buffer);
			}
			execution_flag = parseString(user_input, args, piped_args);
			if (execution_flag == 1)
			{
				executeCommands(args);
			}
			if (execution_flag == 2)
			{
				executePipedCommands(args, piped_args);
			}
		}
		fclose(input_file);
	}

	return 0;
}
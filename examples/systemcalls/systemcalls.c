#include "systemcalls.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
	int status = system(cmd);//system() = fork() + exec() + wait()
	
	if(status == -1){//fork() fails
		perror("system");
		return false;
	}
	else{
		if(WIFEXITED(status)){//check if child terminated with exit() correctly
			int exit_code = WEXITSTATUS(status);//if child exit, the get its code
			//printf("Command exited with status %d\n", exit_code);

			if(exit_code != 0){
				fprintf(stderr, "Command failed\n");
				return false;
			}
		}
		else{
			fprintf(stderr, "Command did not exit normally");
			return false;
		}
	}

    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{	
	if(count < 1) return false;

    va_list args;
    va_start(args, count);

    char * command[count+1];
    for(int i=0; i<count; i++){
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;//exec  requires NULL-terminated array

/*
 * TODO
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    va_end(args);

	pid_t pid;
	int status;

	switch(pid = fork()){
		case -1:
			perror("fork failed");
			return false;

		case 0:
			//child process
			execv(command[0], command);
			//if do the following line means execv does not replace current process
			perror("execv failed");
			_exit(127);

		default:
			//parent process
			if(waitpid(pid, &status, 0) == -1){
				perror("waitpid failed");
				return false;
			}
			//WIFEXITED return non-zero value -> normal termination (not killed by signal or else)
			//if right part is true then do left part WEXITSTATUS to check the program's exit code -> return value of main() or exit()
			if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
				return true;
			}
			else{
				fprintf(stderr, "Child process exited with status %d\n", WEXITSTATUS(status));
				return false;
			}
	}
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);

	pid_t pid;
	//I/O descriptors should be assigned before calling execv
	int fd = open(outputfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);

	switch(pid = fork()){
		case -1:
			perror("fork");
			return false;
		case 0:
			//child process
			if(dup2(fd, STDOUT_FILENO) < 0){//STDOUT_FILENO = 1
				perror("dup2");
				_exit(1);
			}
			close(fd);

			execv(command[0], command);
			perror("execv");
			_exit(2);

		default:
			//parent process
			close(fd);
			int status;
			if(waitpid(pid, &status, 0) == -1){
				perror("waitpid");
				return false;
			}
			
			return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	}

    return true;
}

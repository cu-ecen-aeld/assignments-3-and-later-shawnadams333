#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "systemcalls.h"

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
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    bool retval = true;

    if (cmd == NULL)
    {
        retval = false;
    }
    else
    {
        int status = system(cmd);

        if (status != 0)
        {
            retval = false;
        }
    }

    return retval;
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
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

/*
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    bool retval = true;
    pid_t pid;
    pid_t rc;

    pid = fork();

    printf("\n");

    if (pid == -1) {
        retval = false;
    }
    else if (pid == 0) {
        /* child process */
        char** const ptr_to_const_arr = &command[0];
        printf("child: running %s\n", command[0]);
        rc = execv(command[0], ptr_to_const_arr);

        /* we only get here if there was an error in execv() */
        printf("child: execv() error %d, errno: %d\n", (int)rc, errno);
        exit(rc);
    }

    /* parent process runs this */
    int status = 0;
    rc = waitpid(pid, &status, 0);

    printf("parent: waitpid() returned %d\n", (int)rc);
    printf("parent: status: %x\n", status);
    printf("parent: wifexited(status): %x\n", (int)WIFEXITED(status));
    printf("parent: wexitstatus(status): %x\n", (int)WEXITSTATUS(status));

    if (rc < 0) {
        printf("waitpid() returned an error\n");
        retval = false;
    }
    else if (WEXITSTATUS(status) != 0) {
        printf("parent: child exit status: %d\n", WEXITSTATUS(status));
        retval = false;
    }

    printf("\n");

    va_end(args);

    return retval;
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
    bool retval = true;

    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);

    if (fd < 0) {
        printf("Error opening file: %s: %d\n", outputfile, errno);
        retval = false;
    }
    if (dup2(fd, 1) < 0) {
        printf("Error redirecting stdout: %d\n", errno);
        retval = false;
    }

    pid_t rc;

    pid_t pid = fork();

    printf("\n");

    if (pid == -1) {
        retval = false;
    }
    else if (pid == 0) {
        /* child process */
        char** const ptr_to_const_arr = &command[0];
        printf("child: running %s\n", command[0]);
        rc = execv(command[0], ptr_to_const_arr);

        /* we only get here if there was an error in execv() */
        printf("child: execv() error %d, errno: %d\n", (int)rc, errno);
        exit(rc);
    }

    /* parent process runs this */
    int status = 0;
    rc = waitpid(pid, &status, 0);

    printf("parent: waitpid() returned %d\n", (int)rc);
    printf("parent: status: %x\n", status);
    printf("parent: wifexited(status): %x\n", (int)WIFEXITED(status));
    printf("parent: wexitstatus(status): %x\n", (int)WEXITSTATUS(status));

    if (rc < 0) {
        printf("waitpid() returned an error\n");
        retval = false;
    }
    else if (WEXITSTATUS(status) != 0) {
        printf("parent: child exit status: %d\n", WEXITSTATUS(status));
        retval = false;
    }

    if (fd > 0) {
        close(fd);
    }

    va_end(args);

    return retval;
}

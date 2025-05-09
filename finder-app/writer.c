/**
 * Write a string to a file. The string and filename are both supplied by the user.
 */

#include <syslog.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static const int FAIL = 1;
static const int SUCCESS = 0;
static const char* string = NULL;
static const char* filename = NULL;

int processArgs(int argc, char *argv[])
{
    int status = SUCCESS;

    if (argc < 3)
    {
        syslog(LOG_USER, "incorrect number of arguments: %d\n", argc);
        status = FAIL;
    }
    else
    {
        printf("argv[1]: %s, argv[2]: %s\n", argv[1], argv[2]);
        filename = argv[1];
        string = argv[2];
        printf("string: %s, filename: %s\n", string, filename);
        syslog(LOG_DEBUG, "string: %s, filename: %s\n", string, filename);
    }

    return status;
}


int writeString()
{
    int status = SUCCESS;

    int flags = O_CREAT | O_RDWR;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int fd = open(filename, flags, mode);

    if (fd < 0)
    {
        syslog(LOG_USER, "error opening file: %s\n", filename);
        status = FAIL;
    }
    else
    {
        size_t bytes = strlen(string);
        printf("number of bytes in the string: %zu\n", bytes);

        ssize_t written = write(fd, string, bytes);
 
        if (written < 0)
        {
            syslog(LOG_USER, "error writing %s to %s\n", string, filename);
            status = FAIL;
        }

        close(fd);
    }

    return status;
}


/**
 * program entrypoint
 */
int main(int argc, char *argv[])
{
    openlog(NULL, 0, LOG_USER);
 
    int status = processArgs(argc, argv);

    if (status == SUCCESS)
    {
        status = writeString();
    }

    return status;
}

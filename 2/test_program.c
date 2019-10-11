#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    int pid = fork();
    if (pid)
    {
        // child
        char *args[2];
        args[0] = "./process";
        args[1] = NULL;
        execvp(args[0], args);
    }
    else
    {
        // parent
        sleep(5);
        if (kill(pid, SIGINT))
        {
            fprintf(stderr, "terminate of %d failed", (int)pid);
        }
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        printf("\nSIGINT received: Interrupt signal detected.\n");
    }
    else if (signal == SIGTERM)
    {
        printf("\nSIGTERM received: Termination requested.\n");
        printf("Exiting program...\n");
        exit(0);
    }
    else if (signal == SIGUSR1)
    {
        printf("\nSIGUSR1 received: User-defined event detected.\n");
    }
}

int main()
{
    printf("Signal Handling Program Started\n");
    printf("Process ID: %d\n", getpid());

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);

    printf("Waiting for signals...\n");

    while (1)
    {
        pause();
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

char buffer[100];
int p[2];

void handler(int s)
{
    if (s == SIGUSR1)
    {
        int num = (rand() % 3) + 1;
        sprintf(buffer, "%d", num);
        write(p[1], buffer, sizeof(buffer));
    }
    if (s == SIGUSR2)
    {
        printf("Received Signal to exit from Master\n");
        exit(0);
    }
}

void execChild(int *pipePassed)
{
    signal(SIGUSR1, handler); // define SIGUSR1 to be the signal for getting the random number
    signal(SIGUSR2, handler); // define SIGUSR2 to be the signal to end the process
    p[0] = pipePassed[0];
    p[1] = pipePassed[1];
    srand((unsigned int)time(NULL) + p[1]); // adding file descriptors to the seed so that the seed for both child is different and random output of both of them is different
    close(p[0]);
    while (true) // run the program until it receives signal from master
    {
    }
}

int getMove(int p)
{
    int x;
    read(p, buffer, sizeof(buffer));
    x = atoi(buffer);
    return x;
}

int main()
{
    pid_t processC, processD; // PIDs for Child C and Child D
    int pipeC[2], pipeD[2];   // Pipeline for Process C and D
    pipe(pipeC);
    pipe(pipeD);
    processC = fork(); // create the child process C
    if (processC)      // Parent is executed
    {
        processD = fork(); // create the child D from the parent
        if (processD)      // again, it's Master, so close the writing end of the pipe
        {
            close(pipeC[1]);
            close(pipeD[1]);
        }
        else // Process D is being executed
        {
            execChild(pipeD); // execute the child Process
            perror(0);
        }
    }
    else // Process C is being executed
    {
        execChild(pipeC); // execute the child Process
        perror(0);
    }

    /* Parent Process will get executed below */

    printf("Master Process will sleep for 1 second before receiving signals so that Child C and D get executed in parallel\n\n");
    sleep(1);
    srand((unsigned int)time(NULL)); // seed the rand function with the current time
    int moveC, moveD, round = 1;
    double scoreC = 0.0, scoreD = 0.0;
    while (scoreC <= 10.0 && scoreD <= 10.0) // continue the game till the score is less than 10
    {
        printf("Starting Round: %d \n", round);
        round++;
        kill(processC, SIGUSR1);
        moveC = getMove(pipeC[0]);
        printf("Child C got %d \n", moveC);
        kill(processD, SIGUSR1);
        moveD = getMove(pipeD[0]);
        printf("Child D got %d \n", moveD);
        if (moveC == moveD)
        {
            scoreC += 0.5;
            scoreD += 0.5;
            continue;
        }
        if (moveC == 1) // Process C gives Paper
        {
            if (moveD == 2) // Process D gives scissor
            {
                scoreD += 1.0;
            }
            else
            {
                scoreC += 1.0;
            }
        }
        else if (moveC == 2) // Process C gives Scissor
        {
            if (moveD == 3) // Process D gives stone
            {
                scoreD += 1.0;
            }
            else
            {
                scoreC += 1.0;
            }
        }
        else // Process C gives Stone
        {
            if (moveD == 1) // Process D gives paper
            {
                scoreD += 1.0;
            }
            else
            {
                scoreC += 1.0;
            }
        }
        printf("Current Score of Child C is %lf and Current Score of Child D is %lf\n", scoreC, scoreD);
    }
    printf("\nGame Over\n");
    printf("Child C got score %lf\n", scoreC);
    printf("Child D got score %lf\n", scoreD);
    if (scoreC == scoreD) // tie condition
    {
        moveC = rand();
        moveD = rand();
        if (moveC > moveD)
        {
            printf("Child C has won the game\n");
        }
        else
        {
            printf("Child D has won the game\n");
        }
    }
    else if (scoreC > scoreD)
    {
        printf("Child C has won the game\n");
    }
    else
    {
        printf("Child D has won the game\n");
    }

    printf("\nSending the exit signal for both Process C and D\n");
    kill(processC, SIGUSR2);
    kill(processD, SIGUSR2);
    waitpid(processC, NULL, 0);
    waitpid(processD, NULL, 0);
    printf("Master Process finishes now\n");
    exit(0);
}
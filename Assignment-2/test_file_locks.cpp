#include <iostream>
#include <sys/file.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

using namespace std;

void delep(char *file_path)
{
    int childPid = fork();
    // call this command and pass the output to the parent
    if (childPid == 0)
    {   
        if (execlp("fuser", "fuser", file_path, (char *)NULL) < 0)
        {
            perror("");
        }
        else
        {
            perror("Executer command: ");
        }

        exit(0);
    }
    else if (childPid < 0)
    {
        perror("Unable to fork\n");
    }
    wait(0);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("No filename given");
        exit(0);
    }

    char *random=strdup("WRITING SOME GARBAGE\n");

    delep(argv[1]);
    printf("Are you sure you want to kill the above processes?");

    return 0;
}
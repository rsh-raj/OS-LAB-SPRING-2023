#include <iostream>
#include <sys/file.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
using namespace std;
void delep(char *file_path);
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("No filename given");
        exit(0);
    }
    int file1 = open(argv[1], O_WRONLY, O_TRUNC);
    // if (file1 < 0)
    // {
    //     printf("%s doesn't exists", argv[1]);
    //     exit(0);
    // }
    // flock(file1,LOCK_EX);
    // char *random=strdup("WRITING SOME GARBAGE\n");
    // while(1){
    //     write(file1,random,21);
    //     sleep(3);

    // }
    delep(argv[1]);

    return 0;
}
void delep(char *file_path)
{
    int childPid = fork();
    if (childPid == 0)
    {
        if (execlp("lsof", "lsof", file_path, (char *)NULL) < 0)
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
    printf("Are you sure you want to kill the above processes?");
}
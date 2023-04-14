#include <iostream>
#include <sys/file.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("No filename given");
        exit(0);
    }

    int file1 = open(argv[1], O_WRONLY,O_TRUNC);
    if (file1 < 0)
    {
        printf("%s doesn't exists", argv[1]);
        exit(0);
    }
    int pid = getpid();
    cout<<"Pid of opener is : "<<pid<<endl;

    flock(file1,LOCK_EX); // locking the file
    char *random=strdup("WRITING SOME GARBAGE\n");

    cout<<"Pid of locker is : "<<pid<<endl;
    
    while(1){
        write(file1,random,21);
        sleep(3);
    }

    return 0;
}
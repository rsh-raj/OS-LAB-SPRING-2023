#include <iostream>
#include <sys/file.h>
#include <unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include <vector>

using namespace std;


void file_lock_detection(char *file_path){
    char out[200];
    int fd[2];
    pipe(fd);

    if(fork() == 0){
        // redirecting the stdout of the process 
        close(1);
        dup(fd[1]);

        int x = execlp("fuser", "fuser", file_path, (char *)NULL);
        // cout<<x<<endl;

        exit(0);
    }
    wait(0);

    // making read non blocking
    int flags = fcntl(fd[0], F_GETFL, 0);
    fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);

    // cout<<"wow"<<endl;
    // parent processing

    // checking if the file has any locks or not
    int x = read(fd[0], out, 500);
    if(x == -1){
        cout<<"File has no locks"<<endl;
        if(!remove(file_path)){

            cout<<"File successfully deleted !"<<endl;
        }else cout<<"Failed to delete file"<<endl;

        return ;
    }

    // parsing the pids from the 
    vector<int> locking_pids;
    char temp[20];
    int index = 0;

    for (int i = 0; out[i] != '\0'; i++)
    {
        if(out[i] == ' ') continue;
        else{
            while(out[i] != ' '){
                if(out[i] == '\0') break;
                temp[index++] = out[i++];
            }
        }
        temp[index++] = '\0';
        locking_pids.push_back(atoi(temp));
        index = 0;
    }

    // for(auto it : locking_pids){
    //     cout<<it<<" ";
    // }
    // cout<<endl;
    cout<<"The following processes are locking/read/write the file :"<<endl;
    for(auto it : locking_pids){
        char str[10];
        sprintf(str, "%d", it);
        char buf[50];
        int fd[2];
        pipe(fd);

        if(fork() == 0){
            close(1);
            dup(fd[1]);
            execlp("ps", "ps", "-p", str, "-o", "comm=", NULL);
            exit(0);
        }

        wait(0);
        read(fd[0], buf, 50);
        buf[strlen(buf)-1] = '\0';
        
        cout<<"PID : "<<it<<"   ";
        cout<<"Name : "<<buf<<endl;
    }

    while(1){
        cout<<"Do you want to delete them (Y/n) ? ";
        char ans;
        cin>>ans;
        if(ans == 'y' || ans == 'Y'){
            // kill process with the given pids
            for(auto it : locking_pids){
                kill(it, SIGKILL);
            }
            if(!remove(file_path)){

                cout<<"File successfully deleted !"<<endl;
            }else cout<<"Failed to delete file"<<endl;
            break;
        }
        else if(ans == 'n' || ans == 'N'){
            cout<<"Not deleted"<<endl;
            break;
        }
        else cout<<"Incorrect answer"<<endl;
    }
}

int main(int argc, char **argv)
{
    // if (argc < 2)
    // {
    //     printf("No filename given");
    //     exit(0);
    // }
    char s[20];
    strcpy(s,"a.txt");
    file_lock_detection(s);

    return 0;
}
#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

using namespace std;

void sig_handler(int signum){
    int x;
    cout<<"Inside handler"<<endl;
    x = fork();
    int status;
    if(x == 0){
        // do something
        exit(0);
    }

    waitpid(x,&status,0);
}

int main(){
    int cnt;
    cout<<"Process Start"<<endl;
    // signal(SIGTSTP, sig_handler);
    int x = fork();

    if(x == 0){
        sleep(5);
        cout<<"This is success!!"<<endl;
        sleep(5);
        exit(0);
    }

    wait(0);

    cout<<"Vallah"<<endl;
}
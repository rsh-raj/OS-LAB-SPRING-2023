#include<stdio.h>  
#include<stdlib.h>  
#include<unistd.h>  
#include<sys/shm.h>  
#include<string.h>  

int main(){

    int iteration = 100;
    char *args[] = {"./sort", "25000000", NULL};
    char buf[20];
    void *shared_mem;
    int shmid;

    key_t key = 2345;
    shmid = shmget(key, 20, 0666);
    shared_mem = shmat(shmid,NULL,0); 
    float time_observed[iteration];
    int index = 0;


    for (int i = 0; i < iteration; i++)
    {   
        printf("\nIteration %d : ", i+1);
        if(fork() == 0){
            execvp("./sort", args);
        }

        wait(0);
        strcpy(buf, (char *)shared_mem);
        printf("WE HAVE RECEIVED THE VALUE %s\n", buf);
        time_observed[index++] = atof(buf);

        if(i == iteration-1) break;
    }

    float sum = 0;
    for (int i = 0; i < iteration; i++)
    {
        sum += time_observed[i];
    }
    
    printf("AVERAGE RUNNING TIME PROGRAM IS %f\n", sum/(float)iteration);
    
    
    return 0;
}
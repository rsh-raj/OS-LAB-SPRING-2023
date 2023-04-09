#include <iostream>
#include <pthread.h>
#include <random>
#include <signal.h>
#include <semaphore.h>
#include <bits/stdc++.h>

#include "mutex_locks.h"
#include "semaphores.h"
#include "functions.h"

using namespace std;

typedef struct room
{
    int number;
    bool current_empty;
    int time_occupied;
    int num_guests_before; // if this == 2, then current_empty will be false
    int guest_number;
    int guest_priority;
    pthread_t guest_thread_id;

    room(int number)
    {
        this->current_empty = true;
        this->number = number;
        this->num_guests_before = 0;
        this->time_occupied = 0;
    }

    room() {}
} room;

room *Room;
set<pair<int, int>> GuestPriority_and_RoomNumber;

int n, x, y;
int num_rooms_that_require_cleaning = 0;
int num_rooms_that_are_cleaned = 0;
int current_global_state = 0;

pthread_mutex_t print_lock;
pthread_mutex_t room_allocation_lock;
pthread_mutex_t priority_check_lock;
pthread_mutex_t num_room_cleaner_update_lock;
pthread_mutex_t temporary;
pthread_mutex_t num_room_that_cleaned_lock;
pthread_cond_t cleaning_staff_has_arrived;

sem_t num_availabe_rooms;
sem_t invoke_cleaner_thread;
sem_t staff_room_cleaning_over;

char **make_arr(char *cmd)
{
    int index = 0;
    char temp[100];
    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(20 * sizeof(char));

    int cnt = 0;

    for (int i = 0; cmd[i] != '\0'; i++)
    {
        if(cmd[i] == ' ') continue;

        // we have some text here
        cnt = 0;
        while(cmd[i] != '\0' && cmd[i] != ' '){
            temp[cnt++] = cmd[i++];
        }

        temp[cnt++] = '\0';
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        cmdarr = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        cmdarr[index] = (char *)malloc(20 * sizeof(char));

        if (cmd[i] == '\0')  break;

    }

    cmdarr[index] = NULL;
    return cmdarr;
}

char **get_current_time(){
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime ( &rawtime );
    char **mkarr = make_arr(asctime(timeinfo));
    return mkarr;
}

void print_guest_info(int guest_id, int index, int stay_time)
{   
    char **c = get_current_time();
    pthread_mutex_lock(&print_lock);
    cout << "Guest " << guest_id << " allocated room number " << index << " for duration " << stay_time << " seconds " <<"at time "<<c[3]<<endl;
    pthread_mutex_unlock(&print_lock);
    free(c);
}

void print_kick_out_info(int guest_id, int guest_id_removed, int room, int stay_time, int prior_enter, int prior_leave)
{   
    char **c = get_current_time();

    pthread_mutex_lock(&print_lock);
    cout << "(Guest:" << guest_id << ",priority:"<<prior_enter<<") kickout out (Guest:" << guest_id_removed << ",priority:"<<prior_leave<<") for room number " << room << " and now stays there for " << stay_time << " seconds " << "at time "<<c[3]<<endl;
    pthread_mutex_unlock(&print_lock);
    free(c);
}

void print_guest_leave_room(int guest_id, int room)
{   
    char **c = get_current_time();

    pthread_mutex_lock(&print_lock);
    cout << "Guest " << guest_id << " leaves the room number " << room << " at time "<<c[3]<< endl;
    pthread_mutex_unlock(&print_lock);
    free(c);
}

void print_staff_cleaning_info(int staff_id, int index, int time_to_clean)
{   
    char **c = get_current_time();
    pthread_mutex_lock(&print_lock);
    cout << "Staff " << staff_id << " cleaned room number " << index << " after time " << time_to_clean << " seconds "<<"at time "<<c[3]<<endl;
    pthread_mutex_unlock(&print_lock);
    free(c);
}

int main()
{
    srand(time(0));
    // int n,x,y;

    while(1){
        cout << "Enter the values of n,x,y : ";
        cin >> n >> x >> y;

        // y > n > x > 1
        if(x > 1){
            if(n > x){
                if(y > n) break;
            }
        }
        cout<<"Incorrect Values, please re-enter"<<endl;
        continue;
    }
    

    // initialising room data structure
    Room = (room *)malloc(n * sizeof(room));
    for (int i = 0; i < n; i++)
    {   
        int cnt = i+1;
        Room[i] = room(cnt);
    }

    // initializing guest data structure

    // initializing mutexes and semaphores
    sem_init(&num_availabe_rooms, 0, n);
    sem_init(&invoke_cleaner_thread, 0, 0);
    sem_init(&staff_room_cleaning_over, 0 , 0);

    pthread_mutex_init(&print_lock, NULL);
    pthread_mutex_init(&room_allocation_lock, NULL);
    pthread_mutex_init(&priority_check_lock, NULL);
    pthread_mutex_init(&num_room_cleaner_update_lock, NULL);
    pthread_mutex_init(&num_room_that_cleaned_lock, NULL);
    pthread_mutex_init(&temporary, NULL);
    pthread_cond_init(&cleaning_staff_has_arrived, NULL);

    // n rooms, x staff threads and y guest threads
    pthread_t guest[y];
    pthread_t staff[x];

    char ** alpha = get_current_time();
    cout << "CREATING GUEST AND STAFF THREADS AT TIME "<<alpha[3]<<"......." << endl;
    free(alpha);
    
    int thread_id[y];
    int cleaner_id[x];
    for (int i = 0; i < y; i++)
    {
        thread_id[i] = i + 1;
        pthread_create(&guest[i], NULL, guest_routine, (void *)&thread_id[i]);
    }

    for (int i = 0; i < x; i++)
    {
        cleaner_id[i] = i + 1;
        pthread_create(&staff[i], NULL, staff_routine, (void *)&cleaner_id[i]);
    }

    // joining the threads
    for (int i = 0; i < y; i++)
    {
        pthread_join(guest[i], NULL);
    }

    for (int i = 0; i < x; i++)
    {
        pthread_join(staff[i], NULL);
    }

    return 0;
}
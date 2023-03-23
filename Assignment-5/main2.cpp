#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <random>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <bits/stdc++.h>

using namespace std;

typedef struct room
{
    int number;
    bool current_empty;
    int time_occupied;
    int num_guests_before; // if this == 2, then current_empty will be false
    int guest_number;
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

void print_guest_info(int guest_id, int index, int stay_time)
{
    pthread_mutex_lock(&print_lock);
    cout << "Guest " << guest_id << " allocated room number " << index << " for duration " << stay_time << " seconds" << endl;
    pthread_mutex_unlock(&print_lock);
}

void print_kick_out_info(int guest_id, int guest_id_removed, int room, int stay_time)
{
    pthread_mutex_lock(&print_lock);
    cout << "Guest " << guest_id << " kickout out Guest " << guest_id_removed << " for room number " << room << " and now stays there for " << stay_time << " seconds" << endl;
    pthread_mutex_unlock(&print_lock);
}

void print_guest_leave_room(int guest_id, int room)
{
    pthread_mutex_lock(&print_lock);
    cout << "Guest " << guest_id << " leaves the room " << room << " and now sleeps..." << endl;
    pthread_mutex_unlock(&print_lock);
}

void print_staff_cleaning_info(int staff_id, int index, int time_to_clean)
{
    pthread_mutex_lock(&print_lock);
    cout << "Staff " << staff_id << " cleaned room number " << index << " after time " << time_to_clean << " seconds" << endl;
    pthread_mutex_unlock(&print_lock);
}

void *guest_routine(void *i)
{
    int *guest_id = (int *)i;

    // if cleaning in process, please wait, only proceed when cleaning is over
    while (1)
    {
        int sleep_time = rand() % 10 + 1;

        if (num_rooms_that_require_cleaning != n)
        {
            pthread_mutex_lock(&print_lock);
            cout << "Guest " << *guest_id << " sleeping for " << sleep_time << " seconds" << endl;
            pthread_mutex_unlock(&print_lock);
        }

        sleep(sleep_time);

        // request for the room
        int stay_time = rand() % 20 + 1;

        int val, temp = 0, already_waited = 0;

        // wait till the rooms are cleaned
        // if (num_rooms_that_require_cleaning == n)
        // {   
        //     already_waited = 1;
        //     sem_wait(&num_availabe_rooms);
        // }

        pthread_mutex_lock(&priority_check_lock);
        sem_getvalue(&num_availabe_rooms, &val);
        // cout<<"got value "<<val<<endl;
        if (val == 0 && GuestPriority_and_RoomNumber.size() > 0)
        {
            // get the least priority guest and remove it
            // all in the queue will have room occupancy < 2

            int prior = (*GuestPriority_and_RoomNumber.begin()).first;

            if (prior < (y - *guest_id + 1))
            {
                temp = 1;
            }
        }

        // both of these have to be atomic
        pthread_mutex_unlock(&priority_check_lock);

        // wait if not room is empty
        if (temp == 0 && already_waited == 0)
            sem_wait(&num_availabe_rooms);

        // assign a room
        int index = -1;
        pthread_t thread_id_removed;
        int guest_id_removed;
        pthread_mutex_lock(&room_allocation_lock);

        // book the room for the thread
        if (!temp)
        {
            for (int i = 0; i < n; i++)
            {
                if (Room[i].current_empty)
                {
                    index = i;
                    break;
                }
            }
        }

        else
        {
            index = (*GuestPriority_and_RoomNumber.begin()).second;
            GuestPriority_and_RoomNumber.erase(GuestPriority_and_RoomNumber.begin());
            thread_id_removed = Room[index].guest_thread_id;
            guest_id_removed = Room[index].guest_number;

            // kill the thread with that id that occupied that room
            pthread_cancel(thread_id_removed);
            pthread_t thread;
            pthread_create(&thread, NULL, guest_routine, (void *)&guest_id_removed);
        }

        // fill the room with the guest datails
        Room[index].num_guests_before++;
        if (Room[index].num_guests_before < 2)
            GuestPriority_and_RoomNumber.insert(make_pair((y - *guest_id + 1), index));
        pthread_mutex_unlock(&room_allocation_lock);

        Room[index].current_empty = false;
        Room[index].guest_thread_id = pthread_self();
        Room[index].guest_number = *guest_id;
        Room[index].time_occupied += stay_time;

        // print the information of booking to the console
        if (temp)
            print_kick_out_info(*guest_id, guest_id_removed, index, stay_time);
        else
            print_guest_info(*guest_id, index, stay_time);

        // check if cleaning process needs to be initiated
        if (Room[index].num_guests_before == 2)
        {

            pthread_mutex_lock(&num_room_cleaner_update_lock);
            num_rooms_that_require_cleaning++;
            if (num_rooms_that_require_cleaning == n)
            {
                // start the cleaner threads
                pthread_mutex_lock(&print_lock);
                cout << "Removing Guest " << *guest_id << " as cleaning starts in room number " << index <<endl;

                pthread_cond_broadcast(&cleaning_staff_has_arrived);

                cout << "\n...........INVOKE CLEANER THREADS............\n"
                     << endl;
                pthread_mutex_unlock(&print_lock);

                num_rooms_that_are_cleaned = 0;
                sem_post(&invoke_cleaner_thread);
                // empty all the guest threads from the hotel
                pthread_mutex_unlock(&num_room_cleaner_update_lock);

                // clear the set data structure
                // set the lock ?? 
                GuestPriority_and_RoomNumber.clear();

                // this guest removed from the room by the cleaner thread
                continue;
            }
            pthread_mutex_unlock(&num_room_cleaner_update_lock);
        }

        // sleep for the stay time
        sleep(stay_time);

        // getting the timestruct to pass to the pthread function
        // struct timeval timestampx;
        // gettimeofday(&timestampx, NULL);

        // timestampx.tv_sec += stay_time;
        // struct timespec spec;
        // // clock_gettime(CLOCK_REALTIME, &spec);
        // // spec.tv_sec += stay_time;
        // TIMEVAL_TO_TIMESPEC(&timestampx, &spec);

        // pthread_mutex_lock(&temporary);
        // int output = pthread_cond_timedwait(&cleaning_staff_has_arrived, &temporary, &spec);
        // cout<<"OUTPUT IS "<<output<<endl;
        // pthread_mutex_unlock(&temporary);


        // remove this index from the set
        pthread_mutex_lock(&priority_check_lock);
        auto it = GuestPriority_and_RoomNumber.find(make_pair(y - *guest_id + 1, index));
        if (it != GuestPriority_and_RoomNumber.end())
            GuestPriority_and_RoomNumber.erase(it);
        pthread_mutex_unlock(&priority_check_lock);


        // if(output == 0){
        //     cout<<"Removing Guest "<<*guest_id<<" as cleaning starts in room number "<<index<<endl;
        //     continue;
        // }

        if (num_rooms_that_require_cleaning == n)
            continue;
        // update information on guest leaving the room
        if (Room[index].num_guests_before < 2)
        {
            Room[index].current_empty = true;
            sem_post(&num_availabe_rooms);
        }
        print_guest_leave_room(*guest_id, index);
    }

    return NULL;
}

void *staff_routine(void *i)
{
    int *staff_id = (int *)i;

    while (1)
    {
        cout << "I am waiting here ....." << endl;
        sem_wait(&invoke_cleaner_thread);
        sem_post(&invoke_cleaner_thread);


        pthread_mutex_lock(&print_lock);
        int val;
        sem_getvalue(&staff_room_cleaning_over, &val);
        if(val != 0) sem_wait(&staff_room_cleaning_over);

        cout << "Cleaner " << *staff_id << " started working!" << endl;
        pthread_mutex_unlock(&print_lock);

        while (1)
        {
            // get a dirty room
            int index = -1;
            pthread_mutex_lock(&room_allocation_lock);

            for (int i = 0; i < n; i++)
            {
                if (Room[i].num_guests_before == 2)
                {
                    index = i;
                    Room[i].num_guests_before = 0;
                    break;
                }
            }

            // unlock
            pthread_mutex_unlock(&room_allocation_lock);
            cout<<"Staff "<<*staff_id<<" got index "<<index<<endl;
            // can do better
            if (index == -1)
            {
                sem_wait(&staff_room_cleaning_over);
                sem_post(&staff_room_cleaning_over);
                break;
            }

            int time_to_clean = Room[index].time_occupied;
            Room[index].time_occupied = 0;
            Room[index].current_empty = true;
            sleep(time_to_clean / 5);

            print_staff_cleaning_info(*staff_id, index, time_to_clean / 5);

            // lock
            pthread_mutex_lock(&num_room_that_cleaned_lock);
            num_rooms_that_are_cleaned++;
            if (num_rooms_that_are_cleaned == n)
            {
                num_rooms_that_require_cleaning = 0;
                sem_wait(&invoke_cleaner_thread);
                sem_post(&staff_room_cleaning_over);

                cout << "\nAll rooms cleaned!! Guests Allowed\n"
                     << endl;
                for (int i = 0; i < n; i++)
                {
                    sem_post(&num_availabe_rooms);
                }
                pthread_mutex_unlock(&num_room_that_cleaned_lock);

                // all staff again wait for rooms to get dirty
                break;
            }
            // unlock
            pthread_mutex_unlock(&num_room_that_cleaned_lock);
        }
    }

    return NULL;
}

int main()
{
    srand(time(0));
    // int n,x,y;
    cout << "Enter the values : ";
    cin >> n >> x >> y;

    // initialising room data structure
    Room = (room *)malloc(n * sizeof(room));
    for (int i = 0; i < n; i++)
    {
        Room[i] = room(i + 1);
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

    cout << "Creating the guest and staff threads...." << endl;
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

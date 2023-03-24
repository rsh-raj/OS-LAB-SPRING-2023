#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <bits/stdc++.h>
#include "mutex_locks.h"
#include "semaphores.h"
#include "functions.h"
#include "global_data.h"

using namespace std;

void *staff_routine(void *i)
{
    int *staff_id = (int *)i;

    while (1)
    {
        sem_wait(&invoke_cleaner_thread);
        sem_post(&invoke_cleaner_thread);


        pthread_mutex_lock(&print_lock);
        int val;
        sem_getvalue(&staff_room_cleaning_over, &val);
        if(val != 0) sem_wait(&staff_room_cleaning_over);

        cout << "CLEANER " << *staff_id << " STARTED WORKING....." << endl;
        pthread_mutex_unlock(&print_lock);

        while (1)
        {
            // get a dirty room
            int index = -1;
            pthread_mutex_lock(&room_allocation_lock);

            for (int i = 0; i < n; i++)
            {   
                if (Room[i].num_guests_before >= 2)
                {
                    index = i;
                    Room[i].num_guests_before = 0;
                    break;
                }
            }

            // unlock
            pthread_mutex_unlock(&room_allocation_lock);
            // can do better
            if (index == -1)
            {   
                // if we remove this, it will not stop, just will prnt redundant information
                sem_wait(&staff_room_cleaning_over);
                sem_post(&staff_room_cleaning_over);
                break;
            }

            int time_to_clean = Room[index].time_occupied;
            Room[index].time_occupied = 0;
            Room[index].current_empty = true;
            sleep(time_to_clean / 3);

            print_staff_cleaning_info(*staff_id, index, time_to_clean / 3);

            // lock
            pthread_mutex_lock(&num_room_that_cleaned_lock);
            num_rooms_that_are_cleaned++;
            if (num_rooms_that_are_cleaned == n)
            {
                num_rooms_that_require_cleaning = 0;
                sem_wait(&invoke_cleaner_thread);
                sem_post(&staff_room_cleaning_over);

                cout << "\n----------------------ALL ROOMS CLEANED AT TIME "<<get_current_time()<<"---------------------------\n"
                     << endl;
                int val;
                sem_getvalue(&num_availabe_rooms, &val);
                for (int i = 0; i < n-val; i++)
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
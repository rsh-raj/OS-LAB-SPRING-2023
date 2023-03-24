#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <bits/stdc++.h>
#include "mutex_locks.h"
#include "semaphores.h"
#include "functions.h"
#include "global_data.h"

using namespace std;

void *guest_routine(void *i)
{
    int *guest_id = (int *)i;

    // if cleaning in process, please wait, only proceed when cleaning is over
    while (1)
    {
        int sleep_time = rand() % 10 + 1;
        int current_thread_state;

        // if (num_rooms_that_require_cleaning != n)
        // {
        //     pthread_mutex_lock(&print_lock);
        //     cout << "Guest " << *guest_id << " sleeping for " << sleep_time << " seconds" << endl;
        //     pthread_mutex_unlock(&print_lock);
        // }

        pthread_mutex_lock(&print_lock);
        cout << "Guest " << *guest_id << " sleeping for " << sleep_time << " seconds" <<endl;
        pthread_mutex_unlock(&print_lock);

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
                if (Room[i].current_empty && Room[i].num_guests_before < 2)
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

        if(index == -1) continue;

        // fill the room with the guest datails
        Room[index].current_empty = false;
        Room[index].num_guests_before++;
        if (Room[index].num_guests_before < 2)
            GuestPriority_and_RoomNumber.insert(make_pair((y - *guest_id + 1), index));
        pthread_mutex_unlock(&room_allocation_lock);

        Room[index].guest_thread_id = pthread_self();
        Room[index].guest_number = *guest_id;
        Room[index].time_occupied += stay_time;
        current_thread_state = current_global_state;

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

                cout << "\n...........INVOKE CLEANER THREADS............\n"
                     << endl;
                pthread_mutex_unlock(&print_lock);
                pthread_cond_broadcast(&cleaning_staff_has_arrived);
                current_global_state++;

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

        if(current_thread_state != current_global_state) continue;
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
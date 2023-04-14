// #include <bits/stdc++.h>
#ifndef GLOBAL_DATA_
#define GLOBAL_DATA_

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

extern room *Room;
extern set<pair<int, int>> GuestPriority_and_RoomNumber;

extern int n, x, y;
extern int num_rooms_that_require_cleaning;
extern int num_rooms_that_are_cleaned;
extern int current_global_state;

#endif
#ifndef FUNCTIONS
#define FUNCTIONS

void print_guest_info(int guest_id, int index, int stay_time);
void print_kick_out_info(int guest_id, int guest_id_removed, int room, int stay_time, int ,int);
void print_guest_leave_room(int guest_id, int room);
void print_staff_cleaning_info(int staff_id, int index, int time_to_clean);
void *guest_routine(void *);
void *staff_routine(void *);
char **get_current_time();
char **make_arr(char *);

#endif
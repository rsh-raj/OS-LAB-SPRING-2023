#ifndef MUTEX_LOCKS
#define MUTEX_LOCKS

extern pthread_mutex_t print_lock;
extern pthread_mutex_t room_allocation_lock;
extern pthread_mutex_t priority_check_lock;
extern pthread_mutex_t num_room_cleaner_update_lock;
extern pthread_mutex_t temporary;
extern pthread_mutex_t num_room_that_cleaned_lock;
extern pthread_cond_t cleaning_staff_has_arrived;

#endif
#ifndef TIME_H
#define TIME_H

#include <stdbool.h>

// Initialises the time manager
extern void mc_time_init(void);
// Gets the current timestamp
extern char *mc_time_get_timestamp(void);
// Sets the time by providing a string
extern bool mc_time_set_time_str(char *newTimeString);

#endif /* TIME_H */

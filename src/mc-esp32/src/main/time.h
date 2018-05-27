#ifndef TIME_H
#define TIME_H

#include <stdbool.h>

extern void mc_time_init(void);
extern char *mc_time_get_timestamp(void);
extern bool mc_time_set_time_str(char *newTimeString);

#endif /* TIME_H */

#ifndef NETWORK_H
#define NETWORK_H

#include "adt.h"

typedef ADT_Node Packet;

extern char *serialise_packet(const char *s);

#endif /* NETWORK_H */

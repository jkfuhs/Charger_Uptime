#ifndef CHARGER_UPTIME_H
#define CHARGER_UPTIME_H

#include <stdio.h>
#include "bin_tree.h"

bin_tree build_station_tree(FILE *input_file, bin_tree *station_tree);
void process_statuses(bin_tree charger_tree, bin_tree station_tree, FILE *input_file);
void print_station_data(bin_tree station_tree);

typedef struct Linked_times_st *LL_node;
struct Linked_times_st
{
    uint64_t start;
    uint64_t end;
    LL_node next;
};


#endif
#ifndef BINTREE_H
#define BINTREE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct bin_tree_node_st *bin_tree;
struct bin_tree_node_st
{
    uint32_t d1;
    uint32_t d2;
    bin_tree left;
    bin_tree right;
    uint32_t height;
    bin_tree runtime;
    bin_tree uptime;
};

bin_tree add_node(bin_tree head, uint32_t d1, uint32_t d2, bin_tree runtime, bin_tree uptime);
void pre_print(bin_tree N);

#endif
#include "charger_uptime.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

uint64_t min64(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

uint64_t max64(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}


bin_tree build_station_tree(FILE *infile, bin_tree *station_tree)
{
    char *str = NULL;
    size_t len;
    ssize_t nread;
    char c;
    uint32_t station;
    uint32_t charger;
    bin_tree charger_tree = NULL;
    bin_tree station_runtime, station_uptime;

    nread = getline(&str, &len, infile);
    if (nread == -1)
    {
        perror("getline error");
        exit(-1);
    }
    if (strcmp(str, "[Stations]\n"))
    {
        perror("input file format. Expected: [Stations]");
        exit(-1);
    }

    if (str)
        free(str);

    while ((c = getc(infile)) != EOF && c != '[')
    {
        if (!(c >= '0' && c<= '9'))
        {
            continue;
        }
        // printf("station c: %c\n", c);
        if (c >= '0' && c <= '9')
        {
            station = c - '0';
        }
        // extract station ID
        while ((c = getc(infile)) >= '0' && c <= '9')
        {
            station *= 10;
            station += c - '0';
        }
        // extract charger IDs
        // printf("station: %d\n", station);
        charger = 0;
        station_runtime = add_node(NULL, 0, 0, NULL, NULL);
        station_uptime = add_node(NULL, 0, 0, NULL, NULL);
        *station_tree = add_node(*station_tree, station, 0, station_runtime, station_uptime);
        while ((c = getc(infile)) != EOF)
        {
            // printf("charger c: %c\n", c);
            if (c >= '0' && c <= '9')
            {
                charger *= 10;
                charger += c - '0';
            }
            // add charger to tree
            else
            {
                charger_tree = add_node(charger_tree, charger, station, station_runtime, station_uptime);
                charger = 0;
            }
            if (c == '\n')
            {
                break;
            }
        }
        station = 0;
    }

    return charger_tree;
}

void parse_status(bin_tree charger_tree, bin_tree station_tree, char* line)
{
    uint32_t charger;
    uint32_t station;
    uint64_t start_time;
    uint64_t end_time;
    bin_tree charger_node;
    bin_tree station_node;

    char *token;

    if (!charger_tree)
    {
        return;
    }
    token = strtok(line, "\n ");
    charger = atol(token);

    token = strtok(NULL, "\n ");
    start_time = atoll(token);

    token = strtok(NULL, "\n ");
    end_time = atoll(token);

    token = strtok(NULL, "\n ");

    charger_node = get_node(charger_tree, charger);
    if (!charger_node)
    {
        fprintf(stderr, "Charger %d does not exist\n", charger);
        return;
    }

    station = charger_node->d2;
    station_node = get_node(station_tree, station);
    if (!station_node)
    {
        fprintf(stderr, "Station %d does not exist\n", station);
        return;
    }

    if (!station_node->runtime)
    {
        station_node->runtime = add_node(station_node->runtime, start_time, end_time, NULL, NULL);
    }
    else
    {
        station_node->runtime->d1 = min64(station_node->runtime->d1, start_time);
        station_node->runtime->d2 = max64(station_node->runtime->d2, end_time);
    }

    if (strcmp(token, "true") == 0)
    {
        station_node->uptime = add_node(station_node->uptime, start_time, end_time, NULL, NULL);
    }
    else if (strcmp(token, "false") != 0)
    {
        perror("something went wrong. up != true/false");
        exit(-1);
    }
}

void process_statuses(bin_tree charger_tree, bin_tree station_tree, FILE *infile)
{
    char *str = NULL;
    size_t len;
    ssize_t nread;

    // header line: [Charger Availability Reports]
    nread = getline(&str, &len, infile);
    if (nread == -1)
    {
        perror("[Charger Availability Reports] read error");
        exit(-1);
    }
    if (strcmp(str, "Charger Availability Reports]\n"))
    {
        perror("input file format: missing [Charger Availability Reports] header");
        exit(-1);
    }
    // parse statuses
    while ((nread = getline(&str, &len, infile)) != -1)
    {
        parse_status(charger_tree, station_tree, str);
    }
    if (str)
    {
        free(str);
    }
}

LL_node new_LL_node(uint64_t start, uint64_t end, LL_node next)
{
    LL_node N;
    N = malloc(sizeof(struct Linked_times_st));
    if (!N)
    {
        perror("Malloc: Could not allocate linked list in new_LL_node");
        return NULL;
    }
    N->start = start;
    N->end = end;
    N->next = next;
    return N; 
}

LL_node tree_to_LL(bin_tree tree, LL_node *head, LL_node temp)
{
    if (!tree)
    {
        return temp;
    }

    temp = tree_to_LL(tree->left, head, temp);
    if (!temp)
    {
        temp = new_LL_node(tree->d1, tree->d2, NULL);
    }
    if (!(*head))
    {
        *head = temp;
    }

    if (tree->d1 <= temp->end)
    {
        temp->end = max64(temp->end, tree->d2);
    }
    else
    {
        temp->next = new_LL_node(tree->d1, tree->d2, NULL);
        temp = temp->next;
    }
    temp = tree_to_LL(tree->right, head, temp);
    return temp;
}

void free_LL(LL_node L)
{
    LL_node temp;

    if (!L)
    {
        return;
    }

    while ((temp = L->next))
    {
        free(L);
        L = temp;
    }
    return;
}

void print_station_data(bin_tree station_tree)
{
    LL_node uptimes_list = NULL;
    LL_node temp;
    uint64_t total_runtime;
    uint64_t total_uptime;
    uint64_t percentage;
    if (!station_tree)
    {
        return;
    }

    print_station_data(station_tree->left);
    tree_to_LL(station_tree->uptime, &uptimes_list, NULL);
    free_tree(station_tree->uptime);
    
    total_runtime = station_tree->runtime->d2 - station_tree->runtime->d1;
    free_tree(station_tree->runtime);
    
    total_uptime = 0;
    temp = uptimes_list;
    while (temp)
    {
        total_uptime += temp->end - temp->start;
        temp = temp->next;
    }
    free_LL(uptimes_list);
    
    percentage = (100 * total_uptime) / total_runtime;
    printf("%ld %ld\n", station_tree->d1, percentage);

    print_station_data(station_tree->right);
    return;
}
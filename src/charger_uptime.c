#include "charger_uptime.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// helper function to find min of uint64_t values
uint64_t min64(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

// helper function to find max of uint64_t values
uint64_t max64(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}

/* function to build a binary tree mapping stations to uptime and runtime
 * and a binary tree mapping chargers to stations. Updates station tree
 * by reference.
 * Returns: binary tree mapping chargers to stations
 * Params:  a filestream and a binary tree pointer for the station tree
 */
bin_tree build_station_tree(FILE *infile, bin_tree *station_tree)
{
    char *str = NULL;
    size_t len;
    ssize_t nread;
    char c;
    uint32_t station;
    uint32_t charger;
    bin_tree charger_tree = NULL;
    bin_tree node;

    // read the first line of the file to check headers
    nread = getline(&str, &len, infile);
    if (nread == -1)
    {
        perror("getline error. Expected: [Stations]");
        printf("ERROR\n");
        exit(-1);
    }
    if (strcmp(str, "[Stations]\n"))
    {
        perror("input file format. Expected: [Stations]");
        printf("ERROR\n");
        exit(-1);
    }

    if (str)
        free(str);

    // read Station IDs and Charger IDs character by character to
    //    check for format errors.
    while ((c = getc(infile)) != EOF && c != '[')
    {
        if (c == '\n' || c == '\t' || c == ' ')
        {
            continue;
        }
        // printf("station c: %c\n", c);
        if (c >= '0' && c <= '9')
        {
            station = c - '0';
        }
        else 
        {
            perror("file format error: expected Station ID");
            printf("ERROR\n");
            exit(-1);
        }
        // extract station ID
        while ((c = getc(infile)) >= '0' && c <= '9')
        {
            if (station > ((uint32_t)(-1)) / 10)
            {
                perror("Station ID must be unsigned 32-bit integer");
                printf("ERROR\n");
                exit(-1);
            }
            station *= 10;
            if (station > ((uint32_t)(-1)) - (c - '0'))
            {
                perror("Station ID must be unsigned 32-bit integer");
                printf("ERROR\n");
                exit(-1);
            }
            station += c - '0';
        }
        if (c != '\n' && c != '\t' && c != ' ')
        {
            perror("file format error: expected Station ID or Charger ID");
            printf("ERROR\n");
            exit(-1);
        }
        // extract charger IDs
        charger = 0;

        // add station to tree unless it already exists
        if (get_node(*station_tree, station) == NULL)
        {
            (*station_tree) = add_node(*station_tree, station, 0, NULL, NULL);
        }
        while ((c = getc(infile)) != EOF)
        {
            if (c >= '0' && c <= '9')
            {
                if (charger > (((uint32_t)(-1)) / 10) - (c - '0'))
                {
                    perror("Charger must be a 32-bit unsigned integer");
                    printf("ERROR\n");
                    exit(-1);
                }
                charger *= 10;
                charger += c - '0';
            }
            // add charger to tree
            else if (c == ' ' || c == '\n' || c == '\t')
            {
                // If the charger already exists under a different station, error and exit.
                if ((node = get_node(charger_tree, charger)) && node->d2 != station)
                {
                    if (node->d2 != station)
                    {
                        fprintf(stderr, "Charger %d listed twice under stations %d, %ld\n", charger, station, node->d2);
                        printf("ERROR\n");
                        exit(-1);
                    }
                }
                else 
                {
                    // Add the charger to the tree
                    charger_tree = add_node(charger_tree, charger, station, NULL, NULL);
                    charger = 0;
                }
            }
            else 
            {
                perror("expected Station ID or whitespace deliminator");
                printf("ERROR\n");
                exit(-1);
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

// Parses a line from the [Charger Availability Reports]
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
        perror("No listed chargers. Entry Ignored");
        return;
    }

    // get data from line
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
        fprintf(stderr, "Charger %d does not exist. Entry Ignored\n", charger);
        return;
    }

    station = charger_node->d2;
    station_node = get_node(station_tree, station);
    if (!station_node)
    {
        fprintf(stderr, "Station %d does not exist. Entry Ignored\n", station);
        return;
    }

    if (start_time > end_time)
    {
        perror("Start time greater than end time. Entry Ignored");
        return;
    }

    // update start and end runtimes
    if (!station_node->runtime)
    {
        station_node->runtime = add_node(station_node->runtime, start_time, end_time, NULL, NULL);
    }
    else
    {
        station_node->runtime->d1 = min64(station_node->runtime->d1, start_time);
        station_node->runtime->d2 = max64(station_node->runtime->d2, end_time);
    }

    // check true/false value
    if (strcmp(token, "true") == 0)
    {
        station_node->uptime = add_node(station_node->uptime, start_time, end_time, NULL, NULL);
    }
    else if (strcmp(token, "false") != 0)
    {
        perror("file format error: expected \"true/false\"");
        printf("ERROR\n");
        exit(-1);
    }
}

// read [Charger Availability Reports] line by line
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
        printf("ERROR\n");
        exit(-1);
    }
    if (strcmp(str, "Charger Availability Reports]\n"))
    {
        perror("input file format: expected [Charger Availability Reports] header");
        printf("ERROR\n");
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

// helper function to create a Linked List node
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

// helper function to convert a sorted binary tree storing times
// to a linked list with overlapping times removed
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
        if (!temp)
        {
            perror("New LL node failed");
            printf("ERROR\n");
            exit(-1);
        }
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
        if (!temp->next)
        {
            perror("new LL_node failed");
            printf("ERROR\n");
            exit(-1);
        }
        temp = temp->next;
    }
    temp = tree_to_LL(tree->right, head, temp);
    return temp;
}

// helper function to free a Linked List
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

/* Recursively iterates through binary tree of stations in ascending order and 
 * calculates uptime and runtime, then prints relevant data to stdout.
 Returns: void
 Params:  A binary tree storing station IDs and uptime/runtime data */
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

    // recurse left
    print_station_data(station_tree->left);
    if (station_tree->uptime)
    {
        tree_to_LL(station_tree->uptime, &uptimes_list, NULL);
        free_tree(station_tree->uptime);
    }

    if (station_tree->runtime)
    {
        total_runtime = station_tree->runtime->d2 - station_tree->runtime->d1;
        free_tree(station_tree->runtime);
    }
    else 
    {
        total_runtime = 0;
    }
    
    total_uptime = 0;
    temp = uptimes_list;
    while (temp)
    {
        total_uptime += temp->end - temp->start;
        temp = temp->next;
    }
    free_LL(uptimes_list);
    
    if (total_runtime > 0)
    {
        percentage = (100 * total_uptime) / total_runtime;
        printf("%ld %ld\n", station_tree->d1, percentage);
    }
    else
    {
        printf("%ld No Data\n", station_tree->d1);
    }

    // recurse right
    print_station_data(station_tree->right);
    return;
}
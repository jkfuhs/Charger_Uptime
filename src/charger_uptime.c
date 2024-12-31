#include "charger_uptime.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

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

void process_statuses(bin_tree charger_tree, FILE *infile)
{
    char *str = NULL;
    size_t len;
    ssize_t nread;

    while ((nread = getline(&str), )
}
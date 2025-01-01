#include "charger_uptime.h"
#include "bin_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXFILENAME

int main(int argc, char* argv[])
{
    FILE *input_file;
    bin_tree station_tree = NULL, charger_tree;
    if (argc != 2)
    {
        perror("incorrect number of arguments.\nExpected usage: ./charger_uptime input_file");
        printf("ERROR\n");
        exit(-1);
    }

    if (strlen(argv[1]) > FILENAME_MAX)
    {
        perror("filename exceeds max filename");
        printf("ERROR\n");
        exit(-1);
    }

    input_file = fopen(argv[1], "r");
    if (input_file == NULL)
    {
        perror("Error opening input file");
        printf("ERROR\n");
        exit(-1);
    }
    charger_tree = build_station_tree(input_file, &station_tree);
    process_statuses(charger_tree, station_tree, input_file);
    fclose(input_file);

    free_tree(charger_tree);
    print_station_data(station_tree);
    free_tree(station_tree);
    return 0;
}
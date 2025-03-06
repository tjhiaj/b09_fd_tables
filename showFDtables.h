#ifndef SHOW_FD_TABLES_H
#define SHOW_FD_TABLES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>

#define MAX_PATH_LENGTH 4096
#define MAX_PIDS 32768
#define PROC_PATH "/proc"

#define FLAG_PRE_PROCESS 1
#define FLAG_SYSTEM_WIDE 2
#define FLAG_VNODES 3
#define FLAG_OUTPUT_TXT 4
#define FLAG_OUTPUT_BINARY 5
#define FLAG_SUMMARY 6
#define FLAG_THRESHOLD 7
#define FLAG_COMPOSITE 0

typedef struct {
    char * pid;
    int count;
} PIDEntry;

// Function declarations
void processFDEntry(int flag, int *row, struct dirent *entry, struct dirent *fd_entry, char *link_path, int* pidCount, PIDEntry** pidTable, FILE* file);
void processFD(int flag, int *row, int* pidCount, PIDEntry** pidTable, int target_pid, FILE* file, char * fd_path, struct dirent *entry, DIR *dir);
void processDirectory(int flag, int* pidCount, PIDEntry** pidTable, int target_pid, FILE* file);
void parseArguments(int argc, char **argv, int *flags, int *threshold_val, pid_t *target_pid, int *flag_detected);
void processFlags(int *flags, int threshold_val, pid_t target_pid, int flag_detected, PIDEntry **pidTable, int *pidCount);
int isNumber(char *input);
void updatePIDTable(int* pidCount, PIDEntry** pidTable, struct dirent *entry);
void printData(int flag, int *row, struct dirent *entry, struct dirent *fd_entry, char *target_path, struct stat statbuf, FILE* file);
void printHeader(int flag, FILE* file);
void printSummary(int pidCount, PIDEntry *pidTable);
void printThreshold(int pidCount, PIDEntry *pidTable, int threshold);

#endif

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>

#define PROC_PATH "/proc"
#define MAX_PATH_LENGTH 500
#define MAX_PIDS 10000

typedef struct {
    char * pid;
    int count;
} PIDEntry;

int isNumber(char* input) {
    int length = strlen(input);
    for (int i = 0; i < length; i++) {
        if (!isdigit(input[i])) {
            return 0;
        }
    }
    return 1;
}

void printHeader(int pre_process, int systemWide, int Vnodes, FILE* file, int output_TXT){
    if (pre_process){
        printf("%-8s %-8s %-8s\n", "", "PID", "FD");
        printf("        =============\n");
    }
    else if (systemWide){
        printf("%-8s %-8s %-8s %-32s\n", "", "PID", "FD", "Filename" );
        printf("        ============================\n");
    }
    else if (Vnodes){
        printf("%-8s %s\n", "","Inode" );
        printf("        ================\n");
    }
    else if (output_TXT){
        fprintf(file, "%-8s %-8s %-8s %-32s %s\n", "", "PID", "FD", "Filename", "Inode");
        fprintf(file, "        ================================================\n");
    }
    else{
        printf("%-8s %-8s %-8s %-32s %s\n", "", "PID", "FD", "Filename", "Inode");
        printf("        ================================================\n");
    }
}

void printSummary(int pidCount, PIDEntry * pidTable){
    printf("Summary Table\n" );
    printf("================\n");
    for (int i = 0; i < pidCount; i++) {
        printf("%s (%d)", pidTable[i].pid, pidTable[i].count);
        if (i < pidCount - 1) {
            printf(",  ");
        }
    }
    printf("\n");
}

void printThreshold(int pidCount, PIDEntry * pidTable, int threshold_val){
    printf("## Offending processes -- #FD threshold=%d\n", threshold_val);
    for (int i = 0; i < pidCount; i++) {
        if (pidTable[i].count > threshold_val){
            printf("%s (%d)", pidTable[i].pid, pidTable[i].count);
            if (i < pidCount - 1) {
                printf(",  ");
            }
        }
    }
    printf("\n");
}

void printData(int pre_process, int systemWide, int Vnodes, int * row, struct dirent * entry, struct dirent * fd_entry, char * target_path, struct stat statbuf, FILE* file, int output_TXT){
    if (pre_process){
        printf("%-8d %-8s %-8s\n", (*row)++, entry->d_name, fd_entry->d_name);
    }
    else if (systemWide){
        printf("%-8d %-8s %-8s %-32s\n", (*row)++, entry->d_name, fd_entry->d_name, target_path);
    }
    else if (Vnodes){
        printf("%-8d %ld\n", (*row)++, (long)statbuf.st_ino);
    }
    else if (output_TXT){
        fprintf(file, "%-8d %-8s %-8s %-32s %ld\n", (*row)++, entry->d_name, fd_entry->d_name, target_path, (long)statbuf.st_ino);
    }
    else{
        printf("%-8d %-8s %-8s %-32s %ld\n", (*row)++, entry->d_name, fd_entry->d_name, target_path, (long)statbuf.st_ino);
    }
}

void processData(int pre_process, int systemWide, int Vnodes, int summary, int* pidCount, PIDEntry** pidTable, int threshold, int target_pid, FILE* file, int output_TXT){
    int row = 0;

    DIR *proc = opendir(PROC_PATH);
    if (!proc) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && isNumber(entry->d_name)) {
            int current_pid = atoi(entry->d_name);
            if (target_pid > 0 && current_pid != target_pid) {
                continue;
            }
            char fd_path[MAX_PATH_LENGTH];
            strncpy(fd_path, PROC_PATH, sizeof(fd_path) - 1);
            fd_path[sizeof(fd_path) - 1] = '\0';
            
            strncat(fd_path, "/", sizeof(fd_path) - strlen(fd_path) - 1);
            strncat(fd_path, entry->d_name, sizeof(fd_path) - strlen(fd_path) - 1);
            strncat(fd_path, "/fd", sizeof(fd_path) - strlen(fd_path) - 1);
            DIR *dir = opendir(fd_path);
            if (!dir) continue;

            struct dirent *fd_entry;
            while ((fd_entry = readdir(dir)) != NULL) {
                if (fd_entry->d_type != DT_LNK) continue;

                char link_path[MAX_PATH_LENGTH], target_path[MAX_PATH_LENGTH];
                strncpy(link_path, fd_path, sizeof(link_path) - 1);
                link_path[sizeof(link_path) - 1] = '\0';

                strncat(link_path, "/", sizeof(link_path) - strlen(link_path) - 1);
                strncat(link_path, fd_entry->d_name, sizeof(link_path) - strlen(link_path) - 1);
                
                ssize_t len = readlink(link_path, target_path, sizeof(target_path) - 1);
                if (len != -1) {
                    target_path[len] = '\0';
                    struct stat statbuf;
                    if (stat(link_path, &statbuf) == 0) {
                        if (summary || threshold) {
                            int found = 0;
                            for (int i = 0; i < *pidCount; i++) {
                                if (strcmp((*pidTable)[i].pid, entry->d_name) == 0) {
                                    (*pidTable)[i].count++;
                                    found = 1;
                                    break;
                                }
                            }
                            if (!found && *pidCount < MAX_PIDS) {
                                (*pidTable)[*pidCount].pid = entry->d_name;
                                (*pidTable)[*pidCount].count = 1;
                                (*pidCount)++;
                            }
                        } else {
                            printData(pre_process, systemWide, Vnodes, &row, entry, fd_entry, target_path, statbuf, file, output_TXT);
                        }
                    }
                }
            }
            closedir(dir);
        }
    }
    closedir(proc);
}

int main(int argc, char ** argv) {
    int pre_process = 0;
    int systemWide = 0;
    int Vnodes = 0;
    int composite = 0;
    int summary = 0;
    int threshold = 0;
    int threshold_val;
    int flag_detected = 0;
    pid_t target_pid = 0;
    int output_TXT = 1;
    
    PIDEntry* pidTable = malloc(MAX_PIDS * sizeof(PIDEntry));
    int pidCount = 0;

    FILE *file = fopen("compositeTable.txt", "w");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], "--pre-process", 13) == 0) {
            pre_process = 1;
            flag_detected = 1;
        } 
        else if (strncmp(argv[i], "--systemWide", 12) == 0) {
            systemWide = 1;
            flag_detected = 1;
        }
        else if (strncmp(argv[i], "--Vnodes", 8) == 0) {
            Vnodes = 1;
            flag_detected = 1;
        }
        else if (strncmp(argv[i], "--composite", 11) == 0) {
            composite = 1;
            flag_detected = 1;
        }
        else if (strncmp(argv[i], "--summary", 9) == 0) {
            summary = 1;
            flag_detected = 1;
        }
        else if (i == 1 && isdigit(argv[i][0])) { 
            target_pid = atoi(argv[i]);
        }
        else if (strncmp(argv[i], "--threshold=", 12) == 0) {
            threshold = 1;
            threshold_val = atoi(argv[i] + 12);
        }
        else if (strncmp(argv[i], "--output_TXT", 12) == 0) {
            output_TXT = 1;
            flag_detected = 1;
        }
    }

    if (pre_process) {
        printHeader(pre_process, 0, 0, file, 0);
        processData(pre_process, 0, 0, 0, &pidCount, &pidTable, 0, target_pid, file, 0);
    }
    if (systemWide) {
        printHeader(0, systemWide, 0, file, 0);
        processData(0, systemWide, 0, 0, &pidCount, &pidTable, 0, target_pid, file, 0);
    }
    if (Vnodes) {
        printHeader(0, 0, Vnodes, file, 0);
        processData(0, 0, Vnodes, 0, &pidCount, &pidTable, 0, target_pid, file, 0);
    }
    if (!flag_detected || composite){
        printHeader(0, 0, 0, file, 0);
        processData(0, 0, 0, 0, &pidCount, &pidTable, 0, target_pid, file, 0);
    }
    if (summary) {
        processData(0, 0, Vnodes, summary, &pidCount, &pidTable, 0, target_pid, file, 0);
        printSummary(pidCount, pidTable);
    }
    if (threshold){
        processData(0, 0, Vnodes, summary, &pidCount, &pidTable, threshold, target_pid, file, 0);
        printThreshold(pidCount, pidTable, threshold_val);
    }
    if (output_TXT){
        printHeader(0, 0, 0, file, output_TXT);
        processData(0, 0, 0, 0, &pidCount, &pidTable, 0, target_pid, file, output_TXT);
    }
    
    fclose(file);
    free(pidTable);
    return 0;
}
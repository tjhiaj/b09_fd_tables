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
#define FLAG_PRE_PROCESS 1
#define FLAG_SYSTEM_WIDE 2
#define FLAG_VNODES 3
#define FLAG_OUTPUT_TXT 4
#define FLAG_OUTPUT_BINARY 5
#define FLAG_SUMMARY 6
#define FLAG_THRESHOLD 7

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

void printHeader(int flag, FILE* file) {
    if (flag == FLAG_PRE_PROCESS) {
        // pre-process
        printf("%-8s %-8s %-8s\n", "", "PID", "FD");
        printf("        =============\n");
    }
    else if (flag == FLAG_SYSTEM_WIDE){
        // systemWide
        printf("%-8s %-8s %-8s %-32s\n", "", "PID", "FD", "Filename");
        printf("        ============================\n");    
    }
    else if (flag == FLAG_VNODES){
        // Vnodes
        printf("%-8s %s\n", "", "Inode");
        printf("        ================\n");
    }
    else if (flag == FLAG_OUTPUT_TXT){
        // output_TXT
        fprintf(file, "%-8s %-8s %-8s %-32s %s\n", "", "PID", "FD", "Filename", "Inode");
        fprintf(file, "        ================================================\n");
    }
    else if (flag == FLAG_OUTPUT_BINARY){
        // output_binary
        fwrite("        PID      FD       Filename                       Inode\n", sizeof(char), 55, file);
        fwrite("        ================================================\n", sizeof(char), 49, file);
    }
    else{
        // comoposite or no flags
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

void printData(int flag, int * row, struct dirent * entry, struct dirent * fd_entry, char * target_path, struct stat statbuf, FILE* file){
    if (flag == FLAG_PRE_PROCESS){
        printf("%-8d %-8s %-8s\n", (*row)++, entry->d_name, fd_entry->d_name);
    }
    else if (flag == FLAG_SYSTEM_WIDE){
        printf("%-8d %-8s %-8s %-32s\n", (*row)++, entry->d_name, fd_entry->d_name, target_path);
    }
    else if (flag == FLAG_VNODES){
        printf("%-8d %ld\n", (*row)++, (long)statbuf.st_ino);
    }
    else if (flag == FLAG_OUTPUT_TXT){
        fprintf(file, "%-8d %-8s %-8s %-32s %ld\n", (*row)++, entry->d_name, fd_entry->d_name, target_path, (long)statbuf.st_ino);
    }
    else if (flag == FLAG_OUTPUT_BINARY){
        fwrite(row, sizeof(int), 1, file);
        fwrite(entry->d_name, sizeof(char), strlen(entry->d_name) + 1, file);
        fwrite(fd_entry->d_name, sizeof(char), strlen(fd_entry->d_name) + 1, file);
        fwrite(target_path, sizeof(char), strlen(target_path) + 1, file);
        fwrite(&statbuf.st_ino, sizeof(ino_t), 1, file);
    }
    else{
        printf("%-8d %-8s %-8s %-32s %ld\n", (*row)++, entry->d_name, fd_entry->d_name, target_path, (long)statbuf.st_ino);
    }
}

void updatePIDTable(int * pidCount, PIDEntry** pidTable, struct dirent *entry) {
    for (int i = 0; i < *pidCount; i++) {
        if (strcmp((*pidTable)[i].pid, entry->d_name) == 0) {
            (*pidTable)[i].count++;
            return;
        }
    }
    if (*pidCount < MAX_PIDS) {
        (*pidTable)[*pidCount].pid = entry->d_name;
        (*pidTable)[*pidCount].count = 1;
        (*pidCount)++;
    }
}

void processFD(int flag, int row, int* pidCount, PIDEntry** pidTable, int target_pid, FILE* file, char * fd_path, struct dirent *entry, DIR *dir) {
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
                if (flag == FLAG_SUMMARY || flag == FLAG_THRESHOLD) {
                    updatePIDTable(pidCount, pidTable, entry);
                } else {
                    printData(flag, &row, entry, fd_entry, target_path, statbuf, file);
                }
            }
        }
    }
    closedir(dir);
}

void processDirectory(int flag, int* pidCount, PIDEntry** pidTable, int target_pid, FILE* file) {
    int row = 0;

    if (target_pid > 0) {
        char fd_path[MAX_PATH_LENGTH];
        strncpy(fd_path, PROC_PATH, sizeof(fd_path) - 1);
        fd_path[sizeof(fd_path) - 1] = '\0';
        strncat(fd_path, "/", sizeof(fd_path) - strlen(fd_path) - 1);

        char pid_str[20];
        sprintf(pid_str, "%d", target_pid);
        strncat(fd_path, pid_str, sizeof(fd_path) - strlen(fd_path) - 1);
        strncat(fd_path, "/fd", sizeof(fd_path) - strlen(fd_path) - 1);
        
        DIR *dir = opendir(fd_path);
        if (!dir) {
            perror("opendir failed");
            return;
        }

        struct dirent entry;
        sprintf(entry.d_name, "%d", target_pid);

        processFD(flag, row, pidCount, pidTable, target_pid, file, fd_path, &entry, dir);
        return;
    }

    DIR *proc = opendir(PROC_PATH);
    if (!proc) {
        perror("opendir failed");
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

            processFD(flag, row, pidCount, pidTable, target_pid, file, fd_path, entry, dir);
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
    int output_TXT = 0;
    int output_binary = 0;
    
    PIDEntry* pidTable = malloc(MAX_PIDS * sizeof(PIDEntry));
    if (!pidTable) {
        perror("malloc failed");
        return 1;
    }
    int pidCount = 0;

    FILE *file_txt = NULL;
    FILE *file_binary = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], "--pre-process", 14) == 0) {
            pre_process = 1;
            flag_detected = 1;
        } 
        else if (strncmp(argv[i], "--systemWide", 13) == 0) {
            systemWide = 1;
            flag_detected = 1;
        }
        else if (strncmp(argv[i], "--Vnodes", 8) == 0) {
            Vnodes = 1;
            flag_detected = 1;
        }
        else if (strncmp(argv[i], "--composite", 12) == 0) {
            composite = 1;
            flag_detected = 1;
        }
        else if (strncmp(argv[i], "--summary", 10) == 0) {
            summary = 1;
            flag_detected = 1;
        }
        else if (i == 1 && isdigit(argv[i][0])) { 
            target_pid = atoi(argv[i]);
        }
        else if (strncmp(argv[i], "--threshold=", 13) == 0) {
            threshold = 1;
            threshold_val = atoi(argv[i] + 12);
        }
        else if (strncmp(argv[i], "--output_TXT", 13) == 0) {
            output_TXT = 1;
            flag_detected = 1;
        }
        else if (strncmp(argv[i], "--output_binary", 16) == 0) {
            output_binary = 1;
            flag_detected = 1;
        }
    }

    if (pre_process) {
        printHeader(FLAG_PRE_PROCESS, NULL);
        processDirectory(FLAG_PRE_PROCESS, &pidCount, &pidTable, target_pid, NULL);
    }
    if (systemWide) {
        printHeader(FLAG_SYSTEM_WIDE, NULL);
        processDirectory(FLAG_SYSTEM_WIDE, &pidCount, &pidTable, target_pid, NULL);
    }
    if (Vnodes) {
        printHeader(FLAG_VNODES, NULL);
        processDirectory(FLAG_VNODES, &pidCount, &pidTable, target_pid, NULL);
    }
    if (!flag_detected || composite){
        printHeader(0, NULL);
        processDirectory(0, &pidCount, &pidTable, target_pid, NULL);
    }
    if (summary) {
        processDirectory(FLAG_SUMMARY, &pidCount, &pidTable, target_pid, NULL);
        printSummary(pidCount, pidTable);
    }
    if (threshold){
        processDirectory(FLAG_THRESHOLD, &pidCount, &pidTable, target_pid, NULL);
        printThreshold(pidCount, pidTable, threshold_val);
    }
    if (output_TXT){
        file_txt = fopen("compositeTable.txt", "w");
        if (!file_txt) {
            perror("fopen failed");
            return 1;
        }
        printHeader(FLAG_OUTPUT_TXT, NULL);
        processDirectory(FLAG_OUTPUT_TXT, &pidCount, &pidTable, target_pid, NULL);
        fclose(file_txt);
    }
    if (output_binary){
        file_binary = fopen("compositeTable.bin", "wb");
        if (!file_binary) {
            perror("fopen failed");
            return 1;
        }
        printHeader(FLAG_OUTPUT_BINARY, NULL);
        processDirectory(FLAG_OUTPUT_BINARY, &pidCount, &pidTable, target_pid, NULL);
        fclose(file_binary);
    }
    
    free(pidTable);
    return 0;
}
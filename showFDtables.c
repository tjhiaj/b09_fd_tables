#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#define PROC_PATH "/proc"
#define MAX_PATH_LENGTH 512

int isNumber(char* input) {
    int length = strlen(input);
    for (int i = 0; i < length; i++) {
        if (!isdigit(input[i])) {
            return 0;
        }
    }
    return 1;
}

void printHeader(int pre_process){
    if (pre_process){
        printf("%-8s %-8s %-8s\n", "", "PID", "FD");
        printf("        =============\n");
    }
    else{
        printf("%-8s %-8s %-8s %-32s %s\n", "", "PID", "FD", "Filename", "Inode");
        printf("        ================================================\n");
    }
}

void printData(int pre_process, int * row, struct dirent * entry, struct dirent * fd_entry, char * target_path, struct stat statbuf){
    if (pre_process){
        printf("%-8d %-8s %-8s\n", (*row)++, entry->d_name, fd_entry->d_name);
    }
    else{
        printf("%-8d %-8s %-8s %-32s %ld\n", (*row)++, entry->d_name, fd_entry->d_name, target_path, (long)statbuf.st_ino);
    }
}

int main(int argc, char ** argv) {
    int row = 0;
    int pre_process = 0;
    int flag_detected = 0;

    for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], "--pre-process", 12) == 0) {
            pre_process = 1;
            flag_detected = 1;
        } 
    }

    printHeader(pre_process);

    DIR *proc = opendir(PROC_PATH);
    if (!proc) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && isNumber(entry->d_name)) {
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
                        printData(pre_process, &row, entry, fd_entry, target_path, statbuf);
                    }
                }
            }
            closedir(dir);
        }
    }
    closedir(proc);
    return 0;
}
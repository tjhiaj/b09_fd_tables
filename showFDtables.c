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

int main() {
    int row = 0;

    printf("%-8s %-8s %-8s %-32s %s\n", "", "PID", "FD", "Filename", "Inode");
    printf("        ================================================\n");

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
                        printf("%-8d %-8s %-8s %-32s %ld\n", row++, entry->d_name, fd_entry->d_name, target_path, (long)statbuf.st_ino);
                    }
                }
            }
            closedir(dir);
        }
    }
    closedir(proc);
    return 0;
}
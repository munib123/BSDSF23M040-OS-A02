#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

void do_ls(char *dirname) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir(dirname);
    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;  // Skip hidden files
        printf("%s\n", entry->d_name);  // Print in single column
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        do_ls(".");  // Current directory if no args
    } else {
        for (int i = 1; i < argc; i++) {
            do_ls(argv[i]);
        }
    }
    return 0;
}
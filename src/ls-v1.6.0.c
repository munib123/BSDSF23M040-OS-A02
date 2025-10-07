#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// ANSI color codes for file type colorization  
#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_PINK "\033[0;35m"
#define COLOR_REVERSE "\033[7m"

int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void print_colored(char *dirname, char *name) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dirname, name);
    struct stat st;
    lstat(path, &st);
    char *color = COLOR_RESET;
    if (S_ISDIR(st.st_mode)) color = COLOR_BLUE;
    else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) color = COLOR_GREEN;  // Executable
    else if (strstr(name, ".tar") || strstr(name, ".gz") || strstr(name, ".zip")) color = COLOR_RED;
    else if (S_ISLNK(st.st_mode)) color = COLOR_PINK;
    else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) color = COLOR_REVERSE;
    printf("%s%s%s", color, name, COLOR_RESET);
}

void mode_to_str(mode_t mode, char *str) {
    strcpy(str, "----------");
    if (S_ISDIR(mode)) str[0] = 'd';
    else if (S_ISLNK(mode)) str[0] = 'l';
    else if (S_ISCHR(mode)) str[0] = 'c';
    else if (S_ISBLK(mode)) str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';

    if (mode & S_IRUSR) str[1] = 'r';
    if (mode & S_IWUSR) str[2] = 'w';
    if (mode & S_IXUSR) str[3] = 'x';
    if (mode & S_IRGRP) str[4] = 'r';
    if (mode & S_IWGRP) str[5] = 'w';
    if (mode & S_IXGRP) str[6] = 'x';
    if (mode & S_IROTH) str[7] = 'r';
    if (mode & S_IWOTH) str[8] = 'w';
    if (mode & S_IXOTH) str[9] = 'x';
    // Handle special bits if needed (setuid, etc.)
    if (mode & S_ISUID) str[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID) str[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX) str[9] = (mode & S_IXOTH) ? 't' : 'T';
}

void print_horizontal(char *dirname, char **files, int count) {
    int max_len = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (len > max_len) max_len = len;
    }
    int col_width = max_len + 2;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col ? w.ws_col : 80;

    int current_pos = 0;
    for (int i = 0; i < count; i++) {
        if (current_pos + col_width > term_width) {
            printf("\n");
            current_pos = 0;
        }
        print_colored(dirname, files[i]);
        // Add padding to match column width
        int name_len = strlen(files[i]);
        for (int j = name_len; j < col_width; j++) {
            printf(" ");
        }
        current_pos += col_width;
    }
    printf("\n");
}

void do_ls(char *dirname, int long_format, int horizontal, int recursive) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) { 
        perror("opendir"); 
        exit(1); 
    }
    
    // Print directory header if recursive
    if (recursive) {
        printf("%s:\n", dirname);
    }
    struct dirent *entry;
    char **files = NULL;
    int count = 0, capacity = 10;
    files = malloc(capacity * sizeof(char*));
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (count >= capacity) {
            capacity *= 2;
            files = realloc(files, capacity * sizeof(char*));
        }
        files[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);

    // Sort files alphabetically
    qsort(files, count, sizeof(char*), compare);

    if (long_format) {
        // To align columns, find max lengths for links, size, etc.
        int max_links = 0, max_size = 0;
        for (int i = 0; i < count; i++) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
            struct stat st;
            lstat(path, &st);
            int links = (int)st.st_nlink;
            if (links > max_links) max_links = links;
            int size = (int)st.st_size;
            if (size > max_size) max_size = size;
        }
        int links_width = snprintf(NULL, 0, "%d", max_links);
        int size_width = snprintf(NULL, 0, "%d", max_size);

        for (int i = 0; i < count; i++) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
            struct stat st;
            lstat(path, &st);
            char perm[11];
            mode_to_str(st.st_mode, perm);
            struct passwd *pwd = getpwuid(st.st_uid);
            struct group *grp = getgrgid(st.st_gid);
            char *time_str = ctime(&st.st_mtime);
            time_str[strlen(time_str)-1] = '\0';  // Remove newline
            printf("%s %*d %s %s %*d %s ",
                perm,
                links_width, (int)st.st_nlink,
                pwd ? pwd->pw_name : "unknown",
                grp ? grp->gr_name : "unknown",
                size_width, (int)st.st_size,
                time_str + 4);  // Skip day of week
            print_colored(dirname, files[i]);
            printf("\n");
        }
    } else {
        if (horizontal) {
            print_horizontal(dirname, files, count);
        } else {
            // Find max filename length
            int max_len = 0;
            for (int i = 0; i < count; i++) {
                int len = strlen(files[i]);
                if (len > max_len) max_len = len;
            }
            int col_width = max_len + 2;  // Spacing

            // Get terminal width
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            int term_width = w.ws_col;
            if (term_width <= 0) term_width = 80;  // Fallback

            int num_cols = term_width / col_width;
            if (num_cols < 1) num_cols = 1;
            int num_rows = (count + num_cols - 1) / num_cols;

            // Print in "down then across" format
            for (int row = 0; row < num_rows; row++) {
                for (int col = 0; col < num_cols; col++) {
                    int idx = col * num_rows + row;
                    if (idx < count) {
                        print_colored(dirname, files[idx]);
                        // Add padding to match column width
                        int name_len = strlen(files[idx]);
                        for (int j = name_len; j < col_width; j++) {
                            printf(" ");
                        }
                    }
                }
                printf("\n");
            }
        }
    }
    
    // Recursive processing of subdirectories
    if (recursive) {
        for (int i = 0; i < count; i++) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dirname, files[i]);
            struct stat st;
            lstat(path, &st);
            if (S_ISDIR(st.st_mode) && strcmp(files[i], ".") != 0 && strcmp(files[i], "..") != 0) {
                printf("\n");  // Separate sections
                do_ls(path, long_format, horizontal, recursive);  // Recursive call
            }
        }
    }
    
    // Free memory
    for (int i = 0; i < count; i++) free(files[i]);
    free(files);
}

int main(int argc, char *argv[]) {
    int long_format = 0;
    int horizontal = 0;
    int recursive = 0;
    int opt;
    
    while ((opt = getopt(argc, argv, "lxR")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            case 'x':
                horizontal = 1;
                break;
            case 'R':
                recursive = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [dir]\n", argv[0]);
                exit(1);
        }
    }
    
    // Handle remaining arguments (directories)
    if (optind >= argc) {
        do_ls(".", long_format, horizontal, recursive);  // Current directory if no args
    } else {
        for (int i = optind; i < argc; i++) {
            do_ls(argv[i], long_format, horizontal, recursive);
        }
    }
    return 0;
}
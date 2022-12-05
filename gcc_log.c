#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int fds[2];
    pipe(fds);
    pid_t pid = fork();
    if (pid == 0) {
        pid_t _pid = fork();
        if (_pid == 0) {
            close(fds[0]);
            dup2(fds[1], 2);
            close(fds[1]);
            execlp("gcc", "gcc", argv[1], "-o", "program", NULL);

            
        } else {
            waitpid(pid, NULL, 0);
            struct stat file_stat;
            if (lstat("program", &file_stat) != -1)
                execlp("unlink", "unlink", "program", NULL);
        }

    } else {
        waitpid(pid, NULL, 0);
        close(fds[1]);
        ssize_t errors_num = 0;
        ssize_t warnings_num = 0;
        ssize_t last_str_error = -1;
        ssize_t last_str_warning = -1;
        ssize_t tmp_str;
        size_t tmp_symbol;
        FILE *file = fdopen(fds[0], "r");
        char *line = NULL;
        char *ptr;
        unsigned long length = strlen(argv[1]);
        while (getline(&line, &tmp_symbol, file) != -1) {
            tmp_str = strtol(line + length + 1, &ptr, 10);
            strtol(ptr + 1, &ptr, 10);
            ptr += 2;
            if (strncmp(ptr, "warning", 7) == 0) {
                if (tmp_str != last_str_warning) {
                    warnings_num++;
                    last_str_warning = tmp_str;
                }
            } else if (strncmp(ptr, "error", 5) == 0) {
                if (tmp_str != last_str_error) {
                    errors_num++;
                    last_str_error = tmp_str;
                }
            }
        }
        printf("Unique lines with warnings: %ld\n"
               "Unique lines with errors: %ld\n", warnings_num, errors_num);

        close(fds[0]);
        fclose(file);
        return 0;
    }
}

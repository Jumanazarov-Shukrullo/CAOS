#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

int main (int argc, char** argv) {

    pid_t pid;
    const size_t size = 4096;
    char expression[size];
    memset(expression, '\0', sizeof(expression));
    fgets(expression, size, stdin);

    for (size_t i = 0; i < size; ++i) {
        if (expression[i] == '\n') {
            expression[i] = '\0';
            break;
        }
    }
    char *file_name = "expr.cpp";
    int fd = open(file_name, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR) ;
    char *cpp_file = "#include <iostream>\n"
                     "#include <stdio.h>\n"
                     "int main() {\n"
                     "int res = %s;\n"
                     "std::cout << res << std::endl;\n"
                     "}\n";
    dprintf(fd, cpp_file, expression);
    if ((pid = fork()) == 0) {
        execlp("g++", "g++", file_name, "-o", "program", NULL);
        return 0;
    }

    waitpid(pid, NULL, 0);


    if ((pid = fork()) == 0) {
        execlp("./program", "./program", NULL);
        return 0;
    }

    waitpid(pid, NULL, 0);


    if ((pid = fork()) == 0) {
        execlp("unlink", "unlink", "program", NULL);
        return 0;
    }

    waitpid(pid, NULL, 0);

    if ((pid = fork()) == 0) {
        execlp("unlink", "unlink", "expr.cpp", NULL);
        return 0;
    }
    close(fd);
}

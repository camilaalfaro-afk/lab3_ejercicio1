#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

int main(void) {
    char input[MAX_LINE];
    char *args[MAX_ARGS];

    pid_t pid;
    int status;

    while (1) {
        printf("mini-shell> ");
        fflush(stdout);

        if (fgets(input, MAX_LINE, stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        int i = 0;
        args[i] = strtok(input, " ");

        while (args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, " ");
        }

        args[i] = NULL;

        if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "salir") == 0) {
            break;
        }

        pid = fork();

        if (pid < 0) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execvp(args[0], args);
            perror("Error en execvp");
            exit(EXIT_FAILURE);
        } else {
            waitpid(pid, &status, 0);
        }
    }

    return EXIT_SUCCESS;
}

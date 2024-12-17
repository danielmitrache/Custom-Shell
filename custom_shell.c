#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define SUCCES_EXIT 0
#define FAILURE_EXIT 1
#define VSH_DELIMITERS " \t\r\a\n"
#define NUM_COMMS 4
#define MAX_PATH_SIZE 256

int vsh_goto(char **args);
int vsh_exit(char **args);
int vsh_di(char **args);
int vsh_back(char **args);

char *builtin_comms[] = {
    "goto",
    "exit",
    "di",
    "back"
};

int (*builtin_func[]) (char **) = {
    &vsh_goto,
    &vsh_exit,
    &vsh_di,
    &vsh_back
};

char last_working_directory[MAX_PATH_SIZE], working_directory[MAX_PATH_SIZE];

char** get_arguments(char *command) {
    char **arguments = malloc(BUFFER_SIZE * sizeof(char*));
    char *argument;
    int index = 0;

    argument = strtok(command, VSH_DELIMITERS);
    while (argument) {
        arguments[index ++] = argument;
        argument = strtok(NULL, VSH_DELIMITERS);
    }
    arguments[index] = 0;
    return arguments;
}

int read_command(char *command) {
    printf("%s >>> ", working_directory);
    if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
        return 1;
    }

    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "exit") == 0) {
        return 1;
    }
    return 0;
}

int vsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("vsh");
        }
        exit(FAILURE_EXIT);
    }
    else if(pid < 0) {
        perror("vsh");
    }
    else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

int get_num_args(char **args) {
    int num_args = 0;
    while(args[num_args ++]);
    return num_args - 1;
}

int vsh_goto(char **args) {
    int num_args = get_num_args(args);
    strcpy(last_working_directory, working_directory);
    printf("%s\n", last_working_directory);
    if(num_args != 2) {
        fprintf(stderr, "Syntax: goto <path>");
        return 1;
    }
    else if(chdir(args[1]) != 0) {
        perror("vsh");
        return 1;
    }
    if(getcwd(working_directory, sizeof(working_directory)) == NULL){
        perror("vsh");
        return 1;
    }
    return 0;
}

int vsh_exit(char **args) {
    return 0;
}

int vsh_di(char **args) {
    int num_args = get_num_args(args);
    char *path = ".";
    if(num_args == 2) {
        path = args[1];
    }

    DIR *dir = opendir(path);
    if(dir == NULL){
        perror("vsh");
        return 1;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry -> d_name);
    }
    closedir(dir);
    return 0;
}

int vsh_back(char **args) {
    if(chdir(last_working_directory) != 0) {
        perror("vsh");
        return 1;
    }
    char *temp = malloc(MAX_PATH_SIZE * sizeof(char));
    strcpy(temp, last_working_directory);
    strcpy(last_working_directory, working_directory);
    strcpy(working_directory, temp);
    free(temp);
    return 0;
}

int vsh_execute(char **args) {
    if(args[0] == NULL)
        return 1;

    for(int i = 0; i < NUM_COMMS; i ++) {
        if(strcmp(args[0], builtin_comms[i]) == 0) {
            return (*builtin_func[i]) (args);
        }
    }

    return vsh_launch(args);
}

void vsh_loop() {
    char *command = malloc(BUFFER_SIZE * sizeof(char)); 
    char **arguments;
    int status_code = 0;

    do {
        status_code = read_command(command);
        arguments = get_arguments(command);
        vsh_execute(arguments);
    } while (status_code == 0);
}

int main(void) {
    if(getcwd(working_directory, sizeof(working_directory)) == NULL){
        perror("vsh");
        return 1;
    }
    vsh_loop();
    return SUCCES_EXIT;
}

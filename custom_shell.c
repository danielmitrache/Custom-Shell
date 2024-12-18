#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h> 

#define BUFFER_SIZE 1024
#define VSH_DELIMITERS " \t\r\a\n"
#define NUM_COMMS 8
#define MAX_PATH_SIZE 256

int vsh_goto(char **args);
int vsh_exit(char **args);
int vsh_di(char **args);
int vsh_back(char **args);
int vsh_cf(char **args);
int vsh_rf(char **args);
int vsh_yap(char **args);
int vsh_look(char **args);

char *builtin_comms[] = {
    "goto",
    "exit",
    "di",
    "back",
    "cf",
    "rf",
    "yap",
    "look"
};

int (*builtin_func[]) (char **) = {
    &vsh_goto,
    &vsh_exit,
    &vsh_di,
    &vsh_back,
    &vsh_cf,
    &vsh_rf,
    &vsh_yap,
    &vsh_look
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

int get_num_args(char **args) {
    int num_args = 0;
    while(args[num_args ++]);
    return num_args - 1;
}

int vsh_goto(char **args) {
    int num_args = get_num_args(args);
    strcpy(last_working_directory, working_directory);

    if(num_args != 2) {
        fprintf(stderr, "Syntax: goto <path>\n");
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
        char *name = entry -> d_name;
        if(name[0] == '.')
            continue;
        printf("%s\n", name);
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

int vsh_cf(char **args) {
    int num_args = get_num_args(args);
    if(num_args < 2) {
        printf("Syntax: cf <file1> ...");
        return 1;
    }
    for(int i = 1; i < num_args; i ++) {
        char *filename = args[i];
        open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }
    return 0;
}

int vsh_rf(char **args) {
    int num_args = get_num_args(args);
    if (num_args < 2) {
        printf("Syntax: rf <file1> ...\n");
        return 1;
    }
    for (int i = 1; i < num_args; i++) {
        char *filename = args[i];
        if (unlink(filename) == -1) {
            perror("vsh");
        } else {
            printf("File '%s' removed successfully.\n", filename);
        }
    }
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
    printf("Command not found!\n");
    return 0;
}

int vsh_yap(char **args) {
    char *text, *filename;
    text = malloc(BUFFER_SIZE * sizeof(char));
    int print_to_stdout = 1;
    for(int i = 1; args[i]; i ++) {
        if(strcmp(args[i], "#=>") == 0) {
            if(!args[i + 1] || args[i + 2]) {
                printf("Syntax: yap <message> #=> <file>");
                return 1;
            }
            filename = args[++ i];
            print_to_stdout = 0;
            break;
        }
        strcat(text, args[i]);
        strcat(text, " ");
    }
    text[strlen(text) - 1] = '\n';
    if(print_to_stdout) {
        printf("%s", text);
    }
    else {
        int fd = open(filename, O_RDWR | O_APPEND | O_CREAT);
        if (fd < 0) {
            perror("Error opening the file");
            return 1;
        }
        if(write(fd, text, strlen(text)) < 0){
            perror("Error writing to file");
            return 1;
        }
        close(fd);
    }
    free(text);
    return 0;
}

int vsh_look(char **args) {
    int num_args = get_num_args(args);
    if(num_args != 2){
        printf("Syntax: look <file>");
        return 1;
    }
    int fd = open(args[1], O_RDONLY);
    if(fd < 0) {
        perror("Error opening the file");
        return 1;
    }
    struct stat stbuf;
    stat(args[1], &stbuf);
    int filesize = stbuf.st_size;
    char *buffer = malloc(filesize * sizeof(char));
    if(read(fd, buffer, filesize) < 0) {
        perror("Error reading from file");
        return 1;
    }
    printf("%s", buffer);
    free(buffer);
    close(fd);
    return 0;
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

int vsh_init() {
    if(getcwd(working_directory, sizeof(working_directory)) == NULL){
        perror("vsh");
        return 1;
    }
    strcpy(last_working_directory, working_directory);
    return 0;
}

int main(void) {
    if(vsh_init() != 0){
        perror("Error initializing!");
        return 1;
    }
    vsh_loop();
    return 0;
}

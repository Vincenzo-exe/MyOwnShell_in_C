#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  while (1) {
    printf("$ ");
    // Wait for user input
    char input[100];
    fgets(input, 100, stdin);
    // Remove the trailing newline
    input[strlen(input) - 1] = '\0';
    if (strcmp(input, "exit") == 0) {
      break;
    } else if (strcmp(input, "pwd") == 0) {
      // Implement pwd builtin
      char cwd[1024];
      if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
      } else {
        perror("getcwd failed");
      }
    } else if (strncmp(input, "cd ", 3) == 0) {
      // Implement cd builtin
      char *directory = input + 3;
      // Remove trailing whitespace if present
      int len = strlen(directory);
      while (len > 0 && (directory[len-1] == ' ' || directory[len-1] == '\t' || directory[len-1] == '\n')) {
        directory[len-1] = '\0';
        len--;
      }
      
      // Handle ~ expansion
      char expanded_path[1024];
      if (directory[0] == '~') {
        const char *home = getenv("HOME");
        if (home == NULL) {
          printf("cd: %s: No such file or directory\n", directory);
        } else {
          // Replace ~ with home directory
          if (directory[1] == '\0') {
            strcpy(expanded_path, home);
          } else if (directory[1] == '/') {
            snprintf(expanded_path, sizeof(expanded_path), "%s%s", home, directory + 1);
          } else {
            printf("cd: %s: No such file or directory\n", directory);
            continue;
          }
          directory = expanded_path;
          
          if (chdir(directory) != 0) {
            printf("cd: %s: No such file or directory\n", input + 3);
          }
        }
      } else if (chdir(directory) != 0) {
        printf("cd: %s: No such file or directory\n", input + 3);
      }
    } else if (strncmp(input, "echo ", 5) == 0) {
      printf("%s\n", input + 5);
    } else if (strncmp(input, "type ", 5) == 0) {
      // Extract the argument after "type " command
      char *arg = input + 5;
      // Check if the argument is a shell builtin
      if (!strcmp(arg, "exit") || !strcmp(arg, "echo") || !strcmp(arg, "type") || !strcmp(arg, "pwd") || !strcmp(arg, "cd")) {
        printf("%s is a shell builtin\n", arg);
      } else {
        // Search for the command in PATH
        char *path_env = getenv("PATH");
        if (path_env == NULL) {
          printf("%s: not found\n", arg);
          continue;
        }
        
        // Create a copy of PATH since strtok modifies the string
        char *path_copy = malloc(strlen(path_env) + 1);
        strcpy(path_copy, path_env);
        
        int found = 0;
        char *directory = strtok(path_copy, ":");
        
        while (directory != NULL && !found) {
          // Build the full path: directory/command
          char full_path[512];
          snprintf(full_path, sizeof(full_path), "%s/%s", directory, arg);
          
          // Check if file exists and has execute permissions
          if (access(full_path, X_OK) == 0) {
            printf("%s is %s\n", arg, full_path);
            found = 1;
          }
          
          directory = strtok(NULL, ":");
        }
        
        free(path_copy);
        
        if (!found) {
          printf("%s: not found\n", arg);
        }
      }
    } else {
      // Handle external program execution
      // Parse input into arguments
      char input_copy[100];
      strcpy(input_copy, input);
      
      // Count arguments
      int arg_count = 0;
      char *temp = input_copy;
      while (*temp) {
        if (*temp != ' ' && (temp == input_copy || *(temp - 1) == ' ')) {
          arg_count++;
        }
        temp++;
      }
      
      // Allocate argv array (arg_count + 1 for NULL terminator)
      char **args = malloc((arg_count + 1) * sizeof(char*));
      
      // Parse arguments
      strcpy(input_copy, input);
      args[0] = strtok(input_copy, " ");
      for (int i = 1; i < arg_count; i++) {
        args[i] = strtok(NULL, " ");
      }
      args[arg_count] = NULL;
      
      // Search for command in PATH
      char *path_env = getenv("PATH");
      if (path_env == NULL) {
        printf("%s: command not found\n", args[0]);
        free(args);
        continue;
      }
      
      char *path_copy = malloc(strlen(path_env) + 1);
      strcpy(path_copy, path_env);
      
      int found = 0;
      char full_path[512];
      char *directory = strtok(path_copy, ":");
      
      while (directory != NULL && !found) {
        snprintf(full_path, sizeof(full_path), "%s/%s", directory, args[0]);
        
        if (access(full_path, X_OK) == 0) {
          found = 1;
        } else {
          directory = strtok(NULL, ":");
        }
      }
      
      free(path_copy);
      
      if (found) {
        // Fork and execute
        pid_t pid = fork();
        if (pid == 0) {
          // Child process
          execv(full_path, args);
          perror("execv failed");
          exit(1);
        } else if (pid > 0) {
          // Parent process - wait for child
          wait(NULL);
        } else {
          perror("fork failed");
        }
      } else {
        printf("%s: command not found\n", args[0]);
      }
      
      free(args);
    }
  }
  return 0;
}
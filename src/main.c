#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

// Array of builtin commands for autocompletion
const char *builtins[] = {"exit", "echo", "type", "pwd", "cd", NULL};

// Autocompletion function
char *command_generator(const char *text, int state) {
  static int list_index, len;
  const char *name;
  
  if (!state) {
    list_index = 0;
    len = strlen(text);
  }
  
  while ((name = builtins[list_index++]) != NULL) {
    if (strncmp(name, text, len) == 0) {
      return malloc(strlen(name) + 1) ? strcpy(malloc(strlen(name) + 1), name) : NULL;
    }
  }
  
  return NULL;
}

char **command_completion(const char *text, int start, int end) {
  // Only complete at the start of the line
  if (start == 0) {
    rl_completion_append_character = ' ';
    return rl_completion_matches(text, command_generator);
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  
  // Set up readline autocompletion
  rl_attempted_completion_function = command_completion;
  
  while (1) {
    char *input = readline("$ ");
    
    // Handle EOF (Ctrl+D)
    if (input == NULL) {
      break;
    }
    
    // Skip empty input
    if (strlen(input) == 0) {
      free(input);
      continue;
    }
    
    // Add to history
    add_history(input);
    
    if (strcmp(input, "exit") == 0) {
      free(input);
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
            free(input);
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
          free(input);
          continue;
        }
        
        char *path_copy = malloc(strlen(path_env) + 1);
        strcpy(path_copy, path_env);
        
        int found = 0;
        char *directory = strtok(path_copy, ":");
        
        while (directory != NULL && !found) {
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
      char input_copy[100];
      strcpy(input_copy, input);
      
      int arg_count = 0;
      char *temp = input_copy;
      while (*temp) {
        if (*temp != ' ' && (temp == input_copy || *(temp - 1) == ' ')) {
          arg_count++;
        }
        temp++;
      }
      
      char **args = malloc((arg_count + 1) * sizeof(char*));
      
      strcpy(input_copy, input);
      args[0] = strtok(input_copy, " ");
      for (int i = 1; i < arg_count; i++) {
        args[i] = strtok(NULL, " ");
      }
      args[arg_count] = NULL;
      
      char *path_env = getenv("PATH");
      if (path_env == NULL) {
        printf("%s: command not found\n", args[0]);
        free(args);
        free(input);
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
        pid_t pid = fork();
        if (pid == 0) {
          execv(full_path, args);
          perror("execv failed");
          exit(1);
        } else if (pid > 0) {
          wait(NULL);
        } else {
          perror("fork failed");
        }
      } else {
        printf("%s: command not found\n", args[0]);
      }
      
      free(args);
    }
    
    free(input);
  }
  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    }else if (strncmp(input, "echo ", 5) == 0) {
      printf("%s\n", input + 5);
    } else if (strncmp(input, "type ", 5) == 0) {
      // Extract the argument after "type " command
      char *arg = input + 5;
      // Check if the argument is a shell builtin
      if (!strcmp(arg, "exit") || !strcmp(arg, "echo") || !strcmp(arg, "type"))
        printf("%s is a shell builtin\n", arg);
      else
        printf("%s: not found\n", arg);
    } else {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
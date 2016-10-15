/***************************************************************************//**

  @file         main.c

i  @author       Stephen Brennan

  @date         Thursday,  8 January 2015

  @brief        LSH (Libstephen SHell)

*******************************************************************************/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  Function Declarations for builtin shell commands:
 */
int cmd_cd(char **args);
int cmd_help(char **args);
int cmd_exit(char **args);
int cmd_pwd(char **args);
int cmd_date(char **args);
void cshell_intro();
/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "pw"
};

int (*builtin_func[]) (char **) = {
  &cmd_cd,
  &cmd_help,
  &cmd_exit,
  &cmd_pwd
};

int cmd_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/



void cshell_intro()
{
  printf("======= CUSTOM SHELL INITITATED =======\n");
  printf("Type 'help' to get a list of allowed commands\n");
}

int cmd_pwd(char **args)
{
  if (args[1] == NULL) {
    // run pwd command
    char pwd[1024];
    if(getcwd(pwd, sizeof(pwd)) !=NULL)
	printf("PWD:\n %s\n", pwd);
    else
	perror("getcwd() error occured");
  } 
  else {
    fprintf(stderr, "chsell: unexpected argument to \"pw\"\n");
  }

}

int cmd_date(char **args)
{
  // Check to see a command tail is provided

}

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int cmd_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "cshell: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("cshell");
    }
  }
  return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int cmd_help(char **args)
{
  int i;
  printf("\n======= HELP SECTION =======\n");
  printf("These are the following commands that can be used by this CustomShell\n");

  for (i = 0; i < cmd_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.");
  printf("\n======= HELP SECTION =======\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int cmd_exit(char **args)
{
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int cshell_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("cshell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("cshell");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int cshell_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < cmd_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return cshell_launch(args);
}

#define CSHELL_RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *cshell_read_line(void)
{
  int bufsize = CSHELL_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "cshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += CSHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "cshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define CSHELL_TOK_BUFSIZE 64
#define CSHELL_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **cshell_split_line(char *line)
{
  int bufsize = CSHELL_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "cshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, CSHELL_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += CSHELL_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "cshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, CSHELL_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void cshell_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("CustomShell> ");
    line = cshell_read_line();
    args = cshell_split_line(line);
    status = cshell_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  cshell_intro();
  cshell_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}


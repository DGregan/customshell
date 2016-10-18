
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h> // Used for cmd_date
#include <grp.h> // Used to retrieve group related info
#include <sys/types.h> // Used for gid, uid
#include <limits.h>

#define NUM_GROUPS_MAX 10 // Maximum number of groups to be display for cmd_userinfo()
/*
  Function Declarations for builtin shell commands:
 */
int cmd_cd(char **args);
int cmd_help(char **args);
int cmd_exit(char **args);
int cmd_pwd(char **args);
int cmd_date(char **args);
int cmd_ifconfig(char **args);
int cmd_userinfo(char **args);
void cshell_intro();
/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "pw",
  "dt",
  "ifc",
  "ud"
};


int (*builtin_func[]) (char **) = {
  &cmd_cd,
  &cmd_help,
  &cmd_exit,
  &cmd_pwd,
  &cmd_date,
  &cmd_ifconfig,
  &cmd_userinfo
};

int cmd_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief - Introduction to CustomShell
   @param - n/a
   @return Alawys returns 1, to continue executing
*/

void cshell_intro()
{
  printf("======= CUSTOM SHELL INITITATED =======\n");
  printf("Type 'help' to get a list of allowed commands\n");
  printf("======= CUSTOM SHELL INITITATED =======\n");

}

/**
   @brief - Allows user to use the 'pwd' command
   @param - checks if additional parameters are entered and negates them
   @return Alawys returns 1, to continue executing
*/

int cmd_pwd(char **args)
{
  if (args[1] == NULL) { // Check if command tail provided
    // run pwd command
    system("pwd");
  }
  else {
    fprintf(stderr, "chsell: unexpected argument to \"pw\"\n");
  }
  return 1;
}


/**
   @brief - Displays current date in the format YYYY-MM-DD-HH-MM-SS
   @param checks if additional parameters are entered and negates them
   @return Alawys returns 1, to continue executing
*/

int cmd_date(char **args)
{
  // Check to see a command tail is provided
  if (args[1] == NULL) {
   time_t cur_time;
   struct tm *tmp;
   char buffer[64];
   time(&cur_time);
   tmp = localtime(&cur_time);
   strftime(buffer, 64,"Time and Date(YYYY-MM-DD-HH-MM-SS): %Y%m%d%H%M%S", tmp);
   printf("%s\n", buffer);
   } else {
     fprintf(stderr, "cshell: unexpected argument to \"pw\"\n");
   }
  return 1;
}

/**
   @brief - Displays the ip configuration for Eth0 by default, accepts other Eth ports
   @param - Accepts ports but displays eth0 by default
   @return Alawys returns 1, to continue executing
*/

int cmd_ifconfig(char **args)
{
  if (args[1] == NULL)
    system("ifconfig eth0"); // Run command 'ifconfig eth0'
  else {
     if (args[1] !=NULL) { // Checks if command tail is provided
      char ifc_ethx[100]; // New var to hold specific ifc ethX selection
      sprintf(ifc_ethx, "ifconfig %s",args[1]);
      system(ifc_ethx); // Run command if additional argument added	
     }
  }
  return 1;
}

/**
   @brief - Displays UserID, GroupID, username, groupname and Inode of User home directory
   @param
   @return Alawys returns 1, to continue executing
*/

int cmd_userinfo(char **args)
{
  if (args[1] == NULL) {
    struct group *gr;
    int num_groups, i;
    gid_t groups[NUM_GROUPS_MAX];
    num_groups = NUM_GROUPS_MAX;

    if( getgrouplist( getlogin(), getegid(), groups, &num_groups) == -1)
	printf("Group array is too small: %d\n", num_groups); // Error check for group size
    
    // Display UserId and GroupID
    printf("%d, %s," , geteuid(), getenv("LOGNAME"));
    // Display all groups registered to this User
    for(i=0; i< num_groups; i++) {
	gr = getgrgid(groups[i]);
	printf(" %s,", gr->gr_name);
    }
    // Display home directory
    printf(" %s\n", getcwd(getenv("HOME"), 1024));
  }
  else {
    fprintf(stderr, "chsell: unexpected argument to \"uid\"\n");
  }
  
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
      fprintf(stderr, "cshell: unexpected directory parameter used.\n");
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

	// Error check for memory allocation
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

	// Error check for token allocation
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
  // Run command loop.
  system("clear");
  cshell_intro();
  cshell_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <dirent.h> 

#define MAX_LINE 4095 // The maximum length command
#define LINE_BUFFER MAX_LINE + 3
#define MAX_COMMANDS 4096
int historyIncrement = 0;
char *history[MAX_COMMANDS]; // to store the command line in the implementation of the hist command

typedef struct {
  int argc;
  char **argv;
} command_t;

typedef struct {
  char *name;
  int (*function)(int argc, char *argv[]);
} cmd_entry;

int parse_args(char *line, char ***p_argv) {
  int argc = 0;
  char **argv = *p_argv;

  for (argc = 0, argv[argc] = strtok(line," \t\n"); argv[argc] != NULL && argc < _POSIX_ARG_MAX; argv[++argc] = strtok(NULL," \t\n"));
  return argc;
}

int author()
{
  printf("Name: Nick Cox\n");
  return 0;
}

int exitCmd()
{
  exit(0);
}

int cdir(int argc, char *argv[]) {
  char path[MAXPATHLEN];

  if (argc == 1) {
    if (getcwd(path, MAXPATHLEN) == NULL) perror("cdir");
    else printf("%s\n",path);
    return 0;
  }
  if (chdir(argv[1]) == -1) perror("cdir");
  return 0;
}

int create(int argc, char *command[])
{
  char *directory = "-d";

  if (argc == 1) {
    fprintf(stderr, "%s\n", "usage: create [-d] name");
  }
  else if (argc == 2) {
    FILE *fp;

    if (!strcmp(command[1], directory))
      fprintf(stderr, "%s\n", "usage: create [-d] name");
    else {
      if ((fp = fopen(command[1], "r")) != NULL)
        fprintf(stderr, "%s\n", "create: File exists");
      else {
        fp = fopen(command[1], "w");
        fclose(fp);
      }
    }
  }
  else if (argc == 3) {
    if (!strcmp(command[1], directory)) {
      DIR* dir = opendir(command[2]);
      
      if (dir) {
        fprintf(stderr, "%s\n", "create: File exists");
        closedir(dir);
      }
      else
        mkdir(command[2], 0700);
    }
    else
      fprintf(stderr, "%s\n", "usage: create [-d] name");
  }
  return 0;
}

int delete(int argc, char *command[])
{
  char *removeArg = "-r";

  if (argc == 1) {
    fprintf(stderr, "%s\n", "usage: delete [-r] name");
  }
  else if (argc == 2) {
    if (!strcmp(command[1], removeArg))
      fprintf(stderr, "%s\n", "usage: delete [-r] name");
    else {
      DIR* dir = opendir(command[1]);

      if (dir) {
        if (rmdir(command[1]) == -1) perror("delete");
        closedir(dir);
      }
      else
        remove(command[1]);
    }
  }
  else if (argc == 3) {
    if (!strcmp(command[1], removeArg)) {
      remove(command[2]);
    }
    else
      fprintf(stderr, "%s\n", "usage: delete [-r] name");
  }
  return 0;
}

int list()
{
  DIR* currentDir;
  struct dirent *dir;
  currentDir = opendir(".");

  if (currentDir) {
    while ((dir = readdir(currentDir)) != NULL) {
      printf("%s\n", dir->d_name);
    }
    closedir(currentDir);
  }
  return 0;
}

int hist(int argc, char *command[])
{
  if (argc == 1) {
    for (int i = 0; i < historyIncrement; i++) {
      printf("%s", history[i]);
    }
  }
  else {
    memset(history, 0, historyIncrement);
    historyIncrement = 0;
  }
  return 0;
}

int myecho(int argc, char *command[])
{
  return 0;
}

cmd_entry cmd_table[] = {
  {"author", author},
  {"exit", exitCmd},
  {"cdir", cdir},
  {"create", create},
  {"delete", delete},
  {"list", list},
  {"hist", hist},
  {"myecho", myecho},
  {NULL, NULL}
};

void print_prompt(void)
{
  printf("@> ");
}

int get_command(command_t *cmd, int echo) {
  static char line_buffer[LINE_BUFFER];
  clearerr(stdin);

  if (fgets(line_buffer, LINE_BUFFER, stdin) == NULL) {
    if (ferror(stdin))perror("myshell");
    else exit(0);
  }
  if (strnlen(line_buffer, LINE_BUFFER) > MAX_LINE + 1) { // MAX_LINE + 1 accounts for the end line character
    fprintf(stderr, "command too long\n");
    cmd->argc = -1;
    return cmd->argc;
  }
  if (echo) printf("%s", line_buffer);
  history[historyIncrement] = strndup(line_buffer, LINE_BUFFER);
  historyIncrement++;
  cmd->argc = parse_args(line_buffer, &cmd->argv);
  return cmd->argc;
}

void process_command(command_t *command) {
  int i;

  if (command->argc <= 0)
    return;
  for (i=0; ; i++) {
    if (cmd_table[i].name == NULL) {
      fprintf(stderr, "Unrecognized command: %s\n", command->argv[0]);
      return;
    }
    if (!strcmp(command->argv[0], cmd_table[i].name)) {
      cmd_table[i].function(command->argc, command->argv);
      return;
    }
  }
}

int main(int argc, char *argv[]) {
  int echo = argc > 1 && !strcmp(argv[1], "--echo");
  static char *arg_buffer[_POSIX_ARG_MAX];
  command_t command;
  command.argv = arg_buffer;

  while (1) {
    print_prompt();
    get_command(&command, echo);
    process_command(&command);
  }
}
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 100 /* max length of any command */

// ======================================= Types =================================================//

typedef enum CommandsType {
  COMMAND_A,
  COMMAND_C,
  LABEL,

  COMMAND_COUNT
} CommandsType;

typedef struct Command {
  CommandsType type;
  char dest[3];
  char jmp[3];
} Command;

// ======================================= Functions =============================================//

void remove_spaces(char *line) {
  char copyline[MAXLEN];
  char *copypointer = &copyline[0];

  int pc = 0;
  int pcc = 0;

  while (*line != '\0') {
    if (*line == ' ') {  // remove white spaces from line
      pc++;
      line++;
      continue;
    }
    pcc++;
    *copypointer++ = *line++;
  }

  while (pc--) {
    *line--;
  }
  while (pcc--) {
    *line--;
    *copypointer--;
  }

  while (*copypointer != '\0') {
    if (*copypointer == '/') {  // remove comments from line
      *copypointer = '\0';
    }
    *line++ = *copypointer++;
  }
  *line = '\0';
}

int main(int argc, char *argv[]) {
  char command_line[MAXLEN] = "MD = M + 1 // lalal";  // 1111 1101 1101 1000

  remove_spaces(&command_line[0]);
  printf("%s", command_line);
  return 0;

  Command command;

  if (*command_line == '@') {
    command.type = COMMAND_A;
  } else if (*command_line == '(') {
    command.type = LABEL;
  } else {
    command.type = COMMAND_C;
  }

  if (command.type == COMMAND_C) {
  }
}

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 100 /* max length of any command */

// ======================================= Types =================================================//
typedef enum CommandsType {
  C_ARITHMETIC,
  C_PUSH,
  C_POP,
  C_LABEL,
  C_GOTO,
  C_IF,
  C_FUNCTION,
  C_RETURN,
  C_CALL,

  COMMAND_COUNT
} CommandsType;

typedef struct Command {
  CommandsType type;

  char asm_commands[MAXLEN];
} Command;

// ======================================= Globals ===============================================//

// ======================================= Functions ===============================================

void remove_spaces(char *line) {
  char copyline[MAXLEN];
  char *copypointer = copyline;

  int pc = 0;
  int pcc = 0;

  while (*line != '\0') {
    if (isspace(*line)) {  // remove white spaces from line
      pc++;
      line++;
      continue;
    }

    if ((*line == '/') && (*++line == '/')) {
      *--line = '\0';
    } else {
      *copypointer++ = *line++;
      pcc++;
      pc++;
    }
  }

  *copypointer = '\0';
  pcc++;
  pc++;

  while (--pc) {
    line--;
  }

  while (--pcc) {
    copypointer--;
  }

  while (*copypointer != '\0') {
    *line++ = *copypointer++;
  }
  *line = '\0';
}

void writeAritmetic() {
}

void writePush() {
}

void writePop() {
}

// ======================================= Main Loop==============================================//

// Input fileName.vm , Output fileName.asm
int main(int argc, char *argv[]) {
  // File with code to translate
  char file_name[MAXLEN];
  if (argc != 2) {
    printf("Usage: ./vm filename");
    return 1;
  } else {
    strcpy(file_name, argv[1]);
  }

  FILE *fp;     // from
  FILE *fpasm;  // to

  if ((fpasm = fopen("machine_code.asm", "w")) == NULL) {
    printf("Error: can't open file 'machine_code.asm'\n");
    return 1;
  }

  if ((fp = fopen(file_name, "r")) == NULL) {
    printf("Error: can't open file %s\n", file_name);
    return 1;
  }

  char instruction[MAXLEN];

  while (fgets(instruction, MAXLEN, fp) != NULL) {
    remove_spaces(instruction);
    if (strlen(instruction) == 0) continue;

    Command command = {};

    // Parse instruction
    // Type instruction

    // Translate instruction to asembler code for Hack machine
    if (command.type == C_ARITHMETIC) {
      writeAritmetic();
      // writeAritmetic(command.aritmetic_command);
    } else if (command.type == C_PUSH) {
      writePush();
    } else if (command.type == C_POP) {
      writePop();
    }

    fprintf(fpasm, "%s\n", command.asm_commands);
  }

  fclose(fp);
  fclose(fpasm);
  return 0;
}

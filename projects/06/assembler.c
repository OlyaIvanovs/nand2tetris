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
  char dest[10];  // for C command
  char jmp[10];   // for C command
  char comp[10];  // for C command
  int address;    // for A command

  char binary[16];
} Command;

// ======================================= Globals ===============================================//

char *gJump[] = {"", "JGT", "JEQ", "JGE", "JLT", "JNE", "JLE", "JMP"};
char *gJumpBinary[] = {"000", "001", "010", "011", "100", "101", "110", "111"};

char *gDest[] = {"", "M", "D", "MD", "A", "AM", "AD", "AMD"};
char *gDestBinary[] = {"000", "001", "010", "011", "100", "101", "110", "111"};

char *gComp[] = {"",    "0",   "1",   "-1",  "D",   "A",   "!D",  "A",   "-D",  "-A",
                 "D+1", "A+1", "D-1", "A-1", "D+A", "D-A", "A-D", "D&A", "D|A", "M",
                 "!M",  "-M",  "M+1", "M-1", "D+M", "D-M", "M-D", "D&M", "D|M"};
char *gCompBinary[] = {"0000000", "0101010", "0111111", "0111010", "0001100", "0110000",
                       "0001101", "0110001", "0001111", "0110011", "0011111", "0110111",
                       "0001110", "0110010", "0000010", "0010011", "0000111", "0000000",
                       "0010101", "1110000", "1110001", "1110011", "1110111", "1110010",
                       "1000010", "1010011", "1000111", "1000000", "1010101"};

// ======================================= Functions ===============================================

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
    line--;
  }
  while (pcc--) {
    line--;
    copypointer--;
  }

  while (*copypointer != '\0') {
    if (*copypointer == '/') {  // remove comments from line
      *copypointer = '\0';
    }
    *line++ = *copypointer++;
  }
  *line = '\0';
}

int get_address(char *instruction) {
  char address[MAXLEN];
  char *address_p = &address[0];
  int i = 0;
  while ((*instruction++ = *address_p++) != '\0') {
    i++;
  }
  while (i--) {
    address_p--;
  }
  return atoi(address_p);
}

void parse_c_command(char *instruction, Command *command) {
  char *comp_p = strchr(instruction, '=');
  char *jmp_p = strchr(instruction, ';');
  char *end = strchr(instruction, '\0');

  char *dest_p_end;
  if (comp_p) {
    dest_p_end = comp_p;
  } else if (jmp_p) {
    dest_p_end = jmp_p;
  } else {
    dest_p_end = end;
  }
  strncpy(command->dest, instruction, dest_p_end - instruction);

  if (comp_p) {
    char *comp_p_end;
    if (jmp_p) {
      comp_p_end = jmp_p;
    } else {
      comp_p_end = end;
    }
    strncpy(command->comp, comp_p + 1, comp_p_end - comp_p - 1);
  }

  if (jmp_p) {
    strncpy(command->jmp, jmp_p + 1, end - jmp_p - 1);
  }
}

void to_binary(char number_line) {
}

// ======================================= Main Loop==============================================//

int main(int argc, char *argv[]) {
  char instruction[MAXLEN] = "AD = A + 1; JLE // lalal";  // 111 0 110111 110 110
  // char instruction[MAXLEN] = "@32 // lalal";  // 1111 1101 1101 1000

  remove_spaces(&instruction[0]);

  Command command = {};

  // Parse instruction
  if (*instruction == '@') {
    command.type = COMMAND_A;
  } else if (*instruction == '(') {
    command.type = LABEL;
  } else {
    command.type = COMMAND_C;
  }

  if (command.type == COMMAND_C) {
    parse_c_command(&instruction[0], &command);
  }

  if (command.type == COMMAND_A) {
    command.address = get_address(&instruction[1]);
  }

  // Translate instruction to binary code
  if (command.type == COMMAND_C) {
    strcpy(command.binary, "111");

    for (int i = 0; i < 30; i++) {
      if (strcmp(command.comp, gComp[i]) == 0) {
        strcat(command.binary, gCompBinary[i]);
      }
    }

    for (int i = 0; i < 8; i++) {
      if (strcmp(command.dest, gDest[i]) == 0) {
        strcat(command.binary, gDestBinary[i]);
      }
    }

    for (int i = 0; i < 8; i++) {
      if (strcmp(command.jmp, gJump[i]) == 0) {
        strcat(command.binary, gJumpBinary[i]);
      }
    }
  }

  if (command.type == COMMAND_A) {
    strcpy(command.binary, "0");
    to_binary(&command_binary[1]);
  }

  printf("%s", command.binary);
  return 0;
}

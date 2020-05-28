#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 100 /* max length of any command */
#define HASHSIZE 101

// ======================================= Types =================================================//
typedef enum CommandsType {
  COMMAND_A,
  COMMAND_C,
  COMMAND_L,

  COMMAND_COUNT
} CommandsType;

typedef struct Command {
  CommandsType type;
  char dest[10];  // for C command
  char jmp[10];   // for C command
  char comp[10];  // for C command
  int address;    // for A command

  char binary[17];
} Command;

typedef struct Entry {
  struct Entry *next;  // next entry in chain
  char *symbol;
  int address;
} Entry;

// ======================================= Globals ===============================================//

char *gJump[] = {"", "JGT", "JEQ", "JGE", "JLT", "JNE", "JLE", "JMP"};
char *gJumpBinary[] = {"000", "001", "010", "011", "100", "101", "110", "111"};

char *gDest[] = {"", "0", "M", "D", "MD", "A", "AM", "AD", "AMD"};
char *gDestBinary[] = {"000", "000", "001", "010", "011", "100", "101", "110", "111"};

char *gComp[] = {"",    "0",   "1",   "-1",  "D",   "A",   "!D",  "!A",  "-D",  "-A",
                 "D+1", "A+1", "D-1", "A-1", "D+A", "D-A", "A-D", "D&A", "D|A", "M",
                 "!M",  "-M",  "M+1", "M-1", "D+M", "D-M", "M-D", "D&M", "D|M"};
char *gCompBinary[] = {"0000000", "0101010", "0111111", "0111010", "0001100", "0110000",
                       "0001101", "0110001", "0001111", "0110011", "0011111", "0110111",
                       "0001110", "0110010", "0000010", "0010011", "0000111", "0000000",
                       "0010101", "1110000", "1110001", "1110011", "1110111", "1110010",
                       "1000010", "1010011", "1000111", "1000000", "1010101"};

static Entry *hashtab[HASHSIZE];  // TODO: rewrite!!!

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

void parse_c_command(char *instruction, Command *command) {
  char *equal_p = strchr(instruction, '=');
  char *jmp_p = strchr(instruction, ';');
  char *end = strchr(instruction, '\0');

  char *dest_p;
  char *comp_p;
  char *dest_p_end;
  char *comp_p_end;

  if (equal_p) {
    dest_p = instruction;
    dest_p_end = equal_p;
    comp_p = equal_p + 1;
    if (jmp_p) {
      comp_p_end = jmp_p;
    } else {
      comp_p_end = end;
    }
    strncpy(command->dest, dest_p, dest_p_end - dest_p);
    strncpy(command->comp, comp_p, comp_p_end - comp_p);
  }

  if (!equal_p) {
    comp_p = instruction;
    if (jmp_p) {
      comp_p_end = jmp_p;
    } else {
      comp_p_end = end;
    }
    strncpy(command->comp, comp_p, comp_p_end - comp_p);
  }

  if (jmp_p) {
    strncpy(command->jmp, jmp_p + 1, end - jmp_p - 1);
  }
}

void to_binary(int value, char *command_line, int len) {
  char line[MAXLEN];
  int i = 0;
  while (value != 0) {
    line[i++] = (value % 2 + '0');
    value = value / 2;
  }
  int empty_bites = len - i;
  while (empty_bites--) {
    *command_line++ = '0';
  }
  while (i--) {
    *command_line++ = line[i];
  }
}

// Symbol table
// Form hash value for symbol string
unsigned hash(char *s) {
  unsigned hashval;

  for (hashval = 0; *s != '\0'; s++) hashval = *s + 31 * hashval;
  return hashval % HASHSIZE;
}

// Look for symbol in hashtab
Entry *lookup(char *s) {
  Entry *entry;

  for (entry = hashtab[hash(s)]; entry != NULL; entry = entry->next) {
    if (strcmp(s, entry->symbol) == 0) return entry;
  }
  return NULL;
}
// Put symbol in hashtab
Entry *add_entry(char *symbol, int address) {
  Entry *entry;
  unsigned hashval = hash(symbol);

  if ((entry = lookup(symbol)) == NULL) {
    entry = malloc(sizeof(*entry));
    if (entry == NULL || (entry->symbol = strdup(symbol)) == NULL) {
      return NULL;
    }
    entry->address = address;
    entry->next = hashtab[hashval];
    hashtab[hashval] = entry;
  }
  // if already exists?
  return entry;
}

// ======================================= Main Loop==============================================//

int main(int argc, char *argv[]) {
  //  Symbol table as global variable TODO!
  // Add predefined symbols in the symbol table
  add_entry("SP", 0);
  add_entry("LCL", 1);
  add_entry("ARG", 2);
  add_entry("THIS", 3);
  add_entry("THAT", 4);
  add_entry("R0", 0);
  add_entry("R1", 1);
  add_entry("R2", 2);
  add_entry("R3", 3);
  add_entry("R4", 4);
  add_entry("R5", 5);
  add_entry("R6", 6);
  add_entry("R7", 7);
  add_entry("R8", 8);
  add_entry("R9", 9);
  add_entry("R10", 10);
  add_entry("R11", 11);
  add_entry("R12", 12);
  add_entry("R13", 13);
  add_entry("R14", 14);
  add_entry("R15", 15);
  add_entry("SCREEN", 16384);
  add_entry("KBD", 24576);

  // File with code to translate
  char file_name[MAXLEN];
  if (argc != 2) {
    printf("Usage: ./assembler filename");
    return 1;
  } else {
    strcpy(file_name, argv[1]);
  }

  FILE *fp;
  FILE *fpbin;

  if ((fpbin = fopen("binary.hack", "w")) == NULL) {
    printf("Error: can't open file 'binary.hack'\n");
    return 1;
  }

  if ((fp = fopen(file_name, "r")) == NULL) {
    printf("Error: can't open file %s\n", file_name);
    return 1;
  }

  char instruction[MAXLEN];
  char label[MAXLEN];
  int rom_address = 0;

  // First pass
  while (fgets(instruction, MAXLEN, fp) != NULL) {
    remove_spaces(instruction);
    if (strlen(instruction) == 0) continue;
    if (*instruction == '(') {
      char *label_end;
      if ((label_end = strrchr(instruction, ')'))) {
        strncpy(label, &instruction[1], label_end - &instruction[1]);
        label[label_end - &instruction[1]] = '\0';
        add_entry(label, rom_address);
      }
    } else {
      rom_address++;
    }
  }
  fclose(fp);

  FILE *fp2 = fopen(file_name, "r");
  int ram_address = 16;
  while (fgets(instruction, MAXLEN, fp2) != NULL) {
    remove_spaces(instruction);
    if (strlen(instruction) == 0) continue;

    Command command = {};
    Entry *symbol;

    // Parse instruction
    if (*instruction == '@') {
      command.type = COMMAND_A;
    } else if (*instruction == '(') {
      command.type = COMMAND_L;
      continue;
    } else {
      command.type = COMMAND_C;
    }

    if (command.type == COMMAND_C) {
      parse_c_command(&instruction[0], &command);
    }

    if (command.type == COMMAND_A) {
      if (isdigit(instruction[1])) {
        command.address = atoi(&instruction[1]);
      } else if (isalpha(instruction[1])) {
        symbol = lookup(&instruction[1]);  // check if symbol in table
        if (symbol) {
          command.address = symbol->address;
        } else {
          command.address = ram_address;
          add_entry(&instruction[1], ram_address++);
        }
      } else {
        // Todo: Error!!! If nothing is founded
      }
    }

    // Translate instruction to binary code
    if (command.type == COMMAND_C) {
      strcpy(command.binary, "111");

      for (int i = 0; i < 29; i++) {
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
      to_binary(command.address, &command.binary[1], 15);  // TODO address size
    }

    fprintf(fpbin, "%s\n", command.binary);
  }

  fclose(fp2);
  fclose(fpbin);
  return 0;
}

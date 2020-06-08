#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 100 /* max length of any command */

// ======================================= Types =================================================//
typedef enum CommandsType {
  C_ARITHMETIC,
  C_PUSH_POP,
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
  char vm_command[MAXLEN];

  char asm_command[MAXLEN];
} Command;

typedef enum KeywordType {
  KEYWORD_POP,
  KEYWORD_PUSH,

  KEYWORD_ADD,
  KEYWORD_SUB,

  KEYWORD_LOCAL,
  KEYWORD_ARGUMENT,
  KEYWORD_THIS,
  KEYWORD_THAT,
  KEYWORD_CONSTANT,
  KEYWORD_STATIC,
  KEYWORD_POINTER,
  KEYWORD_TEMP,

  KEYWORD_COUNT
} KeywordType;

typedef enum TokenPurpose {
  MEMORY_ACCESS,
  SEGMENT,
  INDEX,
  MATH,

  TOKEN_COUNT
} TokenPurpose;

typedef enum MemorySegments {
  LOCAL,
  ARGUMENT,
  THIS,
  THAT,
  CONSTANT,
  STATIC,
  POINTER,
  TEMP
} MemorySegments;

typedef enum MathCommands {
  ADD,
  SUB,

} MathCommands;

typedef struct Keyword {
  char *string;

  KeywordType type;
  TokenPurpose purpose;
  TokenPurpose next_token_purpose;
  MemorySegments segment;
  MathCommands command;
} Keyword;

typedef struct Token {
  TokenPurpose purpose;
  TokenPurpose next_token_purpose;
  union {
    Keyword keyword;
    int number;
    char text[MAXLEN];
  } token_type;
} Token;

// ======================================= Globals ===============================================//

// ======================================= Functions ===============================================

void remove_spaces(char *line) {
  char new_line[MAXLEN];
  int pline = 0;
  int k = 0;

  while (isspace(*line++)) {
    pline++;
    continue;
  }
  line--;
  while (*line != '\0') {
    if (isspace(*line) && isspace(*(line + 1))) {
      line++;
      pline++;
      continue;
    }
    new_line[k++] = *line++;
    pline++;
  }

  new_line[k] = '\0';

  while (pline--) {
    line--;
  }

  strcpy(line, new_line);
}

// Return size of array with tokens, 0 if the order of tokens is wrong
int tokenize(char *line, Token *tokens) {
  Keyword keywords[12] = {
      {"pop", KEYWORD_POP, MEMORY_ACCESS, SEGMENT, 0, 0},
      {"push", KEYWORD_PUSH, MEMORY_ACCESS, SEGMENT, 0, 0},

      {"local", KEYWORD_LOCAL, SEGMENT, INDEX, LOCAL, 0},
      {"argument", KEYWORD_ARGUMENT, SEGMENT, INDEX, ARGUMENT, 0},
      {"this", KEYWORD_THIS, SEGMENT, INDEX, THIS, 0},
      {"that", KEYWORD_THAT, SEGMENT, INDEX, THAT, 0},
      {"constant", KEYWORD_CONSTANT, SEGMENT, INDEX, CONSTANT, 0},
      {"static", KEYWORD_STATIC, SEGMENT, INDEX, STATIC, 0},
      {"pointer", KEYWORD_POINTER, SEGMENT, INDEX, POINTER, 0},
      {"temp", KEYWORD_TEMP, SEGMENT, INDEX, TEMP, 0},

      {"add", KEYWORD_ADD, MATH, 0, 0, ADD},
      {"sub", KEYWORD_SUB, MATH, 0, 0, SUB},
  };

  char word[MAXLEN];
  int k = 0;
  int token_num = 0;
  while (*line != '\0') {
    word[k++] = *line++;
    if (isspace(*line)) {
      word[k] = '\0';
      for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i].string) == 0) {
          tokens->purpose = keywords[i].purpose;
          tokens->next_token_purpose = keywords[i].next_token_purpose;
          tokens->token_type.keyword = keywords[i];
          tokens++;
          token_num++;
        }
      }

      while (k > 0) {
        if (!isdigit(word[--k])) {
          k = 0;
          continue;
        }
        if (k == 0) {
          tokens->purpose = INDEX;
          tokens->token_type.number = atoi(word);
          token_num++;
        }
      }

      // remove spaces
      while (isspace(*line++))
        ;
      line--;
    }
  }
  return token_num;
}

void parsing(Token tokens[10], int token_num, Command *command) {
  // Check if the order of tokens is correct
  for (int i = token_num; i > 0; i--) {
    if ((i > 1) && (tokens[i].purpose != tokens[i - 1].next_token_purpose)) {
      printf("!ERROR! Next Token should be %d", tokens[i - 1].next_token_purpose);
      return;
    }
  }

  if (tokens[0].purpose == MEMORY_ACCESS) {
    command->type = C_PUSH_POP;
  }

  if (tokens[0].purpose == MATH) {
    command->type = C_ARITHMETIC;
  }
}

void writePushPop(Token command_token, Token segment, Token index, Command command) {
  if (segment.token_type.keyword.segment == CONSTANT) {
    printf("\n//%s@%d\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", command.vm_command,
           index.token_type.number);
    return;
  }

  if (segment.token_type.keyword.segment == STATIC) {
    printf("\n//%s@%d\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", command.vm_command,
           index.token_type.number);
    return;
  }

  char *memory_segments[8] = {"LCL", "ARG", "THIS", "THAT", "CONSTANT", "STATIC", "R3", "R5"};
  if (command_token.token_type.keyword.type == KEYWORD_PUSH) {
    printf("\n//%s@%d\nD=A\n@%s\nA=M+D\nD=M\n@SP\nM=M+1\nA=M\nM=D\n", command.vm_command,
           index.token_type.number, memory_segments[segment.token_type.keyword.segment]);
  } else if (command_token.token_type.keyword.type == KEYWORD_POP) {
    printf("\n//%s@%d\nD=A\n@%s\nA=M+D\nD=A\n@R13\nM=D\n@SP\nM=M-1\nA=M\nD=M\n@R13\nA=M\nM=D\n",
           command.vm_command, index.token_type.number,
           memory_segments[segment.token_type.keyword.segment]);
  }
}

void writeAritmetic(Token token_command, Command command) {
  // ADD, SUB
  char *diff[2] = {"M=M+D", "M=M-D"};
  printf("\n//%s@SP\nAM=M-1\nD=M\nA=A-1\n%s\n", command.vm_command,
         diff[token_command.token_type.keyword.command]);
}

// ======================================= Main Loop=============================================//

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
    strcpy(command.vm_command, instruction);

    // Tokenize instruction
    Token tokens[10];
    int tokens_num = tokenize(instruction, &tokens[0]);
    if (!tokens_num) return 0;

    // Parse instruction
    parsing(tokens, tokens_num, &command);

    // Translate instruction to asembler code for Hack machine
    if (command.type == C_PUSH_POP) {
      writePushPop(tokens[0], tokens[1], tokens[2], command);
    }

    if (command.type == C_ARITHMETIC) {
      writeAritmetic(tokens[0], command);
    }

    // fprintf(fpasm, "%s\n", command.asm_commands);
  }

  fclose(fp);
  fclose(fpasm);
  return 0;
}

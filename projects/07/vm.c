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

  char asm_commands[MAXLEN];
} Command;

typedef enum KeywordType {
  KEYWORD_POP,
  KEYWORD_PUSH,

  KEYWORD_ADD,

  KEYWORD_LOCAL,
  KEYWORD_ARGUMENT,
  KEYWORD_THIS,
  KEYWORD_THAT,
  KEYWORD_CONSTANT,

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

typedef struct Keyword {
  char *string;

  KeywordType type;
  TokenPurpose purpose;
  TokenPurpose next_token_purpose;
  MemorySegments segment;
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

// Return size of array with tokens, 0 if the order of tokens is wrong
int tokenize(char *line, Token *tokens) {
  Keyword keywords[8] = {
      {"pop", KEYWORD_POP, MEMORY_ACCESS, SEGMENT, 0},
      {"push", KEYWORD_PUSH, MEMORY_ACCESS, SEGMENT, 0},
      {"add", KEYWORD_ADD, MATH, 0, 0},
      {"local", KEYWORD_LOCAL, SEGMENT, INDEX, LOCAL},
      {"argument", KEYWORD_ARGUMENT, SEGMENT, INDEX, ARGUMENT},
      {"this", KEYWORD_THIS, SEGMENT, INDEX, THIS},
      {"that", KEYWORD_THAT, SEGMENT, INDEX, THAT},
      {"constant", KEYWORD_CONSTANT, SEGMENT, INDEX, CONSTANT},
  };

  while (isspace(*line++)) continue;
  line--;

  char word[MAXLEN];
  int k = 0;
  int token_num = 0;
  printf("line %s\n", line);
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

  printf("token_num %d", token_num);
  // Check if the order of tokens is correct
  for (int i = token_num; i > 0; i--) {
    Token *token = tokens--;
    Token *prev_token = tokens;
    printf("token->purpose %d ", token->purpose);
    if ((i > 1) && (token->purpose != prev_token->next_token_purpose)) {
      printf("!ERROR! Next Token should be %d", prev_token->next_token_purpose);
      return 0;
    }
  }

  return token_num;
}

void writeAritmetic() {
}

void writePushPop(Token command, Token segment, Token index) {
  if (segment.token_type.keyword.segment == CONSTANT) {
    printf("@%d\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", index.token_type.number);
    return;
  }

  char *memory_segments[4] = {"LCL", "ARG", "THIS", "THAT"};
  if (command.token_type.keyword.type == KEYWORD_PUSH) {
    printf("@%d\nD=A\n@%s\nA=M+D\nD=M\n@SP\nM=M+1\nA=M\nM=D\n", index.token_type.number,
           memory_segments[segment.token_type.keyword.segment]);
  } else if (command.token_type.keyword.type == KEYWORD_POP) {
    printf("@%d\nD=A\n@%s\nA=M+D\nD=A\n@R13\nM=D\n@SP\nM=M-1\nA=M\nD=M\n@R13\nA=M\nM=D\n",
           index.token_type.number, memory_segments[segment.token_type.keyword.segment]);
  }
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
    if (strlen(instruction) == 0) continue;

    Command command = {};

    // Tokenize instruction
    Token tokens[10];
    int tokens_num = tokenize(instruction, &tokens[0]);
    if (!tokens_num) return 0;

    // Parse instruction
    for (int i = 0; i < tokens_num; i++) {
      printf("%d ", tokens[i].purpose);
    }

    if (tokens[0].purpose == MEMORY_ACCESS) {
      command.type = C_PUSH_POP;
    }

    // Type instruction

    // Translate instruction to asembler code for Hack machine
    if (command.type == C_PUSH_POP) {
      writePushPop(tokens[0], tokens[1], tokens[2]);
    }

    // if (command.type == C_ARITHMETIC) {
    //   writeAritmetic();
    //   // writeAritmetic(command.aritmetic_command);
    // } else if (command.type == C_PUSH) {
    //   writePush();
    // } else if (command.type == C_POP) {
    //   writePop();
    // }

    // fprintf(fpasm, "%s\n", command.asm_commands);
  }

  fclose(fp);
  fclose(fpasm);
  return 0;
}

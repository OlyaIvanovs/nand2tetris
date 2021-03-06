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

typedef enum KeywordType {
  KEYWORD_POP,
  KEYWORD_PUSH,

  KEYWORD_ADD,
  KEYWORD_SUB,
  KEYWORD_AND,
  KEYWORD_OR,
  KEYWORD_NEG,
  KEYWORD_NOT,
  KEYWORD_EQ,
  KEYWORD_GT,
  KEYWORD_LT,

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

typedef enum MemorySegment {
  LOCAL,
  ARGUMENT,
  THIS,
  THAT,
  CONSTANT,
  STATIC,
  POINTER,
  TEMP
} MemorySegment;

typedef enum MathCommand {
  ADD,
  SUB,
  AND,
  OR,
  NEG,  // unary
  NOT,
  EQ,  // CMP
  GT,
  LT
} MathCommand;

typedef enum ParseResultCode {
  PARSE_ERROR,
  PARSE_SUCCESS,

} ParseResultCode;

typedef struct Command {
  CommandsType type;
  int num_line;  // order nummer of instruction in file

  // for push, pop
  MemorySegment segment;
  int index;

  // for math
  MathCommand math;

  char vm_command[MAXLEN];   // for comments
  char static_name[MAXLEN];  // for static variables names
  char asm_commands[200];
} Command;

typedef struct Keyword {
  char *string;

  KeywordType type;
  TokenPurpose purpose;
  TokenPurpose next_token_purpose;
  MemorySegment segment;
  MathCommand command;
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

typedef struct ParseResult {
  ParseResultCode code;
  char message[MAXLEN];
} ParseResult;

// ======================================= Globals ===============================================//

// ======================================= Functions ===============================================

void remove_spaces(char *line) {
  char new_line[MAXLEN];
  int rc = 0;  // read cursor
  int wc = 0;  // write cursor

  // Skip whitespace at the beginning
  while (isspace(line[rc])) rc++;

  while (line[rc] != '\0') {
    if (isspace(line[rc]) && isspace(line[rc + 1])) {
      rc++;
    } else if (line[rc] == '/' && line[rc + 1] == '/') {  // ignore comments
      new_line[wc++] = '\n';
      line[rc] = '\0';
    } else {
      new_line[wc++] = line[rc++];
    }
  }

  new_line[wc] = '\0';
  strncpy(line, new_line, MAXLEN);
}

// Return size of array with tokens, 0 if the order of tokens is wrong
int tokenize(char *line, Token *tokens) {
  Keyword keywords[19] = {
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
      {"and", KEYWORD_AND, MATH, 0, 0, AND},
      {"or", KEYWORD_OR, MATH, 0, 0, OR},
      {"neg", KEYWORD_NEG, MATH, 0, 0, NEG},
      {"not", KEYWORD_NOT, MATH, 0, 0, NOT},
      {"eq", KEYWORD_EQ, MATH, 0, 0, EQ},
      {"gt", KEYWORD_GT, MATH, 0, 0, GT},
      {"lt", KEYWORD_LT, MATH, 0, 0, LT},
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
          tokens->next_token_purpose = 0;
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

ParseResult parse(Token tokens[10], int token_num, Command *command) {
  ParseResult result = {};
  char *token_purposes[4] = {"MEMORY_ACCESS", "SEGMENT", "INDEX", "MATH"};

  // The number of tokens should be < 3
  if (token_num > 3) {
    sprintf(result.message, "The instruction is too long.");
    result.code = PARSE_ERROR;
    return result;
  }

  // Check if number of tokens is correct
  if (tokens[0].purpose == MEMORY_ACCESS && token_num < 3) {
    sprintf(result.message, "There should be 2 arguments(segment and index)");
    result.code = PARSE_ERROR;
    return result;
  }

  // Check if the order of tokens is correct
  for (int i = token_num - 1; i > 0; i--) {
    if ((i > 1) && (tokens[i].purpose != tokens[i - 1].next_token_purpose)) {
      sprintf(result.message, "Next Token should be %s",
              token_purposes[tokens[i - 1].next_token_purpose]);
      result.code = PARSE_ERROR;
      return result;
    }
  }

  if (tokens[0].purpose == MEMORY_ACCESS) {
    if (tokens[0].token_type.keyword.type == KEYWORD_POP) {
      command->type = C_POP;
    } else if (tokens[0].token_type.keyword.type == KEYWORD_PUSH) {
      command->type = C_PUSH;
    }

    command->segment = tokens[1].token_type.keyword.segment;
    command->index = tokens[2].token_type.number;

    if (command->index < 0) {
      sprintf(result.message, "Index should be > 0");
      result.code = PARSE_ERROR;
      return result;
    }

    if (command->segment == TEMP && (command->index > 7)) {
      sprintf(result.message, "Index for temp segment should be <= 7");
      result.code = PARSE_ERROR;
      return result;
    }

    if (command->segment == POINTER && (command->index > 1)) {
      sprintf(result.message, "Index for pointer segment should be 0 or 1");
      result.code = PARSE_ERROR;
      return result;
    }
  }

  if (tokens[0].purpose == MATH) {
    if (token_num > 1) {
      sprintf(result.message, "The instruction too long.");
      result.code = PARSE_ERROR;
    }
    command->type = C_ARITHMETIC;
    command->math = tokens[0].token_type.keyword.command;
  }

  result.code = PARSE_SUCCESS;

  return result;
}

void writePush(Command *command) {
  if (command->segment == CONSTANT) {
    sprintf(command->asm_commands, "\n//%s@%d\nD=A\n@SP\nM=M+1\nA=M-1\nM=D\n", command->vm_command,
            command->index);
    return;
  }

  if (command->segment == STATIC) {
    sprintf(command->asm_commands, "\n//%s@%s.%d\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n",
            command->vm_command, command->static_name, command->index);
    return;
  }

  if (command->segment == TEMP) {
    sprintf(command->asm_commands, "\n//%s@R%d\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", command->vm_command,
            (5 + command->index));
    return;
  }

  if (command->segment == POINTER) {
    char *pointers[2] = {"THIS", "THAT"};
    sprintf(command->asm_commands, "\n//%s@%s\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", command->vm_command,
            pointers[command->index]);
    return;
  }

  char *memory_segments[4] = {"LCL", "ARG", "THIS", "THAT"};
  sprintf(command->asm_commands, "\n//%s@%d\nD=A\n@%s\nA=M+D\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n",
          command->vm_command, command->index, memory_segments[command->segment]);
}

void writePop(Command *command) {
  if (command->segment == STATIC) {
    sprintf(command->asm_commands, "\n//%s@SP\nM=M-1\nA=M\nD=M\n@%s.%d\nM=D\n", command->vm_command,
            command->static_name, command->index);
    return;
  }

  if (command->segment == TEMP) {
    sprintf(command->asm_commands, "\n//%s@SP\nM=M-1\nA=M\nD=M\n@R%d\nM=D\n", command->vm_command,
            (5 + command->index));
    return;
  }

  if (command->segment == POINTER) {
    char *pointers[2] = {"THIS", "THAT"};
    sprintf(command->asm_commands, "\n//%s@SP\nAM=M-1\nD=M\n@%s\nM=D\n", command->vm_command,
            pointers[command->index]);
    return;
  }

  char *memory_segments[8] = {"LCL", "ARG", "THIS", "THAT"};
  sprintf(command->asm_commands,
          "\n//%s@%d\nD=A\n@%s\nA=M+D\nD=A\n@R13\nM=D\n@SP\nM=M-1\nA=M\nD=M\n@R13\nA=M\nM=D\n",
          command->vm_command, command->index, memory_segments[command->segment]);
}

void writeArithmetic(Command *command) {
  // ADD, SUB, AND, OR, NEG, NOT
  char *diff[9] = {"D+M", "M-D", "D&M", "D|M", "-M", "!M", "JEQ", "JGT", "JLT"};

  if (command->math <= 3) {  // ADD, SUB, AND, OR
    sprintf(command->asm_commands, "\n//%s@SP\nAM=M-1\nD=M\nA=A-1\nM=%s\n", command->vm_command,
            diff[command->math]);
  } else if (command->math > 5) {  // EQ, GT, LT
    sprintf(command->asm_commands,
            "\n//"
            "%s@SP\nAM=M-1\nD=M\nA=A-1\nD=M-D\n@IF_%s_%d\nD;%s\n@SP\nA=M-1\nM=0\n@CONTINUE_%d\n0;"
            "JMP\n(IF_%s_%d)\n@SP\nA=M-1\nM=-1\n(CONTINUE_%d)",
            command->vm_command, diff[command->math], command->num_line, diff[command->math],
            command->num_line, diff[command->math], command->num_line, command->num_line);
  } else if (command->math == NEG || command->math == NOT) {  // Unary
    sprintf(command->asm_commands, "\n//%s@SP\nA=M-1\nD=M\nM=%s\n", command->vm_command,
            diff[command->math]);
  }
}

// ======================================= Main ====================================================

// Input fileName.vm , Output fileName.asm
int main(int argc, char *argv[]) {
  // File with code to translate
  char file_name_to_read[MAXLEN];
  char file_name_to_write[MAXLEN];
  if (argc != 2) {
    printf("Usage: ./vm filename");
    return 1;
  } else {
    strcpy(file_name_to_read, argv[1]);
  }

  FILE *fp;     // from
  FILE *fpasm;  // to

  if ((fp = fopen(file_name_to_read, "r")) == NULL) {
    printf("Error: can't open file %s\n", file_name_to_read);
    return 1;
  }

  // The name of output file is the same as the name of file to read
  // The beginning for static variables names
  char static_filename[MAXLEN];
  int size_filename = strrchr(file_name_to_read, '.') - file_name_to_read;
  strncpy(file_name_to_write, file_name_to_read, size_filename + 1);
  file_name_to_write[size_filename + 1] = '\0';

  int k = strrchr(file_name_to_write, '/') - file_name_to_write + 1;
  for (int i = k, j = 0; file_name_to_write[i] != '.'; i++, j++) {
    static_filename[j] = file_name_to_write[i];
  }
  strncat(file_name_to_write, "asm", 3);

  if ((fpasm = fopen(file_name_to_write, "w")) == NULL) {
    printf("Error: can't open file %s\n", file_name_to_write);
    return 1;
  }

  char instruction[MAXLEN];
  int num_line = 0;

  while (fgets(instruction, MAXLEN, fp) != NULL) {
    remove_spaces(instruction);
    if (strlen(instruction) <= 1) continue;

    Command command = {};
    command.num_line = num_line++;
    strcpy(command.vm_command, instruction);
    strcpy(command.static_name, static_filename);

    // Tokenize instruction
    Token tokens[10];
    int tokens_num = tokenize(instruction, &tokens[0]);
    if (!tokens_num) return 0;

    // Parse instruction
    ParseResult result = parse(tokens, tokens_num, &command);
    if (result.code == PARSE_ERROR) {
      printf("\nError in line %d: %s%s.", command.num_line, instruction, result.message);
      return 1;
    }

    // Translate instruction to assembler code for Hack machine
    if (command.type == C_PUSH) {
      writePush(&command);
    } else if (command.type == C_POP) {
      writePop(&command);
    } else if (command.type == C_ARITHMETIC) {
      writeArithmetic(&command);
    }

    fprintf(fpasm, "%s\n", command.asm_commands);
  }

  fclose(fp);
  fclose(fpasm);
  return 0;
}

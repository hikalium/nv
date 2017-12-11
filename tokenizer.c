#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *buf;      // should be free
  char **tokens;  // should be free, NULL terminated.
} TokenList;

typedef char CharClass;

const int charClassesSize = 1 << (sizeof(char) * 8);
const CharClass *CreateCharClasses() {
  CharClass *charClasses = malloc(sizeof(CharClass) * charClassesSize);
  for (int i = 0; i < charClassesSize; i++) {
    charClasses[i] = 0;
  }
  for (int i = 'A'; i <= 'Z'; i++) {
    charClasses[i] = 1;
  }
  for (int i = 'a'; i <= 'z'; i++) {
    charClasses[i] = 1;
  }
  for (int i = '0'; i <= '9'; i++) {
    charClasses[i] = 2;
  }
  return charClasses;
}

TokenList *CreateTokenListFromString(const char *s, const char *charClasses) {
  TokenList *list = malloc(sizeof(TokenList));
  int sLen = strlen(s);
  int maxTokens = sLen;
  list->buf = malloc(sLen + maxTokens);
  list->tokens = malloc(sizeof(char *) * maxTokens);
  //
  int sp = 0;
  int tp = 0;
  for (int i = 0; i < sLen;) {
    char c = s[i++];
    CharClass cClass = charClasses[(int)c];
    list->tokens[tp++] = &list->buf[sp];
    list->buf[sp++] = c;
    if (cClass != 0) {
      while (charClasses[(int)s[i]] == cClass) {
        list->buf[sp++] = s[i++];
      }
    } else if (c == '"') {
      while (s[i]) {
        if (!s[i]) break;
        if (s[i] == '\\') {
          list->buf[sp++] = s[i++];
          list->buf[sp++] = s[i++];
          continue;
        }
        list->buf[sp++] = s[i++];
        if (s[i - 1] == '"') break;
      }
    }
    list->buf[sp++] = 0;
  }
  list->tokens[tp++] = NULL;
  //
  return list;
}

const int bufSize = 64 * 1024;
const int funcDefSize = 1024;
int main(int argc, char *argv[]) {
  char *buf;
  char *funcDef;
  if (argc < 2) {
    printf("%s <filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  buf = malloc(bufSize);
  funcDef = malloc(funcDefSize);
  FILE *fp = fopen(argv[1], "rb");
  if (!fp) {
    printf("%s: File not found.\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  fread(buf, 1, bufSize, fp);
  const CharClass *charClasses = CreateCharClasses();
  TokenList *list = CreateTokenListFromString(buf, charClasses);
  int nestCount = 0;
  int ignoreLine = 0;
  int tokensInLine = 0;
  int inBlockComment = 0;
  int funcDefUsed = 0;
  int spaceCount = 0;
  int parenCount = 0;
  for (int i = 0; list->tokens[i]; i++) {
    char c = list->tokens[i][0];
    if (inBlockComment) {
      if (list->tokens[i][0] == '*' && list->tokens[i + 1][0] == '/') {
        inBlockComment = 0;
      }
      continue;
    }
    if (c == '\n') {
      ignoreLine = 0;
      tokensInLine = 0;
    } else if (c == '/') {
      if (list->tokens[i + 1][0] == '*') {
        inBlockComment = 1;
      } else {
        ignoreLine = 1;
      }
    } else if (c == '#') {
      ignoreLine = 1;
    } else if (c == ';') {
      funcDefUsed = 0;
    } else if (c == '{') {
      funcDefUsed = 0;
      nestCount++;
    } else if (c == '}') {
      nestCount--;
      funcDefUsed = 0;
    } else if (nestCount == 0 && !ignoreLine) {
      if (c != ' ' || spaceCount == 0) {
        if (c == ' ') {
          spaceCount++;
        } else {
          spaceCount = 0;
        }
        if (funcDefUsed == 0 && c == ' ') continue;
        if (c == '(') parenCount++;
        strcpy(&funcDef[funcDefUsed], list->tokens[i]);
        funcDefUsed += strlen(list->tokens[i]);
        tokensInLine++;
        if (c == ')') {
          parenCount--;
          if (parenCount == 0) {
            if (nestCount == 0 && funcDefUsed) {
              printf("%s;\n", funcDef);
            }
            funcDefUsed = 0;
          }
        }
      }
    }
  }
  return 0;
}

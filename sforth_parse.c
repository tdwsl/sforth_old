#include "sforth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void forth_doString(Forth *fth, const char *text) {
  int max = 100;
  char *s = (char*)malloc(max);
  const int buf = 40;
  int len = 0;

  char quote = 0;
  int comment = 0;

  for(const char *c = text; ; c++) {
    if(*c == '\n' || *c == 0) {
      if(quote || len) {
        s[len] = 0;
        if(strcmp(s, "\\") != 0)
          forth_compileToken(fth, s);
      }

      len = 0;
      quote = 0;
      comment = 0;

      if(*c == 0)
        break;
    }

    else if(comment)
      continue;

    else if(quote) {
      if(*c == quote) {
        s[len] = 0;
        quote = 0;
        forth_compileToken(fth, s);
        len = 0;
      }
      else
        s[len++] = *c;
    }

    else if(*c == ' ' || *c == '\t') {
      if(len) {
        s[len] = 0;
        len = 0;

        if(strcmp(s, ".\"") == 0
            || strcmp(s, ".(") == 0
            || strcmp(s, ".'") == 0) {
          quote = s[1];
          s[1] = '"';
          if(quote == '(')
            quote = ')';
        }

        if(strcmp(s, "\\") == 0)
          comment = 1;
        else
          forth_compileToken(fth, s);
      }
    }

    else
      s[len++] = *c;

    if(len > max-buf) {
      max += buf;
      s = (char*)realloc(s, max);
    }
  }

  free(s);
}

void forth_doFile(Forth *fth, const char *filename) {
  FILE *fp = fopen(filename, "r");
  if(!fp) {
    forth_printf(fth, FORTH_FILENOTFOUND_ERR, filename);
    return;
  }

  int max = 100;
  char *s = (char*)malloc(max);
  int buf = 40;
  int len = 0;

  while(!feof(fp)) {
    char c = fgetc(fp);
    s[len++] = c;
    if(len > max-buf) {
      max += buf;
      s = (char*)realloc(s, max);
    }
  }
  fclose(fp);

  s[len-1] = 0;
  forth_doString(fth, s);

  free(s);
}

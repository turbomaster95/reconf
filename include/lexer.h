#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

char *trim(char *s);
void unquote(char *s);
void get_basename(const char *path, char *out);

#endif // LEXER_H

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <parser.h>

int generate_configure(const ProjectConfig *cfg, const char *out_path);

#endif /* CODEGEN_H */

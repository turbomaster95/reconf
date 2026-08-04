#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define MAX_LINE 1024
#define MAX_ARG 1024
#define MAX_ITEMS 256

typedef enum {
    TYPE_VAR,
    TYPE_HEADER,
    TYPE_CODE,
    TYPE_CUSTOM_LIB,
    TYPE_TARGET_BIN,
    TYPE_TARGET_SO,
    TYPE_TARGET_A,
    TYPE_SOURCES
} ItemType;

typedef struct {
    ItemType type;
    char name[128];
    char arg1[MAX_ARG];
    char target_sources[MAX_ARG];
} ConfigItem;

typedef struct {
    char pkg_name[128];
    char pkg_version[64];
    ConfigItem items[MAX_ITEMS];
    int item_count;
} ProjectConfig;

void config_init(ProjectConfig *cfg);
void parse_bld_co(ProjectConfig *cfg, const char *path);

#endif /* PARSER_H */

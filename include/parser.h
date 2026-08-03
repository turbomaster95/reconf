#ifndef PARSER_H
#define PARSER_H

#define MAX_LINE 4096
#define MAX_ITEMS 1024
#define MAX_ARG  2048

typedef enum {
    TYPE_VAR,
    TYPE_HEADER,
    TYPE_CODE,
    TYPE_TARGET_SO,
    TYPE_TARGET_A,
    TYPE_TARGET_BIN,
    TYPE_CUSTOM_LIB,
    TYPE_SOURCES
} ItemType;

typedef struct {
    ItemType type;
    char name[128];
    char arg1[MAX_ARG];
} ConfigItem;

typedef struct {
    ConfigItem items[MAX_ITEMS];
    int item_count;
    char pkg_name[128];
    char pkg_version[64];
} ProjectConfig;

void config_init(ProjectConfig *cfg);
void parse_bld_co(ProjectConfig *cfg, const char *path);

#endif /* PARSER_H */

#include <parser.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void config_init(ProjectConfig *cfg) {
    memset(cfg, 0, sizeof(ProjectConfig));
    strcpy(cfg->pkg_name, "libnu");
    strcpy(cfg->pkg_version, "1.0.0");
}

void parse_bld_co(ProjectConfig *cfg, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("Failed to open build script");
        exit(1);
    }

    char line[MAX_LINE];
    char multiline_buf[MAX_LINE] = "";
    int in_multiline = 0;

    while (fgets(line, sizeof(line), f)) {
        char *t = trim(line);
        if (t[0] == '#' || t[0] == '\0') {
            if (!in_multiline) continue;
        }

        size_t tlen = strlen(t);
        if (tlen > 0 && t[tlen - 1] == '\\') {
            t[tlen - 1] = '\0';
            strcat(multiline_buf, trim(t));
            strcat(multiline_buf, " ");
            in_multiline = 1;
            continue;
        } else if (in_multiline) {
            strcat(multiline_buf, t);
            t = multiline_buf;
            in_multiline = 0;
        }

        char cmd[64] = {0}, arg1[256] = {0}, arg2[2048] = {0};
        int parsed = sscanf(t, "%63s %255s %[^\n]", cmd, arg1, arg2);
        if (parsed < 1) continue;

        if (strcmp(cmd, "set") == 0) {
            ConfigItem *it = &cfg->items[cfg->item_count++];
            it->type = TYPE_VAR;
            strncpy(it->name, arg1, sizeof(it->name) - 1);
            strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
        } else if (strcmp(cmd, "check_header") == 0) {
            ConfigItem *it = &cfg->items[cfg->item_count++];
            it->type = TYPE_HEADER;
            strncpy(it->name, arg1, sizeof(it->name) - 1);
            strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
            unquote(it->arg1);
        } else if (strcmp(cmd, "check_code") == 0) {
            ConfigItem *it = &cfg->items[cfg->item_count++];
            it->type = TYPE_CODE;
            strncpy(it->name, arg1, sizeof(it->name) - 1);
            strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
            unquote(it->arg1);
	} else if (strcmp(cmd, "lib") == 0) {
 	   ConfigItem *it = &cfg->items[cfg->item_count++];
	    it->type = TYPE_CUSTOM_LIB;
	    strncpy(it->name, arg1, sizeof(it->name) - 1);
	    strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
	} else if (strcmp(cmd, "package") == 0) {
	    strncpy(cfg->pkg_name, arg1, sizeof(cfg->pkg_name) - 1);
	    if (arg2[0]) {
	        strncpy(cfg->pkg_version, trim(arg2), sizeof(cfg->pkg_version) - 1);
	    }
	} else if (strcmp(cmd, "target_so") == 0) {
            ConfigItem *it = &cfg->items[cfg->item_count++];
            it->type = TYPE_TARGET_SO;
            strncpy(it->arg1, arg1, sizeof(it->arg1) - 1);
        } else if (strcmp(cmd, "target_a") == 0) {
            ConfigItem *it = &cfg->items[cfg->item_count++];
            it->type = TYPE_TARGET_A;
            strncpy(it->arg1, arg1, sizeof(it->arg1) - 1);
        } else if (strcmp(cmd, "target_bin") == 0) {
            ConfigItem *it = &cfg->items[cfg->item_count++];
            it->type = TYPE_TARGET_BIN;
            strncpy(it->arg1, arg1, sizeof(it->arg1) - 1);
        } else if (strcmp(cmd, "sources") == 0) {
            ConfigItem *it = &cfg->items[cfg->item_count++];
            it->type = TYPE_SOURCES;
            snprintf(it->arg1, sizeof(it->arg1), "%s %s", arg1, trim(arg2));
        } else if (strcmp(cmd, "INCLUDES") == 0) {
            ConfigItem *it = &cfg->items[cfg->item_count++];
            it->type = TYPE_VAR;
            strcpy(it->name, "INCLUDES");
            snprintf(it->arg1, sizeof(it->arg1), "%s %s", arg1, trim(arg2));
        }

        multiline_buf[0] = '\0';
    }

    fclose(f);
}

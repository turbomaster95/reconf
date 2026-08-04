#include <parser.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void config_init(ProjectConfig *cfg) {
    memset(cfg, 0, sizeof(ProjectConfig));
    strcpy(cfg->pkg_name, "app");
    strcpy(cfg->pkg_version, "0.1.0");
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
    int current_target_idx = -1;

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

        char cmd[64] = {0}, arg1[512] = {0}, arg2[MAX_ARG] = {0};
        int parsed = sscanf(t, "%63s %511s %[^\n]", cmd, arg1, arg2);
        if (parsed < 1) {
            multiline_buf[0] = '\0';
            continue;
        }

        if (strcmp(cmd, "set") == 0) {
            if (cfg->item_count < MAX_ITEMS) {
                ConfigItem *it = &cfg->items[cfg->item_count++];
                it->type = TYPE_VAR;
                strncpy(it->name, arg1, sizeof(it->name) - 1);
                strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
            }
        } else if (strcmp(cmd, "check_header") == 0) {
            if (cfg->item_count < MAX_ITEMS) {
                ConfigItem *it = &cfg->items[cfg->item_count++];
                it->type = TYPE_HEADER;
                strncpy(it->name, arg1, sizeof(it->name) - 1);
                strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
                unquote(it->arg1);
            }
        } else if (strcmp(cmd, "check_code") == 0) {
            if (cfg->item_count < MAX_ITEMS) {
                ConfigItem *it = &cfg->items[cfg->item_count++];
                it->type = TYPE_CODE;
                strncpy(it->name, arg1, sizeof(it->name) - 1);
                strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
                unquote(it->arg1);
            }
        } else if (strcmp(cmd, "run") == 0) {
            if (cfg->item_count < MAX_ITEMS) {
                ConfigItem *it = &cfg->items[cfg->item_count++];
                it->type = TYPE_RUN;
                strncpy(it->name, arg1, sizeof(it->name) - 1);
                strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
                unquote(it->arg1);
                current_target_idx = cfg->item_count - 1;
            }
        } else if (strcmp(cmd, "script") == 0) {
            if (cfg->item_count < MAX_ITEMS) {
                ConfigItem *it = &cfg->items[cfg->item_count++];
                it->type = TYPE_SCRIPT;
                strncpy(it->name, arg1, sizeof(it->name) - 1);
                strncpy(it->arg1, trim(arg2), sizeof(it->arg1) - 1);
                unquote(it->arg1);
                current_target_idx = cfg->item_count - 1;
            }
        } else if (strcmp(cmd, "clean") == 0) {
            if (cfg->item_count < MAX_ITEMS) {
                ConfigItem *it = &cfg->items[cfg->item_count++];
                it->type = TYPE_CLEAN;
                snprintf(it->arg1, sizeof(it->arg1), "%s %s", arg1, trim(arg2));
            }
        } else if (strcmp(cmd, "deps") == 0 && current_target_idx != -1) {
            ConfigItem *tgt = &cfg->items[current_target_idx];
            snprintf(tgt->deps, sizeof(tgt->deps), "%s %s", arg1, trim(arg2));
        } else if (strcmp(cmd, "package") == 0) {
            strncpy(cfg->pkg_name, arg1, sizeof(cfg->pkg_name) - 1);
            if (arg2[0]) {
                strncpy(cfg->pkg_version, trim(arg2), sizeof(cfg->pkg_version) - 1);
            }
        } else if (strcmp(cmd, "target_so") == 0 || strcmp(cmd, "target_a") == 0 || strcmp(cmd, "target_bin") == 0) {
            if (cfg->item_count < MAX_ITEMS) {
                ConfigItem *it = &cfg->items[cfg->item_count++];
                if (strcmp(cmd, "target_so") == 0) it->type = TYPE_TARGET_SO;
                else if (strcmp(cmd, "target_a") == 0) it->type = TYPE_TARGET_A;
                else it->type = TYPE_TARGET_BIN;

                strncpy(it->arg1, arg1, sizeof(it->arg1) - 1);
                current_target_idx = cfg->item_count - 1;
            }
        } else if (strcmp(cmd, "sources") == 0 || (current_target_idx != -1 && strcmp(cmd, cfg->items[current_target_idx].arg1) == 0)) {
            char *sources_ptr = NULL;

            if (strcmp(cmd, "sources") == 0) {
                if (parsed > 2) {
                    snprintf(arg2, sizeof(arg2), "%s %s", arg1, trim(arg2));
                    sources_ptr = arg2;
                } else {
                    sources_ptr = arg1;
                }
            } else {
                if (strcmp(arg1, "-") == 0) {
                    char sub_cmd[64] = {0}, sub_args[MAX_ARG] = {0};
                    if (sscanf(arg2, "%63s %[^\n]", sub_cmd, sub_args) >= 1 && strcmp(sub_cmd, "sources") == 0) {
                        sources_ptr = trim(sub_args);
                    }
                }
            }

            if (sources_ptr && current_target_idx != -1) {
                ConfigItem *tgt = &cfg->items[current_target_idx];
                if (tgt->target_sources[0] != '\0') {
                    strncat(tgt->target_sources, " ", sizeof(tgt->target_sources) - strlen(tgt->target_sources) - 1);
                }
                strncat(tgt->target_sources, sources_ptr, sizeof(tgt->target_sources) - strlen(tgt->target_sources) - 1);
            }
        } else if (strcmp(cmd, "INCLUDES") == 0) {
            if (cfg->item_count < MAX_ITEMS) {
                ConfigItem *it = &cfg->items[cfg->item_count++];
                it->type = TYPE_VAR;
                strcpy(it->name, "INCLUDES");
                snprintf(it->arg1, sizeof(it->arg1), "%s %s", arg1, trim(arg2));
            }
        }

        multiline_buf[0] = '\0';
    }

    fclose(f);
}

#include <stdio.h>
#include <parser.h>
#include <cgen.h>
#include <stdlib.h>

void minit(void *memory, size_t mem_size);
int is_valid_reconf_file(const char *path);

#define ARENA_SIZE (1024 * 1024 * 3) // 3MB
static unsigned char heap_arena[ARENA_SIZE];

int main(int argc, char **argv) {
    const char *bld_file = (argc > 1) ? argv[1] : "build.co";
    const char *out_file = "configure";

    ProjectConfig config;
    config_init(&config);

    if (!is_valid_reconf_file(bld_file)) {
        fprintf(stderr, "reconf: aborting configuration generation.\n");
        return EXIT_FAILURE;
    }

    parse_bld_co(&config, bld_file);

    if (generate_configure(&config, out_file) != 0) {
        fprintf(stderr, "Error: Failed to generate %s\n", out_file);
        return 1;
    }

    printf("Successfully generated executable '%s' from '%s'\n", out_file, bld_file);
    return 0;
}

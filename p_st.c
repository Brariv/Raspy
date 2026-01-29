/*
  Lista PID, estado y nombre leyendo /proc/<pid>/status
    gcc -O2 -Wall -Wextra -o p_st p_st.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#define PATH_MAX_LEN 512
#define LINE_MAX_LEN 1024

static int is_all_digits(const char *s) {
    if (!s || !*s) return 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        if (!isdigit(*p)) return 0;
    }
    return 1;
}

static void extract_field(char *dst, size_t dst_size, const char *src) {
    /* copia hasta '\n' o hasta dst_size-1 */
    size_t i = 0;
    while (src[i] && src[i] != '\n' && i + 1 < dst_size) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

int main(void) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        return 1;
    }

    printf("%-8s %-6s %s\n", "PID", "STATE", "NAME");
    printf("---------------------------------------------\n");

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (!is_all_digits(ent->d_name)) continue;

        char path[PATH_MAX_LEN];
        snprintf(path, sizeof(path), "/proc/%s/status", ent->d_name);

        FILE *f = fopen(path, "r");
        if (!f) continue;

        char line[LINE_MAX_LEN];
        char name[256] = "";
        char state[256] = "";

        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "Name:", 5) == 0) {
                const char *p = line + 5;
                while (*p == ' ' || *p == '\t') p++;
                extract_field(name, sizeof(name), p);
            }
            else if (strncmp(line, "State:", 6) == 0) {
                const char *p = line + 6;
                while (*p == ' ' || *p == '\t') p++;
                extract_field(state, sizeof(state), p);
            }

            if (name[0] && state[0]) break;
        }

        fclose(f);

        if (!name[0]) strcpy(name, "(unknown)");
        if (!state[0]) strcpy(state, "?");

        printf("%-8s %-6c %s\n", ent->d_name, state[0], name);
    }

    closedir(dir);
    return 0;
}

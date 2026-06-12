#include <stdio.h>
#include <stdlib.h>
#include "recorder.h"

/*
 * recorder.c
 *
 * Frame file format (plain text, easy to parse from any language):
 *
 *   line 1:        N                  (number of elements)
 *   line 2..k:     v0 v1 ... vN-1|h1 h2
 *
 * where v* are the array values at that moment and h1/h2 are the
 * highlighted indices (-1 means "no highlight").
 */

static FILE *out = NULL;

static void write_line(const int *array, int n, int hl1, int hl2)
{
    for (int i = 0; i < n; i++) {
        fprintf(out, i == 0 ? "%d" : " %d", array[i]);
    }
    fprintf(out, "|%d %d\n", hl1, hl2);
}

int recorder_open(const char *path, const int *array, int n)
{
    out = fopen(path, "w");
    if (out == NULL) {
        perror("recorder_open");
        return -1;
    }
    fprintf(out, "%d\n", n);
    write_line(array, n, -1, -1);  /* initial unsorted state */
    return 0;
}

void recorder_frame(const int *array, int n, int hl1, int hl2)
{
    if (out != NULL) {
        write_line(array, n, hl1, hl2);
    }
}

void recorder_close(const int *array, int n)
{
    if (out != NULL) {
        write_line(array, n, -1, -1);  /* final sorted state */
        fclose(out);
        out = NULL;
    }
}

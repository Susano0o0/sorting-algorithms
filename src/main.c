#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sorts.h"
#include "recorder.h"

/*
 * main.c
 *
 * Runs every sorting algorithm on the same shuffled array and
 * records each run into data/<name>.frames. Those files are then
 * rendered into GIFs by scripts/render_gif.py.
 *
 * Usage:
 *   ./sortviz            run all algorithms
 *   ./sortviz quick      run only quick sort (any name from the table)
 */

#define ARRAY_SIZE 40
#define RANDOM_SEED 42  /* fixed seed: every run produces the same animation */

typedef void (*sort_fn)(int *array, int n);

typedef struct {
    const char *name;
    sort_fn fn;
} sort_entry;

static const sort_entry SORTS[] = {
    { "bubble",    bubble_sort    },
    { "selection", selection_sort },
    { "insertion", insertion_sort },
    { "merge",     merge_sort     },
    { "quick",     quick_sort     },
    { "heap",      heap_sort      },
};

#define SORT_COUNT (sizeof(SORTS) / sizeof(SORTS[0]))

/* Fill the array with 1..n and shuffle it (Fisher-Yates).
 * A fixed seed keeps the output reproducible, which matters
 * when you want identical GIFs on every machine. */
static void make_shuffled_array(int *array, int n)
{
    for (int i = 0; i < n; i++) {
        array[i] = i + 1;
    }

    srand(RANDOM_SEED);
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

static int is_sorted(const int *array, int n)
{
    for (int i = 1; i < n; i++) {
        if (array[i - 1] > array[i]) {
            return 0;
        }
    }
    return 1;
}

static int run_sort(const sort_entry *entry)
{
    int array[ARRAY_SIZE];
    char path[256];

    make_shuffled_array(array, ARRAY_SIZE);
    snprintf(path, sizeof(path), "data/%s.frames", entry->name);

    if (recorder_open(path, array, ARRAY_SIZE) != 0) {
        return -1;
    }

    entry->fn(array, ARRAY_SIZE);
    recorder_close(array, ARRAY_SIZE);

    /* Sanity check: the recording is useless if the sort is broken. */
    if (!is_sorted(array, ARRAY_SIZE)) {
        fprintf(stderr, "ERROR: %s sort produced an unsorted array!\n",
                entry->name);
        return -1;
    }

    printf("  %-9s -> %s\n", entry->name, path);
    return 0;
}

int main(int argc, char **argv)
{
    printf("Recording sorting animations (n = %d):\n", ARRAY_SIZE);

    int failures = 0;
    for (size_t i = 0; i < SORT_COUNT; i++) {
        /* If an algorithm name was given, skip all the others. */
        if (argc > 1 && strcmp(argv[1], SORTS[i].name) != 0) {
            continue;
        }
        if (run_sort(&SORTS[i]) != 0) {
            failures++;
        }
    }

    if (failures > 0) {
        fprintf(stderr, "%d algorithm(s) failed.\n", failures);
        return EXIT_FAILURE;
    }

    printf("Done. Render the GIFs with: python3 scripts/render_gif.py\n");
    return EXIT_SUCCESS;
}

#ifndef RECORDER_H
#define RECORDER_H

/*
 * recorder.h
 *
 * A tiny "tape recorder" for sorting algorithms.
 *
 * Every time an algorithm does something interesting (a comparison,
 * a swap, a write-back), it calls recorder_frame(). The recorder
 * appends a snapshot of the whole array to a plain text file.
 *
 * Later, scripts/render_gif.py reads that file and turns the
 * snapshots into an animated GIF. This keeps the C code free of
 * any graphics dependencies: it only writes text.
 */

/* Open a frames file and write the initial (unsorted) state.
 * Returns 0 on success, -1 on failure. */
int recorder_open(const char *path, const int *array, int n);

/* Append one snapshot of the array.
 * hl1 and hl2 are indices to highlight in the animation
 * (e.g. the two elements being compared). Pass -1 for "none". */
void recorder_frame(const int *array, int n, int hl1, int hl2);

/* Write the final (sorted) state and close the file. */
void recorder_close(const int *array, int n);

#endif /* RECORDER_H */

#include <stdlib.h>
#include "sorts.h"
#include "recorder.h"

/*
 * sorts.c
 *
 * Implementation notes:
 *  - All sorts work in place and sort in ascending order.
 *  - recorder_frame() is called on every comparison or write,
 *    so the resulting animation shows how each algorithm "thinks".
 *  - The recorder is a no-op when no file is open, so these
 *    functions can also be used as a normal sorting library.
 */

static void swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

/* ----------------------------------------------------------------
 * Bubble sort: O(n^2)
 *
 * Repeatedly walks through the array and swaps adjacent elements
 * that are out of order. After pass k, the k largest elements
 * have "bubbled up" to the end, so each pass can stop earlier.
 * ---------------------------------------------------------------- */
void bubble_sort(int *array, int n)
{
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;

        for (int j = 0; j < n - 1 - i; j++) {
            recorder_frame(array, n, j, j + 1);  /* comparing neighbours */

            if (array[j] > array[j + 1]) {
                swap(&array[j], &array[j + 1]);
                swapped = 1;
                recorder_frame(array, n, j, j + 1);  /* after the swap */
            }
        }

        /* Early exit: a full pass without swaps means we are done. */
        if (!swapped) {
            break;
        }
    }
}

/* ----------------------------------------------------------------
 * Selection sort: O(n^2)
 *
 * For each position i, scans the unsorted tail for the minimum
 * and swaps it into place. Does at most n-1 swaps in total,
 * which is its one practical advantage.
 * ---------------------------------------------------------------- */
void selection_sort(int *array, int n)
{
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;

        for (int j = i + 1; j < n; j++) {
            recorder_frame(array, n, min_idx, j);  /* scanning for minimum */

            if (array[j] < array[min_idx]) {
                min_idx = j;
            }
        }

        if (min_idx != i) {
            swap(&array[i], &array[min_idx]);
            recorder_frame(array, n, i, min_idx);  /* after the swap */
        }
    }
}

/* ----------------------------------------------------------------
 * Insertion sort: O(n^2), but O(n) on nearly sorted input
 *
 * Grows a sorted prefix one element at a time: takes the next
 * element ("key") and shifts larger elements right until the
 * key fits. This is how most people sort playing cards.
 * ---------------------------------------------------------------- */
void insertion_sort(int *array, int n)
{
    for (int i = 1; i < n; i++) {
        int key = array[i];
        int j = i - 1;

        /* Shift everything greater than key one slot to the right. */
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            recorder_frame(array, n, j, j + 1);  /* element shifted */
            j--;
        }

        array[j + 1] = key;
        recorder_frame(array, n, j + 1, -1);  /* key inserted */
    }
}

/* ----------------------------------------------------------------
 * Merge sort: O(n log n), stable, needs O(n) extra memory
 *
 * Classic divide and conquer: split the array in halves, sort
 * each half recursively, then merge the two sorted halves.
 * ---------------------------------------------------------------- */
static void merge(int *array, int *buffer, int left, int mid, int right, int n)
{
    int i = left;      /* cursor in the left half  */
    int j = mid + 1;   /* cursor in the right half */
    int k = left;      /* cursor in the buffer     */

    /* Pick the smaller head of the two halves until one runs out. */
    while (i <= mid && j <= right) {
        if (array[i] <= array[j]) {
            buffer[k++] = array[i++];
        } else {
            buffer[k++] = array[j++];
        }
    }
    while (i <= mid)   buffer[k++] = array[i++];
    while (j <= right) buffer[k++] = array[j++];

    /* Copy the merged run back, recording each write so the
     * animation shows the merge happening element by element. */
    for (k = left; k <= right; k++) {
        array[k] = buffer[k];
        recorder_frame(array, n, k, -1);
    }
}

static void merge_sort_rec(int *array, int *buffer, int left, int right, int n)
{
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;
    merge_sort_rec(array, buffer, left, mid, n);
    merge_sort_rec(array, buffer, mid + 1, right, n);
    merge(array, buffer, left, mid, right, n);
}

void merge_sort(int *array, int n)
{
    int *buffer = malloc((size_t)n * sizeof(int));
    if (buffer == NULL) {
        return;  /* allocation failed, leave the array untouched */
    }

    merge_sort_rec(array, buffer, 0, n - 1, n);
    free(buffer);
}

/* ----------------------------------------------------------------
 * Quick sort: O(n log n) average, O(n^2) worst case
 *
 * Picks a pivot (here: the last element, Lomuto scheme),
 * partitions the array around it and recurses on both sides.
 * In practice one of the fastest comparison sorts.
 * ---------------------------------------------------------------- */
static int partition(int *array, int low, int high, int n)
{
    int pivot = array[high];
    int i = low - 1;  /* boundary of the "smaller than pivot" zone */

    for (int j = low; j < high; j++) {
        recorder_frame(array, n, j, high);  /* comparing with the pivot */

        if (array[j] < pivot) {
            i++;
            swap(&array[i], &array[j]);
            recorder_frame(array, n, i, j);  /* after the swap */
        }
    }

    swap(&array[i + 1], &array[high]);
    recorder_frame(array, n, i + 1, high);  /* pivot moved into place */
    return i + 1;
}

static void quick_sort_rec(int *array, int low, int high, int n)
{
    if (low < high) {
        int p = partition(array, low, high, n);
        quick_sort_rec(array, low, p - 1, n);
        quick_sort_rec(array, p + 1, high, n);
    }
}

void quick_sort(int *array, int n)
{
    quick_sort_rec(array, 0, n - 1, n);
}

/* ----------------------------------------------------------------
 * Heap sort: O(n log n), in place, not stable
 *
 * Builds a max-heap on top of the array, then repeatedly swaps
 * the heap root (the maximum) to the end and shrinks the heap.
 * ---------------------------------------------------------------- */
static void sift_down(int *array, int heap_size, int root, int n)
{
    while (1) {
        int largest = root;
        int left = 2 * root + 1;
        int right = 2 * root + 2;

        if (left < heap_size && array[left] > array[largest]) {
            largest = left;
        }
        if (right < heap_size && array[right] > array[largest]) {
            largest = right;
        }
        if (largest == root) {
            break;  /* heap property restored */
        }

        swap(&array[root], &array[largest]);
        recorder_frame(array, n, root, largest);  /* sift-down swap */
        root = largest;
    }
}

void heap_sort(int *array, int n)
{
    /* Phase 1: build a max-heap (start from the last parent node). */
    for (int i = n / 2 - 1; i >= 0; i--) {
        sift_down(array, n, i, n);
    }

    /* Phase 2: extract the maximum one by one. */
    for (int end = n - 1; end > 0; end--) {
        swap(&array[0], &array[end]);
        recorder_frame(array, n, 0, end);  /* max moved to its place */
        sift_down(array, end, 0, n);
    }
}

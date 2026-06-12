#ifndef SORTS_H
#define SORTS_H

/*
 * sorts.h
 *
 * Six classic sorting algorithms. Each function sorts the array
 * in place (ascending order) and reports its progress through the
 * recorder module, so the steps can be replayed as an animation.
 */

void bubble_sort(int *array, int n);
void selection_sort(int *array, int n);
void insertion_sort(int *array, int n);
void merge_sort(int *array, int n);
void quick_sort(int *array, int n);
void heap_sort(int *array, int n);

#endif /* SORTS_H */

#include <stdio.h>
#include "mymalloc.h"

int main() {
    // Allocate memory for an integer
    int *p1 = (int *)mymalloc(sizeof(int));
    if (p1 != NULL) {
        *p1 = 42;
        printf("Allocated integer: %d\n", *p1);
    }

    // Allocate an array of 5 integers
    int *arr = (int *)mymalloc(5 * sizeof(int));
    if (arr != NULL) {
        for (int i = 0; i < 5; i++) {
            arr[i] = i + 1;
        }
        printf("Allocated array: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }

    // Print heap status
    printf("\nHeap status after allocations:\n");
    printheap();

    // Free the allocated memory
    myfree(p1);
    myfree(arr);

    printf("\nHeap status after freeing:\n");
    printheap();

    // Test different allocation strategies
    setstrategy(FIRST_FIT);
    int *p2 = (int *)mymalloc(sizeof(int));
    
    setstrategy(BEST_FIT);
    char *str = (char *)mymalloc(20);

    if (str != NULL) {
        myfree(str);
    }
    if (p2 != NULL) {
        myfree(p2);
    }

    return 0;
}

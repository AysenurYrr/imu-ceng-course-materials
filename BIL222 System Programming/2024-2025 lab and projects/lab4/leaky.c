#include <stdlib.h>
#include <stdio.h>

void leaky_function() {
    int* arr = malloc(sizeof(int) * 10); // Never freed
    arr[0] = 42;
}

void invalid_access() {
    int* ptr = malloc(sizeof(int));
    free(ptr);
    *ptr = 100; // Writing to freed memory!
}

int main() {
    leaky_function();
    invalid_access();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
typedef struct{
    int *at;
    char y;
    int x;
    char y2;
    struct {
        int b;
    } f1;
    // char z[10];
    // int *pa;
    // int b;
    char y3;
} Foo1;

typedef struct{
    int a;
    int b;
    int *pa;
    Foo1 f1;
} Foo2;
Foo1 f2;
Foo2 f4;
void initialize(int *data) {
    *data = 100; // Dereferencing a NULL pointer
}

int main() {
    int *value = malloc(sizeof(int)); // Fix: Allocate memory
    initialize(value);
    printf("Value: %d\n", *value);
    free(value);
    return 0;
}
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main() {
    int* a = malloc(sizeof(int) * 15);
    a[0] = 137;
    printf( "%d %f %p\n", a[0], sqrt(*a), a );
    return 0;
}

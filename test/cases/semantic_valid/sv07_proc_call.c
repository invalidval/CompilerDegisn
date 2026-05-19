#include <stdio.h>
int a;
int b;


void swap(int *x, int *y);

void swap(int *x, int *y) {
    int tmp;

    tmp = (*x);
    (*x) = (*y);
    (*y) = tmp;

}


int main(void) {
    a = 1;
    b = 2;
    swap(&a, &b);

    return 0;
}

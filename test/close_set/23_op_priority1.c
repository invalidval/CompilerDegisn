#include <stdio.h>
int a;
int b;
int c;
int d;


int main(void) {
    a = 10;
    b = 4;
    c = 2;
    d = 2;
    printf("%d", ((c + (a * b)) - d));

    return 0;
}

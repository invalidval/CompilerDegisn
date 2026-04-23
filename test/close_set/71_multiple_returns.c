#include <stdio.h>
const int aa = 4;
int a;
int b;
int c;
int d;


int main(void) {
    b = 8;
    c = 12;
    a = (b + c);
    d = 9;
    a = ((aa - b) * c);
    printf("%d", a);

    return 0;
}

#include <stdio.h>
int a;

int b;


int func(int p);

int func(int p) {
    int _retval;
p = (p - 1);
_retval = p;

    return _retval;
}


int main(void) {
a = 10;
b = func(a);
printf("%d", b);

    return 0;
}

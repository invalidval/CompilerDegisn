#include <stdio.h>
int g, i;

int func(int n);

int func(int n) {
    int _retval;
g = (g + n);
printf("%d", g);
_retval = g;

    return _retval;
}


int main(void) {
i = 11;
if (((i > 10) && (func(i) != 0))) {
        i = 1;
    } else {
        i = 0;
    }
printf("%d", i);
i = 10;
if (((i > 11) && (func(i) != 0))) {
        i = 1;
    } else {
        i = 0;
    }
printf("%d", i);
i = 100;
if (((i <= 99) || (func(i) != 0))) {
        i = 1;
    } else {
        i = 0;
    }
printf("%d", i);
i = 99;
if (((i <= 100) || (func(i) != 0))) {
        i = 1;
    } else {
        i = 0;
    }
if (((func(99) == 0) && (func(100) != 0))) {
        i = 1;
    } else {
        i = 0;
    }

    return 0;
}

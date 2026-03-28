#include <stdio.h>
int if_if_else();

int if_if_else() {
    int _retval;
    int a, b;
a = 5;
b = 10;
if ((a == 5)) {
        if ((b == 10)) {
        a = 25;
    }
    } else {
        a = (a + 15);
    }
_retval = a;

    return _retval;
}


int main(void) {
printf("%d", if_if_else);

    return 0;
}

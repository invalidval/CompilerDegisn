#include <stdio.h>

int ififelse();

int ififelse() {
    int _retval;
a = 5;
b = 10;
if ((a = 5)) {
        if ((b = 10)) {
        a = 25;
    } else {
        a = (a + 15);
    }
    }
_retval = a;

    return _retval;
}


int main(void) {
printf("%d", ififelse);

    return 0;
}

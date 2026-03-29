#include <stdio.h>
int ret;


int ifwhile();

int ifwhile() {
    int _retval;
    int a;
int b;

a = 0;
b = 1;
if ((a == 5)) {
        for (b = 1; b <= 3; b++) {
    }
b = (b + 25);
_retval = b;
    } else {
        for (a = 0; a <= 4; a++) {
        b = (b * 2);
    }
    }
_retval = b;

    return _retval;
}


int main(void) {
printf("%d", ifwhile());

    return 0;
}

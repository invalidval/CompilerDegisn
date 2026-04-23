#include <stdio.h>
int ret;
int f;


int fn();

int fn() {
    int _retval;
    _retval = 10;

    return _retval;
}


int main(void) {
    ret = 0;
    f = 20;
    ret = f;
    printf("%d", ret);

    return 0;
}

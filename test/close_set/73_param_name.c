#include <stdio.h>
int ret;


int fn(int f);

int fn(int f) {
    int _retval;
    _retval = (f * 2);

    return _retval;
}


int main(void) {
    ret = 0;
    ret = fn(10);
    printf("%d", ret);

    return 0;
}

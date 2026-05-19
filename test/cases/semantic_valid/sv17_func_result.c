#include <stdio.h>
int n;
int result;


int factorial(int x);

int factorial(int x) {
    int _retval;
    if ((x <= 1)) {
        _retval = 1;
    } else {
        _retval = (x * factorial((x - 1)));
    }

    return _retval;
}


int main(void) {
    n = 5;
    result = factorial(n);

    return 0;
}

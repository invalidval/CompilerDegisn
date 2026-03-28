#include <stdio.h>
int x[1], y[1];
int a, b;

int exgcd(int a, int b, int *x, int *y);

int exgcd(int a, int b, int *x, int *y) {
    int _retval;
    int t, r;
if ((b == 0)) {
        (*x) = 1;
(*y) = 0;
_retval = a;
    } else {
        r = exgcd(b, (a % b), x, y);
t = (*x);
(*x) = (*y);
(*y) = (t - ((a / b) * (*y)));
_retval = r;
    }

    return _retval;
}


int main(void) {
a = 7;
b = 15;
x[0] = 1;
y[0] = 1;
exgcd(a, b, &x[0], &y[0]);
x[0] = (((x[0] % b) + b) % b);
printf("%d", x[0]);

    return 0;
}

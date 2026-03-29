#include <stdio.h>
const char t = 's';
const double a = 1e6;
int x, y;
int z[10][7];
int u;

int gcd(int a, int b);

int gcd(int a, int b) {
    int _retval;
if ((b == 0)) {
        _retval = a;
    } else {
        _retval = gcd(b, (a % b));
    }

    return _retval;
}


int main(void) {
x = (2 + 1);
z[(2) - 1][(2) - 2] = 5;
u = 1;
scanf("%d%d", &x, &y);
printf("%d", gcd(x, y));

    return 0;
}

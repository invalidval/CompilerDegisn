#include <stdio.h>
int a;
int b;
int c;
int d;
int e;


int main(void) {
a = 5;
b = 5;
c = 1;
d = (-2);
e = ((((d * 1) / 2) + (a - b)) - ((-(c + 3)) % 2));
printf("%d", e);
e = ((((d % 2) + 67) + (-(a - b))) - (-((c + 2) % 2)));
e = (e + 3);
printf("%d", e);

    return 0;
}

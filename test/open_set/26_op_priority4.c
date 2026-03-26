#include <stdio.h>

int a, b, c, d, e;
int flag;

int main(void) {
scanf("%d", &a);
scanf("%d", &b);
scanf("%d", &c);
scanf("%d", &d);
scanf("%d", &e);
flag = false;
if (((((a - (b * c)) <> (d - (a / c))) or (((a * b) / c) = (e + d))) or (((a + b) + c) = (d + e)))) {
        flag = true;
    }
if (flag) {
        printf("%d", 1);
    }

    return 0;
}

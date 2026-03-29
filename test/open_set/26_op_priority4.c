#include <stdio.h>
int a;
int b;
int c;
int d;
int e;

int flag;


int main(void) {
scanf("%d", &a);
scanf("%d", &b);
scanf("%d", &c);
scanf("%d", &d);
scanf("%d", &e);
flag = 0;
if (((((a - (b * c)) != (d - (a / c))) || (((a * b) / c) == (e + d))) || (((a + b) + c) == (d + e)))) {
        flag = 1;
    }
if (flag) {
        printf("%d", 1);
    }

    return 0;
}

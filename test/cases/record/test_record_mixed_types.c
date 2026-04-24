#include <stdio.h>
typedef struct {
    int a;
    int b;
    int c;
    float x;
    float y;
    int flag;
} data;
data d;


int main(void) {
    d.a = 1;
    d.b = 2;
    d.c = 3;
    d.x = 1.5;
    d.y = 2.5;
    d.flag = 1;
    printf("%d", d.a);
    printf("%d", d.b);
    printf("%d", d.c);
    printf("%f", d.x);
    printf("%f", d.y);

    return 0;
}

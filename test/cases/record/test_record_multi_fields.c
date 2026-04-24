#include <stdio.h>
typedef struct {
    int x;
    int y;
    int z;
} point;
point p;


int main(void) {
    p.x = 1;
    p.y = 2;
    p.z = 3;
    printf("%d", p.x);
    printf("%d", p.y);
    printf("%d", p.z);

    return 0;
}

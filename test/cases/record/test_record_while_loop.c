#include <stdio.h>
typedef struct {
    int x;
    int y;
} point;
point p;


int main(void) {
    p.x = 0;
    p.y = 0;
    while ((p.x < 5)) {
        p.x = (p.x + 1);
        p.y = (p.y + 2);
        printf("%d", p.x);
        printf("%d", p.y);
    }

    return 0;
}

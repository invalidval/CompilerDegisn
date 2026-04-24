#include <stdio.h>
typedef struct {
    int x;
    int y;
} point;
typedef struct {
    int width;
    int height;
} rect;
point p;

rect r;

int area;


int main(void) {
    p.x = 10;
    p.y = 20;
    r.width = 30;
    r.height = 40;
    area = (r.width * r.height);
    printf("%d", p.x);
    printf("%d", p.y);
    printf("%d", area);

    return 0;
}

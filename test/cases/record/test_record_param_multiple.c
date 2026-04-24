#include <stdio.h>
typedef struct {
    int x;
    int y;
} point;
point p1;
point p2;


int addpoints(point a, point b);

int addpoints(point a, point b) {
    int _retval;
    _retval = (((a.x + a.y) + b.x) + b.y);

    return _retval;
}


int main(void) {
    p1.x = 10;
    p1.y = 20;
    p2.x = 30;
    p2.y = 40;
    printf("%d", addpoints(p1, p2));

    return 0;
}

#include <stdio.h>
typedef struct {
    int x;
    int y;
} point;
point p;


void resetpoint();

void resetpoint() {
    p.x = 0;
    p.y = 0;

}


int main(void) {
    p.x = 10;
    p.y = 20;
    printf("%d", p.x);
    printf("%d", p.y);
    resetpoint();
    printf("%d", p.x);
    printf("%d", p.y);

    return 0;
}

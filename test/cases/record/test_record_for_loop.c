#include <stdio.h>
typedef struct {
    int value;
    int step;
} counter;
counter c;

int i;


int main(void) {
    c.value = 0;
    c.step = 5;
    for (i = 1; i <= 10; i++) {
        c.value = (c.value + c.step);
        printf("%d", c.value);
    }

    return 0;
}

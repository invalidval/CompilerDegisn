#include <stdio.h>
typedef struct {
    int count;
    float sum;
    float avg;
} stats;
stats s;

int i;


int main(void) {
    s.count = 0;
    s.sum = 0.0;
    for (i = 1; i <= 5; i++) {
        s.count = (s.count + 1);
        s.sum = (s.sum + i);
    }
    s.avg = (s.sum / s.count);
    printf("%d", s.count);
    printf("%f", s.sum);
    printf("%f", s.avg);

    return 0;
}

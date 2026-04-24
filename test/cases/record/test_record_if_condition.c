#include <stdio.h>
typedef struct {
    int age;
    float score;
    int active;
} person;
person p;


int main(void) {
    p.age = 25;
    p.score = 85.5;
    p.active = 1;
    if (p.active) {
        if ((p.score > 90.0)) {
            printf("%d", 1);
        } else {
            printf("%d", 0);
        }
    } else {
        printf("%d", (-1));
    }

    return 0;
}

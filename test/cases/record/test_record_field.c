#include <stdio.h>
typedef struct {
    int age;
    float score;
} person;
person p;


int main(void) {
    p.age = 25;
    printf("%d", p.age);

    return 0;
}

#include <stdio.h>
typedef struct {
    int age;
    float score;
    char initial;
} person;
person p1;
person p2;


int main(void) {
    p1.age = 25;
    p1.score = 95.5;
    p1.initial = 'A';
    p2.age = 30;
    p2.score = 88.0;
    p2.initial = 'B';
    printf("%d", p1.age);
    printf("%f", p1.score);
    printf("%d", p2.age);
    printf("%f", p2.score);

    return 0;
}

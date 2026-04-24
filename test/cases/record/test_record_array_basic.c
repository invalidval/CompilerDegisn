#include <stdio.h>
typedef struct {
    int age;
    float score;
} person;
person people[3];

int i;


int main(void) {
    people[(1) - 1].age = 20;
    people[(1) - 1].score = 85.5;
    people[(2) - 1].age = 25;
    people[(2) - 1].score = 90.0;
    people[(3) - 1].age = 30;
    people[(3) - 1].score = 95.5;
    for (i = 1; i <= 3; i++) {
        printf("%d", people[(i) - 1].age);
    }

    return 0;
}

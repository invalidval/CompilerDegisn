#include <stdio.h>
typedef struct {
    int age;
    float score;
} person;
person people[3];

int i;

int totalage;

float avgscore;


int main(void) {
    people[(1) - 1].age = 20;
    people[(1) - 1].score = 80.0;
    people[(2) - 1].age = 25;
    people[(2) - 1].score = 90.0;
    people[(3) - 1].age = 30;
    people[(3) - 1].score = 100.0;
    totalage = 0;
    avgscore = 0.0;
    for (i = 1; i <= 3; i++) {
        totalage = (totalage + people[(i) - 1].age);
        avgscore = (avgscore + people[(i) - 1].score);
    }
    avgscore = (avgscore / 3.0);
    printf("%d", totalage);

    return 0;
}

#include <stdio.h>
typedef struct {
    int id;
    char name;
    float score;
    int passed;
} student;
student s1;
student s2;
student s3;


int main(void) {
    s1.id = 1;
    s1.name = 'A';
    s1.score = 95.5;
    s1.passed = 1;
    s2.id = 2;
    s2.name = 'B';
    s2.score = 88.0;
    s2.passed = 1;
    s3.id = 3;
    s3.name = 'C';
    s3.score = 65.5;
    s3.passed = 0;
    printf("%d", s1.id);
    printf("%f", s1.score);
    printf("%d", s2.id);
    printf("%f", s2.score);
    printf("%d", s3.id);
    printf("%f", s3.score);

    return 0;
}

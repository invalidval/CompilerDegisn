#include <stdio.h>
typedef struct {
    int age;
    float score;
} person;
person p;

int result;


int getage(person p);

int getage(person p) {
    int _retval;
    _retval = p.age;

    return _retval;
}


int main(void) {
    p.age = 25;
    p.score = 95.5;
    result = getage(p);
    printf("%d", result);

    return 0;
}

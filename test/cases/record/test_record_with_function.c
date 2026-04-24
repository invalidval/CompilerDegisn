#include <stdio.h>
typedef struct {
    int age;
    float score;
} person;
person p;

int maxage;


int getmax(int a, int b);

int getmax(int a, int b) {
    int _retval;
    if ((a > b)) {
        _retval = a;
    } else {
        _retval = b;
    }

    return _retval;
}


int main(void) {
    p.age = 25;
    p.score = 95.5;
    maxage = getmax(p.age, 30);
    printf("%d", maxage);

    return 0;
}

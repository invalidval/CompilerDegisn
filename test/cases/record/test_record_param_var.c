#include <stdio.h>
typedef struct {
    int age;
    float score;
} person;
person p;


void modifyperson(person *p);

void modifyperson(person *p) {
    (*p).age = 30;
    (*p).score = 88.0;

}


int main(void) {
    p.age = 25;
    p.score = 95.5;
    printf("%d", p.age);
    modifyperson(&p);
    printf("%d", p.age);

    return 0;
}

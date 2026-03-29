#include <stdio.h>
int a;


int defn();

int defn() {
    int _retval;
_retval = 4;

    return _retval;
}


int main(void) {
a = defn();
printf("%d", a);

    return 0;
}

#include <stdio.h>
int a;
int sum;
int i;


int func();

int func() {
    int _retval;
    int b;
int a;

b = 7;
a = 1;
if ((a == b)) {
        a = (a + 1);
_retval = 1;
    } else {
        _retval = 0;
    }

    return _retval;
}


int main(void) {
a = 7;
sum = 0;
for (i = 0; i <= 99; i++) {
        if ((func() == 1)) {
        sum = (sum + 1);
    }
    }
printf("%d", a);
printf("%d", sum);

    return 0;
}

#include <stdio.h>
int m;
int n;
int l;

int i;
int x;

int a0[3];
int a1[3];
int a2[3];
int b0[3];
int b1[3];
int b2[3];
int c0[3];
int c1[3];
int c2[3];


int multiplymatrices();

int multiplymatrices() {
    int _retval;
    int i;

for (i = 0; i <= (m - 1); i++) {
        c0[i] = (((a0[0] * b0[i]) + (a0[1] * b1[i])) + (a0[2] * b2[i]));
c1[i] = (((a1[0] * b0[i]) + (a1[1] * b1[i])) + (a1[2] * b2[i]));
c2[i] = (((a2[0] * b0[i]) + (a2[1] * b1[i])) + (a2[2] * b2[i]));
    }
_retval = 0;

    return _retval;
}


int main(void) {
n = 3;
m = 3;
l = 3;
for (i = 0; i <= (m - 1); i++) {
        a0[i] = i;
a1[i] = i;
a2[i] = i;
b0[i] = i;
b1[i] = i;
b2[i] = i;
    }
multiplymatrices();
for (i = 0; i <= (n - 1); i++) {
        x = c0[i];
printf("%d", x);
    }
for (i = 0; i <= (n - 1); i++) {
        x = c1[i];
printf("%d", x);
    }
for (i = 0; i <= (n - 1); i++) {
        x = c2[i];
printf("%d", x);
    }

    return 0;
}

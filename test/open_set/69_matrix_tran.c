#include <stdio.h>
int m;
int l;
int n;

int a0[3];
int a1[3];
int a2[3];
int b0[3];
int b1[3];
int b2[3];
int c0[3];
int c1[3];
int c2[3];

int i;
int x;


int tran();

int tran() {
    int _retval;
    int i;

for (i = 0; i <= (m - 1); i++) {
        c1[2] = a2[1];
c2[1] = a1[2];
c0[1] = a1[0];
c0[2] = a2[0];
c1[0] = a0[1];
c2[0] = a0[2];
c1[1] = a1[1];
c2[2] = a2[2];
c0[0] = a0[0];
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
tran();
for (i = 0; i <= (n - 1); i++) {
        printf("%d", c0[i]);
    }
for (i = 0; i <= (n - 1); i++) {
        printf("%d", c1[i]);
    }
for (i = 0; i <= (n - 1); i++) {
        printf("%d", c2[i]);
    }

    return 0;
}

#include <stdio.h>
int a[100][100];
int b[100][100];
int res[100][100];

int n1;
int m1;
int n2;
int m2;
int i;
int j;
int k;


void matrix_multiply();
int getint();

void matrix_multiply() {
for (i = 1; i <= m1; i++) {
        for (j = 1; j <= n2; j++) {
        for (k = 1; k <= n1; k++) {
        res[(i) - 1][(j) - 1] = (res[(i) - 1][(j) - 1] + (a[(i) - 1][(k) - 1] * b[(k) - 1][(j) - 1]));
    }
    }
    }

}

int getint() {
    int _retval;
scanf("%d", &getint());

    return _retval;
}


int main(void) {
m1 = getint();
n1 = getint();
for (i = 1; i <= m1; i++) {
        for (j = 1; j <= n1; j++) {
        a[(i) - 1][(j) - 1] = getint();
    }
    }
m2 = getint();
n2 = getint();
for (i = 1; i <= m2; i++) {
        for (j = 1; j <= n2; j++) {
        b[(i) - 1][(j) - 1] = getint();
    }
    }
matrix_multiply();
for (i = 1; i <= m1; i++) {
        for (j = 1; j <= n2; j++) {
        printf("%d", res[(i) - 1][(j) - 1]);
    }
    }

    return 0;
}

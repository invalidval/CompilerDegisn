#include <stdio.h>
int n;

int i;
int tmp;

int iniarr[10];
int sortedarr[10];


int countingsort(int n);

int countingsort(int n) {
    int _retval;
    int countarr[10];

    int i;
    int j;
    int k;
    int jj;

    for (k = 0; k <= 9; k++) {
        countarr[k] = 0;
    }
    for (i = 0; i <= (n - 1); i++) {
        countarr[iniarr[i]] = (countarr[iniarr[i]] + 1);
    }
    for (k = 1; k <= 9; k++) {
        countarr[k] = (countarr[k] + countarr[(k - 1)]);
    }
    for (jj = 0; jj <= (n - 1); jj++) {
        j = (n - jj);
        countarr[iniarr[(j - 1)]] = (countarr[iniarr[(j - 1)]] - 1);
        sortedarr[countarr[iniarr[(j - 1)]]] = iniarr[(j - 1)];
    }
    _retval = 0;

    return _retval;
}


int main(void) {
    n = 10;
    iniarr[0] = 4;
    iniarr[1] = 3;
    iniarr[2] = 9;
    iniarr[3] = 2;
    iniarr[4] = 0;
    iniarr[5] = 1;
    iniarr[6] = 6;
    iniarr[7] = 5;
    iniarr[8] = 7;
    iniarr[9] = 8;
    countingsort(n);
    for (i = 0; i <= (n - 1); i++) {
        tmp = sortedarr[i];
        printf("%d", tmp);
    }

    return 0;
}

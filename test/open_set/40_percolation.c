#include <stdio.h>

int arr[110];
int n;
int m, a, b, k, loc, i;
int flag;

void init(int n);
int findfa(int a);
void mmerge(int a, int b);

void init(int n) {
for (i = 1; i <= ((n * n) + 1); i++) {
        arr[i] = (-1);
    }

}

int findfa(int a) {
    int _retval;
if ((arr[a] = a)) {
        _retval = a;
    } else {
        arr[a] = findfa(arr[a]);
_retval = arr[a];
    }

    return _retval;
}

void mmerge(int a, int b) {
m = findfa(a);
n = findfa(b);
if ((m <> n)) {
        arr[m] = n;
    }

}


int main(void) {
n = 4;
m = 10;
flag = false;
init(n)
k = ((n * n) + 1);
for (i = 0; i <= (m - 1); i++) {
        scanf("%d", &a);
scanf("%d", &b);
if ((flag = false)) {
        loc = ((n * (a - 1)) + b);
arr[loc] = loc;
if ((a = 1)) {
        arr[0] = 0;
mmerge(loc, 0)
    }
if ((a = n)) {
        arr[k] = k;
mmerge(loc, k)
    }
if (((b < n) and (arr[(loc + 1)] <> (-1)))) {
        mmerge(loc, (loc + 1))
    }
if (((b > 1) and (arr[(loc - 1)] <> (-1)))) {
        mmerge(loc, (loc - 1))
    }
if (((a < n) and (arr[(loc + n)] <> (-1)))) {
        mmerge(loc, (loc + n))
    }
if (((a > 1) and (arr[(loc - n)] <> (-1)))) {
        mmerge(loc, (loc - n))
    }
if ((((arr[0] <> (-1)) and (arr[k] <> (-1))) and (findfa(0) = findfa(k)))) {
        flag = true;
printf("%d", (i + 1));
    }
    }
    }
if ((flag = false)) {
        printf("%d", (-1));
    }

    return 0;
}

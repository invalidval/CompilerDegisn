#include <stdio.h>
int a;
int b;
int c;
int d;
int e;
int f;
int g;
int h;
int i;
int j;
int k;
int l;
int m;
int n;
int o;
int p;
int q;
int r;
int s;
int t;
int u;
int v;
int w;
int x;

int sum;
int sum1;
int sum2;
int sum3;


int foo();

int foo() {
    int _retval;
    int arr[16];

    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
    int h;
    int i;
    int j;
    int k;
    int l;
    int m;
    int n;
    int o;
    int p;

    int sum1;
    int sum2;

    arr[0] = 0;
    arr[1] = 1;
    arr[2] = 2;
    arr[3] = 3;
    arr[4] = 0;
    arr[5] = 1;
    arr[6] = 2;
    arr[7] = 3;
    arr[8] = 0;
    arr[9] = 1;
    arr[10] = 2;
    arr[11] = 3;
    arr[12] = 0;
    arr[13] = 1;
    arr[14] = 2;
    arr[15] = 3;
    a = 3;
    b = 7;
    c = 5;
    d = 6;
    e = 1;
    f = 0;
    g = 3;
    h = 5;
    i = 4;
    j = 2;
    k = 7;
    l = 9;
    m = 8;
    n = 1;
    o = 4;
    p = 6;
    sum1 = (((((((a + b) + c) + d) + e) + f) + g) + h);
    sum2 = (((((((i + j) + k) + l) + m) + n) + o) + p);
    _retval = ((sum1 + sum2) + arr[a]);

    return _retval;
}


int main(void) {
    a = 3;
    b = 7;
    c = 5;
    d = 6;
    e = 1;
    f = 0;
    g = 3;
    h = 5;
    i = 4;
    j = 2;
    k = 7;
    l = 9;
    m = 8;
    n = 1;
    o = 4;
    p = 6;
    sum1 = (((((((a + b) + c) + d) + e) + f) + g) + h);
    sum2 = (((((((i + j) + k) + l) + m) + n) + o) + p);
    sum1 = (sum1 + foo());
    q = 4;
    r = 7;
    s = 2;
    t = 5;
    u = 8;
    v = 0;
    w = 6;
    x = 3;
    sum2 = (sum2 + foo());
    a = i;
    b = j;
    c = k;
    d = l;
    e = m;
    f = n;
    g = o;
    h = p;
    sum3 = (((((((q + r) + s) + t) + u) + v) + w) + x);
    sum = ((sum1 + sum2) + sum3);
    printf("%d", sum);

    return 0;
}

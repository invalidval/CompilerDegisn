#include <stdio.h>
int n;
int m;

int ret;
int i;

int u[1005];
int v[1005];
int c[1005];
int fa[1005];


int find(int x);
int same(int x, int y);
int prim();

int find(int x) {
    int _retval;
    int asdf;

if ((x == fa[x])) {
        _retval = x;
    } else {
        asdf = find(fa[x]);
fa[x] = asdf;
_retval = asdf;
    }

    return _retval;
}

int same(int x, int y) {
    int _retval;
x = find(x);
y = find(y);
if ((x == y)) {
        _retval = 1;
    } else {
        _retval = 0;
    }

    return _retval;
}

int prim() {
    int _retval;
    int i;
int j;
int t;
int res;

for (i = 0; i <= (m - 1); i++) {
        for (j = (i + 1); j <= (m - 1); j++) {
        if ((c[i] > c[j])) {
        t = u[i];
u[i] = u[j];
u[j] = t;
t = v[i];
v[i] = v[j];
v[j] = t;
t = c[i];
c[i] = c[j];
c[j] = t;
    }
    }
    }
for (i = 1; i <= n; i++) {
        fa[i] = i;
    }
res = 0;
for (i = 0; i <= (m - 1); i++) {
        if ((same(u[i], v[i]) == 0)) {
        res = (res + c[i]);
fa[find(u[i])] = v[i];
    }
    }
_retval = res;

    return _retval;
}


int main(void) {
ret = 0;
scanf("%d", &n);
scanf("%d", &m);
for (i = 0; i <= (m - 1); i++) {
        scanf("%d", &u[i]);
scanf("%d", &v[i]);
scanf("%d", &c[i]);
    }
ret = prim();
printf("%d", ret);

    return 0;
}

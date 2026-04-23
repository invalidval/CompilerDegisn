#include <stdio.h>
int n;
int ret;
int i;
int t;

int x[100005];
int a[100005];
int b[100005];
int c[100005];

int cnt[400020];


void sorta();
void sortb();
void sortc();

void sorta() {
    int i;
    int j;
    int t;

    for (i = 0; i <= (n - 1); i++) {
        for (j = (i + 1); j <= (n - 1); j++) {
            if ((a[i] > a[j])) {
                t = a[i];
                a[i] = a[j];
                a[j] = t;
            }
        }
    }

}

void sortb() {
    int mx;
    int i;
    int j;
    int now;

    mx = (-100);
    for (i = 0; i <= (n - 1); i++) {
        cnt[b[i]] = (cnt[b[i]] + 1);
        if ((b[i] > mx)) {
            mx = b[i];
        }
    }
    now = 0;
    for (i = 0; i <= mx; i++) {
        for (j = 0; j <= (cnt[i] - 1); j++) {
            b[now] = i;
            now = (now + 1);
        }
    }

}

void sortc() {
    int i;
    int j;
    int id;

    for (i = 0; i <= (n - 1); i++) {
        id = i;
        for (j = (i + 1); j <= (n - 1); j++) {
            if ((c[j] < c[id])) {
                id = j;
            }
        }
        t = c[i];
        c[i] = c[id];
        c[id] = t;
    }

}


int main(void) {
    ret = 0;
    scanf("%d", &n);
    for (i = 0; i <= (n - 1); i++) {
        scanf("%d", &a[i]);
        b[i] = a[i];
        c[i] = b[i];
    }
    sorta();
    sortb();
    sortc();
    for (i = 0; i <= (n - 1); i++) {
        b[i] = (b[i] - a[i]);
        c[i] = ((c[i] - b[i]) - a[i]);
    }
    for (i = 0; i <= (n - 1); i++) {
        if ((b[i] != 0)) {
            ret = 1;
        }
        if ((c[i] != 0)) {
            ret = 2;
        }
    }
    ret = 0;
    printf("%d", ret);

    return 0;
}

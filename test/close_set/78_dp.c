#include <stdio.h>
int t[1005][2];

int dp[1005][35];

int tt;
int w;
int x;
int i;
int j;
int res;


int getint();

int getint() {
    int _retval;
    int n;

    scanf("%d", &n);
    _retval = n;

    return _retval;
}


int main(void) {
    for (i = 0; i <= 1004; i++) {
        t[i][0] = 0;
        t[i][1] = 0;
        for (j = 0; j <= 34; j++) {
            dp[i][j] = 0;
        }
    }
    tt = getint();
    w = getint();
    for (i = 1; i <= tt; i++) {
        x = getint();
        t[i][(x % 2)] = 1;
        dp[i][0] = (dp[(i - 1)][0] + t[i][1]);
    }
    for (i = 1; i <= tt; i++) {
        for (j = 1; j <= w; j++) {
            if (((dp[(i - 1)][j] + t[i][((j + 1) % 2)]) > (dp[(i - 1)][(j - 1)] + t[i][((j + 1) % 2)]))) {
                dp[i][j] = (dp[(i - 1)][j] + t[i][((j + 1) % 2)]);
            } else {
                dp[i][j] = (dp[(i - 1)][(j - 1)] + t[i][((j + 1) % 2)]);
            }
        }
    }
    res = 0;
    for (j = 0; j <= w; j++) {
        if ((res < dp[tt][j])) {
            res = dp[tt][j];
        }
    }
    printf("%d", res);

    return 0;
}

#include <stdio.h>
int store[30];

int n;
int m;

int graph[30][30];

int edges[600][2];

int i;
int ret;


int is_clique(int num);
int maxcliques(int i, int k);

int is_clique(int num) {
    int _retval;
    int i;
    int j;

    _retval = 1;
    for (i = 1; i <= (num - 1); i++) {
        for (j = (i + 1); j <= (num - 1); j++) {
            if ((graph[store[i]][store[j]] == 0)) {
                _retval = 0;
            }
        }
    }

    return _retval;
}

int maxcliques(int i, int k) {
    int _retval;
    int max_;
    int j;
    int tmp;

    max_ = 0;
    for (j = 1; j <= n; j++) {
        store[k] = j;
        if ((is_clique((k + 1)) != 0)) {
            if ((k > max_)) {
                max_ = k;
            }
            tmp = maxcliques(j, (k + 1));
            if ((tmp > max_)) {
                max_ = tmp;
            }
        }
    }
    _retval = max_;

    return _retval;
}


int main(void) {
    scanf("%d", &n);
    scanf("%d", &m);
    for (i = 0; i <= (m - 1); i++) {
        scanf("%d", &edges[i][0]);
        scanf("%d", &edges[i][1]);
    }
    for (i = 0; i <= (m - 1); i++) {
        graph[edges[i][0]][edges[i][1]] = 1;
        graph[edges[i][1]][edges[i][0]] = 1;
    }
    ret = maxcliques(0, 1);
    printf("%d", ret);

    return 0;
}

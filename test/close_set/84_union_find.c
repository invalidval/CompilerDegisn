#include <stdio.h>
int parent[1005];

int n;
int m;
int i;
int p;
int q;
int clusters;


int getint();
int find(int root);
void merge(int p, int q);

int getint() {
    int _retval;
scanf("%d", &getint());

    return _retval;
}

int find(int root) {
    int _retval;
if ((parent[root] == root)) {
        _retval = root;
    } else {
        parent[root] = find(parent[root]);
_retval = parent[root];
    }

    return _retval;
}

void merge(int p, int q) {
    int root_p;
int root_q;

root_p = find(p);
root_q = find(q);
if ((root_p != root_q)) {
        parent[root_q] = root_p;
    }

}


int main(void) {
n = getint();
m = getint();
for (i = 0; i <= (n - 1); i++) {
        parent[i] = i;
    }
for (i = 0; i <= (m - 1); i++) {
        p = getint();
q = getint();
merge(p, q);
    }
clusters = 0;
for (i = 0; i <= (n - 1); i++) {
        if ((parent[i] == i)) {
        clusters = (clusters + 1);
    }
    }
printf("%d", clusters);
scanf("%d", &clusters);

    return 0;
}

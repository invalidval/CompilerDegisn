#include <stdio.h>
const int space = 32;
int arr[1000];

int n;
int k;
int i;
int low;
int high;


void swap(int a, int b);
int findpivot(int start, int endindex);
void findsmallest(int low, int high, int k, int n);
int getint();

void swap(int a, int b) {
    int tmp;

    tmp = arr[a];
    arr[a] = arr[b];
    arr[b] = tmp;

}

int findpivot(int start, int endindex) {
    int _retval;
    int pivot;
    int pindex;
    int i;

    pivot = arr[endindex];
    pindex = start;
    for (i = start; i <= (endindex - 1); i++) {
        if ((arr[i] <= pivot)) {
            swap(i, pindex);
            pindex = (pindex + 1);
        }
    }
    swap(pindex, endindex);
    _retval = pindex;

    return _retval;
}

void findsmallest(int low, int high, int k, int n) {
    int pindex;
    int i;

    if ((low != high)) {
        pindex = findpivot(low, high);
        if ((k == pindex)) {
            for (i = 0; i <= (pindex - 1); i++) {
                printf("%d", arr[i]);
            }
        } else {
            if ((k < pindex)) {
                findsmallest(low, (pindex - 1), k, n);
            } else {
                findsmallest((pindex + 1), high, k, n);
            }
        }
    }

}

int getint() {
    int _retval;
    int n;

    scanf("%d", &n);
    _retval = n;

    return _retval;
}


int main(void) {
    n = getint();
    k = getint();
    for (i = 0; i <= (n - 1); i++) {
        arr[i] = getint();
    }
    low = 0;
    high = (n - 1);
    findsmallest(low, high, k, n);

    return 0;
}

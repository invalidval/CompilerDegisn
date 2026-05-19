#include <stdio.h>
int arr[10];

int i;


int main(void) {
    arr[(1) - 1] = 5;
    arr[(5) - 1] = 10;
    i = (arr[(1) - 1] + arr[(5) - 1]);

    return 0;
}

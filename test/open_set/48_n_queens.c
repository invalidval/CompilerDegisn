#include <stdio.h>

const char newline = ',';
const char blank = ' ';
int ans[50];
int sum, n;
int row[50];
int line1[50];
int line2[100];
int k, i;

void printans();
void f(int step);

void printans() {
sum = (sum + 1);
for (i = 1; i <= n; i++) {
        printf("%d", ans[i]);
if ((i = n)) {
        printf("%c", newline);
    } else {
        printf("%c", blank);
    }
    }

}

void f(int step) {
for (i = 1; i <= n; i++) {
        if ((((row[i] <> 1) and (line1[(step + i)] = 0)) and (line2[((n + step) - i)] = 0))) {
        ans[step] = i;
if ((step = n)) {
        printans()
    }
row[i] = 1;
line1[(step + i)] = 1;
line2[((n + step) - i)] = 1;
f((step + 1))
row[i] = 0;
line1[(step + i)] = 0;
line2[((n + step) - i)] = 0;
    }
    }

}


int main(void) {
sum = 0;
scanf("%d", &k);
for (i = 1; i <= k; i++) {
        scanf("%d", &n);
f(1)
    }
printf("%d", sum);

    return 0;
}

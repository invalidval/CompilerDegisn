#include <stdio.h>
const int space = 32;
const char ne = 'Not exist';
int graph[4][4];

int color[4];

int i;
int m;


void printsolution();
void printmessage();
int issafe();
int graphcoloring(int m, int i);

void printsolution() {
    int i;

for (i = 0; i <= 3; i++) {
        printf("%d", color[i]);
    }

}

void printmessage() {
printf("%c", ne);

}

int issafe() {
    int _retval;
    int i;
int j;

_retval = 1;
for (i = 0; i <= 3; i++) {
        for (j = (i + 1); j <= 3; j++) {
        if (((graph[i][j] != 0) && (color[j] == color[i]))) {
        _retval = 0;
    }
    }
    }

    return _retval;
}

int graphcoloring(int m, int i) {
    int _retval;
    int j;

    int foundsolution;

foundsolution = 0;
if ((i == 4)) {
        if ((issafe() != 0)) {
        printsolution();
_retval = 1;
foundsolution = 1;
    }
    } else {
        for (j = 1; j <= m; j++) {
        color[i] = j;
if ((graphcoloring(m, (i + 1)) != 0)) {
        foundsolution = 1;
break();
    }
color[i] = 0;
    }
    }
if (foundsolution) {
        _retval = 1;
    } else {
        _retval = 0;
    }

    return _retval;
}


int main(void) {
graph[0][0] = 0;
graph[0][1] = 1;
graph[0][2] = 1;
graph[0][3] = 1;
graph[1][0] = 1;
graph[1][1] = 0;
graph[1][2] = 1;
graph[1][3] = 0;
graph[2][0] = 1;
graph[2][1] = 1;
graph[2][2] = 0;
graph[2][3] = 1;
graph[3][0] = 1;
graph[3][1] = 0;
graph[3][2] = 1;
graph[3][3] = 0;
m = 3;
for (i = 0; i <= 3; i++) {
        color[i] = 0;
    }
if ((graphcoloring(m, 0) == 0)) {
        printmessage();
    }

    return 0;
}

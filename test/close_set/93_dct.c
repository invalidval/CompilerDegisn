#include <stdio.h>
const float pi = 3.14159265359;
const float two_pi = 6.28318530718;
const float epsilon = 0.000001;
float test_block[8][8];
float test_dct[8][8];
float test_idct[8][8];

int dim_x;
int dim_y;
int i;
int j;


float my_fabs(int x);
float p(int x);
float my_sin_impl(int x);
float my_sin(int x);
float my_cos(int x);
void write_mat(int n, int m);
void write_mat2(int n, int m);
void dct(int n, int m);
void idct(int n, int m);

float my_fabs(int x) {
    float _retval;
if ((x > 0.0)) {
        _retval = x;
    } else {
        _retval = (-x);
    }

    return _retval;
}

float p(int x) {
    float _retval;
_retval = ((3 * x) - (((4 * x) * x) * x));

    return _retval;
}

float my_sin_impl(int x) {
    float _retval;
if ((my_fabs(x) <= epsilon)) {
        _retval = x;
    } else {
        _retval = p(my_sin_impl((x / 3.0)));
    }

    return _retval;
}

float my_sin(int x) {
    float _retval;
    int xx;

if (((x > two_pi) || (x < (-two_pi)))) {
        xx = 1;
x = (x - 1.0);
    }
if ((x > pi)) {
        x = (x - two_pi);
    }
if ((x < (-pi))) {
        x = (x + two_pi);
    }
_retval = my_sin_impl(x);

    return _retval;
}

float my_cos(int x) {
    float _retval;
_retval = my_sin(((x * pi) / 2));

    return _retval;
}

void write_mat(int n, int m) {
    int i;
int j;

for (i = 0; i <= (n - 1); i++) {
        printf("%f", test_dct[i][0]);
for (j = 1; j <= (m - 1); j++) {
        printf("%f", test_dct[i][j]);
    }
    }

}

void write_mat2(int n, int m) {
    int i;
int j;

for (i = 0; i <= (n - 1); i++) {
        printf("%f", test_idct[i][0]);
for (j = 1; j <= (m - 1); j++) {
        printf("%f", test_idct[i][j]);
    }
    }

}

void dct(int n, int m) {
    int u;
int v;
int i;
int j;

for (u = 0; u <= (n - 1); u++) {
        for (v = 0; v <= (m - 1); v++) {
        test_dct[u][v] = 0;
for (i = 0; i <= (n - 1); i++) {
        for (j = 0; j <= (m - 1); j++) {
        test_dct[u][v] = (test_dct[u][v] + ((test_block[i][j] * my_cos((((pi / n) * (i + (1.0 / 2.0))) * u))) * my_cos((((pi / m) * (i + (1.0 / 2.0))) * v))));
    }
    }
    }
    }

}

void idct(int n, int m) {
    int u;
int v;
int i;
int j;

for (u = 0; u <= (n - 1); u++) {
        for (v = 0; v <= (m - 1); v++) {
        test_idct[u][v] = ((1 / 4.0) * test_dct[0][0]);
for (i = 1; i <= (n - 1); i++) {
        test_idct[u][v] = (test_idct[u][v] + ((1 / 2.0) * test_dct[i][0]));
    }
for (j = 1; j <= (m - 1); j++) {
        test_idct[u][v] = (test_idct[u][v] + ((1 / 2.0) * test_dct[0][j]));
    }
for (i = 1; i <= (n - 1); i++) {
        for (j = 1; j <= (m - 1); j++) {
        test_idct[u][v] = (test_idct[u][v] + ((test_dct[i][j] * my_cos((((pi / n) * (u + (1.0 / 2.0))) * i))) * my_cos((((pi / m) * (v + (1.0 / 2.0))) * j))));
    }
    }
test_idct[u][v] = ((((test_idct[u][v] * 2.0) / n) * 2.0) / m);
    }
    }

}


int main(void) {
dim_x = 0;
dim_y = 0;
scanf("%d", &dim_x);
scanf("%d", &dim_y);
for (i = 0; i <= (dim_x - 1); i++) {
        for (j = 0; j <= (dim_y - 1); j++) {
        scanf("%f", &test_block[i][j]);
    }
    }
dct(dim_x, dim_y);
write_mat(dim_x, dim_y);
idct(dim_x, dim_y);
write_mat2(dim_x, dim_y);

    return 0;
}

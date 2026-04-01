#include <stdio.h>
const float e = 2.1718281828459045;
const char split = '--';
float x;
float y;

int num;
int i;


float my_fabs(int x);
float my_pow(int a, int n);
float my_sqrt(int x);
float f1(int x);
float f2(int x);
float simpson(int a, int b, int flag);
float asr5(int a, int b, int eps, int aa, int flag);
float asr4(int a, int b, int eps, int flag);
float eee(int x);
float my_exp(int x);
float my_ln(int x);
float my_log(int a, int n);
float my_powf(int a, int x);
void putfloat(int f);
float getfloat();

float my_fabs(int x) {
    float _retval;
if ((x > 0)) {
        _retval = x;
    } else {
        _retval = (-x);
    }

    return _retval;
}

float my_pow(int a, int n) {
    float _retval;
    int i;

    float res;

if ((n < 0)) {
        _retval = (1 / my_pow(a, (-n)));
    } else {
        res = 1.0;
for (i = 0; i <= (n - 1); i++) {
        res = (res * a);
    }
_retval = res;
    }

    return _retval;
}

float my_sqrt(int x) {
    float _retval;
    float t;

    int c;

if ((x > 100)) {
        _retval = (10.0 * my_sqrt((x / 100)));
    } else {
        t = (((x / 8) + 0.5) + ((2 * x) / (4 + x)));
for (c = 0; c <= 9; c++) {
        t = ((t + (x / t)) / 2);
    }
_retval = t;
    }

    return _retval;
}

float f1(int x) {
    float _retval;
_retval = (1 / x);

    return _retval;
}

float f2(int x) {
    float _retval;
_retval = (1 / my_sqrt((1 - (x * x))));

    return _retval;
}

float simpson(int a, int b, int flag) {
    float _retval;
    float c;

c = (a + ((b - a) / 2));
_retval = 0;
if ((flag == 1)) {
        _retval = ((((f1(a) + (4 * f1(c))) + f1(b)) * (b - a)) / 6);
    } else {
        _retval = ((((f2(a) + (4 * f2(c))) + f2(b)) * (b - a)) / 6);
    }

    return _retval;
}

float asr5(int a, int b, int eps, int aa, int flag) {
    float _retval;
    float c;
float l;
float r;

c = (a + ((b - a) / 2));
l = simpson(a, c, flag);
r = simpson(c, b, flag);
if ((my_fabs(((l + r) - aa)) <= (15 * eps))) {
        _retval = ((l + r) + (((l + r) - aa) / 15.0));
    } else {
        _retval = (asr5(a, c, (eps / 2), l, flag) + asr5(c, b, (eps / 2), r, flag));
    }

    return _retval;
}

float asr4(int a, int b, int eps, int flag) {
    float _retval;
_retval = asr5(a, b, eps, simpson(a, b, flag), flag);

    return _retval;
}

float eee(int x) {
    float _retval;
    float ee;

if ((x > 0.001)) {
        ee = eee((x / 2));
_retval = (ee * ee);
    } else {
        _retval = (((((1 + x) + ((x * x) / 2)) + (my_pow(x, 3) / 6)) + (my_pow(x, 4) / 24)) + (my_pow(x, 5) / 120));
    }

    return _retval;
}

float my_exp(int x) {
    float _retval;
    float e1;
float e2;

    int n;

if ((x < 0)) {
        _retval = (1 / my_exp((-x)));
    } else {
        n = 1;
x = (x - 1.0);
e1 = my_pow(e, n);
e2 = eee(x);
_retval = (e1 * e2);
    }

    return _retval;
}

float my_ln(int x) {
    float _retval;
_retval = asr4(1, x, 0.00000001, 1);

    return _retval;
}

float my_log(int a, int n) {
    float _retval;
_retval = (my_ln(n) / my_ln(a));

    return _retval;
}

float my_powf(int a, int x) {
    float _retval;
_retval = my_exp((x * my_ln(a)));

    return _retval;
}

void putfloat(int f) {
printf("%f", f);

}

float getfloat() {
    float _retval;
scanf("%f", &getfloat());

    return _retval;
}


int main(void) {
num = 2;
for (i = 0; i <= (num - 1); i++) {
        x = getfloat();
y = getfloat();
putfloat(my_fabs(x));
putfloat(my_pow(x, 2));
putfloat(my_sqrt(x));
putfloat(my_exp(x));
if ((x > 0.0)) {
        putfloat(my_ln(x));
    } else {
        printf("%c", split);
    }
if (((x > 0.0) && (y > 0.0))) {
        putfloat(my_log(x, y));
    } else {
        printf("%c", split);
    }
if ((x > 0.0)) {
        putfloat(my_powf(x, y));
    } else {
        printf("%c", split);
    }
    }
scanf("%d", &num);

    return 0;
}

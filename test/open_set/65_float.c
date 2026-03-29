#include <stdio.h>
const float radius = 5.5;
const float pi = 03.141595653589793;
const float eps = 0.000001;
const float eval1 = 95.033188;
const int conv1 = 233;
const int max = 1000000000;
const float two = 2.9;
const int three = 3;
const int five = 5;
const char e = 'e';
const char o = 'o';
int p;

float arr[10];

float input;
float area;
float area_trunc;


float float_abs(int x);
float circle_area(int radius);
int float_eq(int a, int b);
void error();
void ok();
void assert(int cond);

float float_abs(int x) {
    float _retval;
if ((x < 0)) {
        _retval = (-x);
    } else {
        _retval = x;
    }

    return _retval;
}

float circle_area(int radius) {
    float _retval;
_retval = ((((pi * radius) * radius) + ((radius * radius) * pi)) / 2);

    return _retval;
}

int float_eq(int a, int b) {
    int _retval;
if ((float_abs((a - b)) < eps)) {
        _retval = 1;
    } else {
        _retval = 0;
    }

    return _retval;
}

void error() {
printf("%c", e);

}

void ok() {
printf("%c", o);

}

void assert(int cond) {
if ((cond == 0)) {
        error();
    } else {
        ok();
    }

}


int main(void) {
assert(float_eq(circle_area(5), circle_area(five)));
if ((1.5 != 0.0)) {
        ok();
    }
if ((!(3.3 == 0.0))) {
        ok();
    }
if (((0.0 != 0.0) && (3 != 0.0))) {
        error();
    }
if (((0 != 0.0) || (0.3 != 0.0))) {
        ok();
    }
p = 0;
arr[0] = 1.0;
arr[1] = 2.0;
input = 0.520;
area = ((pi * input) * input);
area_trunc = circle_area(0);
arr[p] = (arr[p] + input);
printf("%f", area);
printf("%f", area_trunc);
printf("%f", arr[0]);

    return 0;
}

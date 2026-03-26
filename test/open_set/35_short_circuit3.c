#include <stdio.h>

const char aa = 'A';
const char bb = 'B';
const char cc = 'C';
const char dd = 'D';
const char e = 'E';
const char f = 'F';
const char g = 'G';
const char h = 'H';
const char i = 'I';
const char j = 'J';
const char k = 'K';
const int c = 1;
int a, b, d;
int i0, i1, i2, i3, i4;

int set_a(int val);
int set_b(int val);
int set_d(int val);

int set_a(int val) {
    int _retval;
a = val;
_retval = val;

    return _retval;
}

int set_b(int val) {
    int _retval;
b = val;
_retval = val;

    return _retval;
}

int set_d(int val) {
    int _retval;
d = val;
_retval = val;

    return _retval;
}


int main(void) {
a = 2;
b = 3;
if (((set_a(0) <> 0) and (set_b(1) <> 0))) {
    }
printf("%d", a);
printf("%d", b);
a = 2;
b = 3;
if (((set_a(0) <> 0) and (set_b(1) <> 0))) {
    }
printf("%d", a);
printf("%d", b);
d = 2;
if (((c >= 1) and (set_d(3) <> 0))) {
    }
printf("%d", d);
if (((c <= 1) or (set_d(4) <> 0))) {
    }
printf("%d", d);
if ((16 >= (3 - (2 + 1)))) {
        printf("%c", aa);
    }
if (((25 - 7) <> (36 - (6 * 3)))) {
        printf("%c", bb);
    }
if ((1 <> (7 % 2))) {
        printf("%c", cc);
    }
if ((3 <= 4)) {
        printf("%c", dd);
    }
if ((0 <> 0)) {
        printf("%c", e);
    }
if ((1 <> 0)) {
        printf("%c", f);
    }
i0 = 0;
i1 = 1;
i2 = 2;
i3 = 3;
i4 = 4;
if (((i0 <> 0) or (i1 <> 0))) {
        printf("%c", g);
    }
if (((i0 >= i1) or (i1 <= i0))) {
        printf("%c", h);
    }
if (((i2 >= i1) and (i4 <> i3))) {
        printf("%c", i);
    }
if ((((i0 = 0) and (i3 < i3)) or (i4 >= i4))) {
        printf("%c", j);
    }
if (((i0 = 0) or ((i3 < i3) and (i4 >= i4)))) {
        printf("%c", k);
    }

    return 0;
}

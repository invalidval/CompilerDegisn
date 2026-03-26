#include <stdio.h>

int arr[32][2];
int i;

int param32_rec(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15, int a16, int a17, int a18, int a19, int a20, int a21, int a22, int a23, int a24, int a25, int a26, int a27, int a28, int a29, int a30, int a31, int a32);
int param32_arr();
int param16(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15, int a16);
int getint(int *index);

int param32_rec(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15, int a16, int a17, int a18, int a19, int a20, int a21, int a22, int a23, int a24, int a25, int a26, int a27, int a28, int a29, int a30, int a31, int a32) {
    int _retval;
if ((a1 = 0)) {
        _retval = a2;
    } else {
        _retval = param32_rec((a1 - 1), ((a2 + a3) % 998244353), a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, 0);
    }

    return _retval;
}

int param32_arr() {
    int _retval;
_retval = (arr[0][0] + arr[0][1]);
_retval = ((sum + arr[1][0]) + arr[1][1]);
_retval = ((sum + arr[2][0]) + arr[2][1]);
_retval = ((sum + arr[3][0]) + arr[3][1]);
_retval = ((sum + arr[4][0]) + arr[4][1]);
_retval = ((sum + arr[5][0]) + arr[5][1]);
_retval = ((sum + arr[6][0]) + arr[6][1]);
_retval = ((sum + arr[7][0]) + arr[7][1]);
_retval = ((sum + arr[8][0]) + arr[8][1]);
_retval = ((sum + arr[9][0]) + arr[9][1]);
_retval = ((sum + arr[10][0]) + arr[10][1]);
_retval = ((sum + arr[11][0]) + arr[11][1]);
_retval = ((sum + arr[12][0]) + arr[12][1]);
_retval = ((sum + arr[13][0]) + arr[13][1]);
_retval = ((sum + arr[14][0]) + arr[14][1]);
_retval = ((sum + arr[15][0]) + arr[15][1]);
_retval = ((sum + arr[16][0]) + arr[16][1]);
_retval = ((sum + arr[17][0]) + arr[17][1]);
_retval = ((sum + arr[18][0]) + arr[18][1]);
_retval = ((sum + arr[19][0]) + arr[19][1]);
_retval = ((sum + arr[20][0]) + arr[20][1]);
_retval = ((sum + arr[21][0]) + arr[21][1]);
_retval = ((sum + arr[22][0]) + arr[22][1]);
_retval = ((sum + arr[23][0]) + arr[23][1]);
_retval = ((sum + arr[24][0]) + arr[24][1]);
_retval = ((sum + arr[25][0]) + arr[25][1]);
_retval = ((sum + arr[26][0]) + arr[26][1]);
_retval = ((sum + arr[27][0]) + arr[27][1]);
_retval = ((sum + arr[28][0]) + arr[28][1]);
_retval = ((sum + arr[29][0]) + arr[29][1]);
_retval = ((sum + arr[30][0]) + arr[30][1]);
_retval = ((sum + arr[31][0]) + arr[31][1]);
_retval = sum;

    return _retval;
}

int param16(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15, int a16) {
    int _retval;
arr2[0] = a1;
arr2[1] = a2;
arr2[2] = a3;
arr2[3] = a4;
arr2[4] = a5;
arr2[5] = a6;
arr2[6] = a7;
arr2[7] = a8;
arr2[8] = a9;
arr2[9] = a10;
arr2[10] = a11;
arr2[11] = a12;
arr2[12] = a13;
arr2[13] = a14;
arr2[14] = a15;
arr2[15] = a16;
_retval = param32_rec(arr2[0], arr2[1], arr2[2], arr2[3], arr2[4], arr2[5], arr2[6], arr2[7], arr2[8], arr2[9], arr2[10], arr2[11], arr2[12], arr2[13], arr2[14], arr2[15], a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);

    return _retval;
}

int getint(int *index) {
    int _retval;
input[0] = 17;
input[1] = 13;
input[2] = 80;
input[3] = 55;
input[4] = 81;
input[5] = 91;
input[6] = 95;
input[7] = 58;
input[8] = 13;
input[9] = 5;
input[10] = 63;
input[11] = 19;
input[12] = 54;
input[13] = 45;
input[14] = 67;
input[15] = 63;
index = (index + 1);
_retval = input[(index - 1)];

    return _retval;
}


int main(void) {
i = 0;
arr[0][0] = param16(getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i), getint(&i));
arr[0][1] = 8848;
for (i = 1; i <= 31; i++) {
        arr[i][0] = (arr[(i - 1)][1] - 1);
arr[i][1] = (arr[(i - 1)][0] - 2);
    }
printf("%d", param32_arr);

    return 0;
}

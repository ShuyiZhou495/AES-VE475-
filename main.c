//
// Created by 周舒意 on 2020/6/8.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct matrix{
    int row;
    int col;
    unsigned char* array;
};

// Matrix added for Sbox.
unsigned char toAdd = 99;

// Mtrix multiplied for Sbox
unsigned char SboxMul[8] = {241, 227, 199, 143, 31, 62, 124, 248};

// C(x) in the Mix Column Layer
unsigned char toMix[4][4] = {
        {2, 3, 1, 1},
        {1, 2, 3, 1},
        {1, 1, 2, 3},
        {3, 1, 1, 2}
};

// P(x) used in AES
unsigned char Px = 27;

// Used in transform key.
unsigned char r = 2;

// Given a decimal, return the inverse of it.
unsigned char findInverse(unsigned char a);

// Given a single byte, with decimal representation n, return decimal of the subbyte version.
unsigned char SubSingleByte(  unsigned char n);

// Subbytes layer. Change the input matrix.
void SubBytes(struct matrix *mat);

// ShiftRow layer, the matrix will be changed.
void shiftRows(struct matrix *mat);

// The multiply method in AES, return decimal.
unsigned char con_multiply(unsigned char left, unsigned char right);

// Input a decimal number, times this number by the matrix and xor by the matrix to generate Sbox number.
unsigned char generateSBox(unsigned char a);

// Mix column layer
void mixColumn(struct matrix *mat);

// Generate all the keys
void change_key_matrix(struct matrix *key);

// Transform the columns that mod 4 equals 0.
void trans_key(unsigned char* K, int round);

// Add round key layer.
void addRoundKey(struct matrix *mat, const struct matrix * key, unsigned char round);

int main(int argc, char *argv[])
{
    int i,j, round;
// here are the plain text that has a bytes.
    unsigned char a[4][4] = {{219, 1, 83, 69}, {19, 1, 1, 1}, {83, 1, 10, 11}, {69, 1, 14, 15}};
    if(argc==3) {
        for (j = 0; j < 4; j++) {
            for (i = 0; i < 4; i++) a[i][j] = argv[1][i + j * 4];
        }
    }
    struct matrix plain = {.array=(unsigned char*)a, .row = 4, .col = 4};

// here are the keys(128bits).
    unsigned char k[4][4];
    for(j=0;j<4;j++){
        for(i=0;i<4;i++) k[i][j]=(unsigned char)argv[2][i+j*4];
    }
    struct matrix key = {.array=(unsigned char*)malloc(4*44), .row=4, .col=44};
    for(i=0; i<4; i++){
        for(j=0;j<4;j++){
            *(key.array + i*4 +j) = k[i][j];
        }
    }
    change_key_matrix(&key);

// start encrypt
    addRoundKey(&plain, &key, 0);
    for(round =1; round<10; round++) {
        SubBytes(&plain);
        shiftRows(&plain);
        mixColumn(&plain);
        addRoundKey(&plain, &key, round);
    }
    SubBytes(&plain);
    shiftRows(&plain);
    addRoundKey(&plain, &key, 10);
    for(i=0; i<4; i++) {
        for(j=0;j<4;j++) {
            printf("%d ", *(plain.array + 4*i +j));
        }
        printf("\n");
    }
    printf("\n");
    for(j=0; j<4; j++) {
        for(i=0;i<4;i++) {
            printf("%c ", *(plain.array + 4*i +j));
        }
    }
    printf("\n");
    return 0;
}

unsigned char con_multiply(unsigned char left, unsigned char right)
{
    unsigned char result=0, temp=0;
    int i, j, n;
    for(i=128,j=7;i>=1;i/=2,j--){
        if(right/i){
            temp=left;
            for(n=0;n<j;n++){
                if(temp>=128){
                    temp=2*temp;
                    temp = temp^Px;
                }
                else temp*=2;
            }
            result = result ^ temp;
            right -= i;
        }
    }
    return result;
}

unsigned char findInverse(unsigned char a)
{
    int i;
    for(i=0;i<256;i++){
        if(con_multiply(a,i)==1) return i;
    }
    return 0;
}

bool ifOddOnes(unsigned char a)
{
    bool flag = false;
    int i;
    for(i=128;a>0;i/=2){
        if(a/i){
            flag = !flag;
            a -= i;
        }
    }
    return flag;
}

unsigned char generateSBox(unsigned char a)
{
    int i, j;
    unsigned char result=0, temp;
    for (i=0,j=1; i<8; i++, j*=2){
        temp = SboxMul[i] & a;
        result += ifOddOnes(temp) * j;
    }
    return result^toAdd;
}


unsigned char SubSingleByte(unsigned char n)
{
    unsigned char inverse=0;
    if(n) inverse = findInverse(n);
    return generateSBox(inverse);
}

void SubBytes(struct matrix *mat)
{
    int i, j;
    for(i = 0; i<mat->row;i++){
        for (j=0;j<mat->col;j++)
            *(mat->array+i*mat->col + j) = SubSingleByte(*(mat->array+i*mat->col + j));
    }
}

void shiftRows(struct matrix *mat)
{
    int i;
    for(i = 1; i <= 3; i++){
        unsigned char temp[4]  = {
                *(mat->array + i*mat->col),
                *(mat->array + i*mat->col + 1),
                *(mat->array + i*mat->col + 2),
                *(mat->array + i*mat->col + 3)
        };
        *(mat->array + i*mat->col) = temp[i];
        *(mat->array + i*mat->col + 1) = temp[(i+1)%4];
        *(mat->array + i*mat->col + 2) = temp[(i+2)%4];
        *(mat->array + i*mat->col + 3) = temp[(i+3)%4];
    }
}

void mixColumn(struct matrix *mat)
{
    unsigned char temp[4], i, j, k;
    for (i=0;i<4;i++){
        for(j=0; j<4; j++){
            temp[j] = 0;
            for(k=0;k<4;k++){
                temp[j]=con_multiply(toMix[j][k], *(mat->array + k*4 + i)) ^ temp[j];
            }
        }
        for(j=0;j<4;j++) *(mat->array + j*4 + i) = temp[j];
    }
}

void trans_key(unsigned char* K, int round)
{
    int i;
    unsigned char rx=1;
    for(i=1; i<round;i++) rx = con_multiply(rx, r);
    unsigned char temp = *(K);
    // Cycally shift
    for(i=0;i<3;i++) *(K+i) = *(K+i+1);
    *(K+3) = temp;
    // subbytes
    for(i=0;i<4;i++) *(K) = SubSingleByte(*(K));
    // xor a and r
    *(K) = *(K)^rx;
}

void addRoundKey(struct matrix *mat, const struct matrix * key, unsigned char round)
{
    unsigned char i, j;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++)
            *(mat->array+4*i + j) = *(mat->array+4*i + j) ^ *(key->array + 4*i + 4*round +j);
    }
}

void change_key_matrix(struct matrix *key)
{
    unsigned char temp[4];
    int i, j, round;
    for(round=1; round<=10; round++){
        // deal with the 0-th column
        for(i=0;i<4;i++) temp[i] = *(key->array + 4*i + 4*round - 1);
        trans_key(temp, round);
        for(i=0;i<4;i++) *(key->array + 4*i + 4*round) = *(key->array + 4 * round - 4 + 4 * i)^temp[i];
        // deal with 1-st, 1-nd, 3-rd column
        for(j=1; j<4; j++){
            for(i=0;i<4;i++) *(key->array+4*i+4*round+j) =
                                     *(key->array+4*i+4*round+j-4) ^ *(key->array+4*i+4*round+j-1);
        }
    }
}
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

struct matrix{
    int row;
    int col;
    unsigned int* array;
};

// Matrix added for Sbox.
unsigned int toAdd = 99;

// Mtrix multiplied for Sbox
unsigned int SboxMul[8] = {241, 227, 199, 143, 31, 62, 124, 248};

// C(x) in the Mix Column Layer
unsigned int toMix[4][4] = {
        {2, 3, 1, 1},
        {1, 2, 3, 1},
        {1, 1, 2, 3},
        {3, 1, 1, 2}
};

// P(x) used in AES
unsigned int Px = 27;

// Used in transform key.
unsigned int r = 2;

// Global matrix, which is used to generate SBOX.
struct matrix toMul;

// Given a binary array, write the inverse of the array in test.
unsigned int findInverse(unsigned int a);

// Given a single byte, with decimal representation n, return decimal of the subbyte version.
unsigned int SubSingleByte(  unsigned int n);

// Subbytes layer. Change the input matrix.
void SubBytes(struct matrix *mat);

// ShiftRow layer, the matrix will be changed.
void shiftRows(struct matrix *mat);

unsigned int con_multiply(unsigned int left, unsigned int right);

unsigned int generateSBox(unsigned int a);

int main(int argc, char *argv[])
{
    printf("%d\n", generateSBox(141));
    printf("%d\n", con_multiply(2, 141));
    printf("%d\n", findInverse(3));
    int i,j, round;
    toMul.row = 8;
    toMul.col = 8;
    toMul.array = (unsigned int*)SboxMul;
// here are the plain text that has a bytes.
    unsigned int a[4][4] = {{0, 203, 2, 3}, {203, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};
//    for(j=0;j<4;j++){
//        for(i=0;i<4;i++) a[i][j]=argv[1][i+j*4];
//    }
    struct matrix plain = {.array=(unsigned int*)a, .row = 4, .col = 4};

// here are the keys(128bits).
//    unsigned int k[4][4];
//    for(j=0;j<4;j++){
//        for(i=0;i<4;i++) k[i][j]=(unsigned char)argv[2][i+j*4];
//    }
//
//    struct matrix key = {.array=(unsigned char*)malloc(4*44), .row=4, .col=44};
//    for(i=0; i<4; i++){
//        for(j=0;j<4;j++){
//            *(key.array + i*4 +j) = k[i][j];
//        }
//    }
//    change_key_matrix(&key);
//
//    addRoundKey(&plain, &key, 0);
//    for(round =1; round<10; round++) {
//        SubBytes(&plain);
//        shiftRows(&plain);
//        mixColumn(&plain);
//        addRoundKey(&plain, &key, round);
//    }
    SubBytes(&plain);
    shiftRows(&plain);
//    addRoundKey(&plain, &key, 10);
    for(i=0; i<4; i++) {
        for(j=0;j<4;j++) {
            printf("%d ", *(plain.array + 4*i +j));
        }
        printf("\n");
    }
    printf("\n");
    return 0;
}

unsigned int con_multiply(unsigned int left, unsigned int right)
{
    unsigned int result=0, temp=0;
    int i, j, n;
    for(i=128,j=7;i>=1;i/=2,j--){
        if(right/i){
            temp=left;
            for(n=0;n<j;n++){
                if(temp>=128){
                    temp=2*temp - 256;
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

unsigned int findInverse(unsigned int a)
{
    unsigned int i;
    for(i=0;i<256;i++){
        if(con_multiply(a,i)==1) return i;
    }
}

bool ifOddOnes(unsigned int a)
{
    bool flag = false;
    int i;
    for(i=128;i>=1, a>0;i/=2){
        if(a/i){
            flag = !flag;
            a -= i;
        }
    }
    return flag;
}

unsigned int generateSBox(unsigned int a)
{
    int i, j;
    unsigned int result=0, temp;
    for (i=0,j=1; i<8; i++, j*=2){
        temp = SboxMul[i] & a;
        result += ifOddOnes(temp) * j;
    }
    printf("%d\n", result);
    return result^toAdd;
}


unsigned int SubSingleByte(unsigned int n)
{
    unsigned int inverse=0;
    if(n) inverse = findInverse(n);
    return generateSBox(inverse);
}

void SubBytes(struct matrix *mat)
{
    unsigned int i, j;
    for(i = 0; i<mat->row;i++){
        for (j=0;j<mat->col;j++)
            *(mat->array+i*mat->col + j) = SubSingleByte(*(mat->array+i*mat->col + j));
    }
}

void shiftRows(struct matrix *mat)
{
    int i;
    for(i = 1; i <= 3; i++){
        unsigned int temp[4]  = {
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

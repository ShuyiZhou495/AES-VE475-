#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

struct matrix{
    unsigned int row;
    unsigned int col;
    unsigned int* array;
};

// Matrix added for Sbox.
  unsigned int toAdd[8] = {1, 1, 0, 0, 0, 1, 1, 0};

// Mtrix multiplied for Sbox
  unsigned int SboxMul[8][8] = {
        {1, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 1, 1},
        {1, 1, 1, 0, 0, 0, 1, 1},
        {1, 1, 1, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 1, 1, 1, 1, 1}
};

// C(x) in the Mix Column Layer
unsigned int toMix[4][4] = {
        {2, 3, 1, 1},
        {1, 2, 3, 1},
        {1, 1, 2, 3},
        {3, 1, 1, 2}
};

// P(x) used in AES
unsigned int Px[8] = {1, 1, 0, 1, 1, 0, 0, 0};

// Used in transform key.
unsigned int r[8] = {0, 0, 0, 0, 0, 0, 1, 0};

// Global matrix, which is used to generate SBOX.
struct matrix toMul;

// Do the xor operation, array1 will be changed to the result, the second array will not be changed..
void xor(  unsigned int* array1, const   unsigned int* array2);

// Do the normal multiplication of two matrixs, the result of (left)(right) will be written in (result).
void multiply(struct matrix *result, const struct matrix *left, const struct matrix *right);

// ShiftRow layer, the matrix will be changed.
void shiftRows(struct matrix *mat);

// Add toAdd array to the input array. The input array will be changed.
void addForSBox(unsigned int *array, struct matrix *pMatrix, unsigned int i);

// Input the array (length 8, binary, represents the bits), change the array to Sbox generated array.
void generateSBox(unsigned int *array);

// Write the binary array of unsigned int a in array.
void toBinary(int a, unsigned int* array);

// Input the binary array, return decimal number.
int toDecimal(const unsigned int *array);

// In the multiplication of AES binary columns, this is shift the array to left for one bits for n times.
// If the most significant bit is 1, it will do xor with P(x).
void multiplyNTimes(unsigned int *result, unsigned int* matrix, unsigned int n);

// Multiplication of AES binary columns. like 00000100*10000000. Result is return in result array.
void con_multiply(unsigned int* result, const unsigned int* left, const unsigned int* right);

// Used in findInverse function. Check whether the flag(input) array is 00000001.
bool testInverse(const unsigned int*array);

// Given a binary array, write the inverse of the array in test.
void findInverse(const unsigned int* array, unsigned int* test);

// Given a single byte, with decimal representation n, return decimal of the subbyte version.
int SubSingleByte(  unsigned int n);

// Subbytes layer. Change the input matrix.
void SubBytes(struct matrix *mat);

// Mix columns layer, change the input matrix.
void mixColumn(struct matrix *mat);

// Used in mix column function. Turn a and b to binary arrays and do the AES multiplication. And then
// xor the result with the binary array of add. And return the decimal of xor result.
int con_multiply_num(int a, unsigned int b, unsigned int add);

void change_key_matrix(struct matrix *key);

int xor_num(int a, unsigned int b);

int xor_num_array(int a, unsigned int* b);

void trans_key(unsigned int* K, int round);

void addRoundKey(struct matrix *mat, const struct matrix * key, unsigned int round);

int main(int argc, char *argv[])
{
    int i,j, round;
    toMul.row = 8;
    toMul.col = 8;
    toMul.array = (unsigned int*)SboxMul;
// here are the plain text that has a bytes.
    unsigned int a[4][4];
    for(j=0;j<4;j++){
        for(i=0;i<4;i++) a[i][j]=(int)argv[1][i+j*4];
    }
    struct matrix plain = {.array=(  unsigned int*)a, .row = 4, .col = 4};

// here are the keys(128bits).
    unsigned int k[4][4];
    for(j=0;j<4;j++){
        for(i=0;i<4;i++) k[i][j]=(  unsigned int)argv[2][i+j*4];
    }

    struct matrix key = {.array=(unsigned int*)malloc(4*44), .row=4, .col=44};
    for(i=0; i<4; i++){
        for(j=0;j<4;j++){
            *(key.array + i*4 +j) = k[i][j];
        }
    }
    change_key_matrix(&key);

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
    for(j=0; j<4; j++) {
        for(i=0;i<4;i++) {
            printf("%c", *(plain.array + 4*i +j));
        }
    }
    printf("\n");
    return 0;
}

void xor(  unsigned int* array1, const   unsigned int* array2){
    unsigned int i;
    for(i=0; i<8; i++) *(array1+i) = (*(array1+i) + *(array2+i)) % 2;
}


void multiply(struct matrix *result, const struct matrix *left, const struct matrix *right)
{
    assert(left->col==right->row);
    unsigned int i,j,k;
    for(i=0;i<result->row;i++){
        for(j=0;j<result->col;j++){
            *(result->array + i * result->col  + j )= 0;
            for(k=0;k<left->col;k++){
                *(result->array + i * result->col  + j ) +=
                        *(left->array + i * left->col  + k ) * *(right->array + k * right->col  + j );
            }
        }
    }
}

void shiftRows(struct matrix *mat)
{
    unsigned int i;
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

void addForSBox(  unsigned int *array, struct matrix *pMatrix, unsigned int i)
{
    xor(array, toAdd);
}

void generateSBox(  unsigned int *array)
{
    struct matrix b = {.col = 1, .row = 8, .array = array};
    struct matrix c = {.col = 1, .row = 8, .array = (  unsigned int*) malloc(8)};
    multiply(&c, &toMul, &b);
    addForSBox(c.array, NULL, 0);
    unsigned int i;
    for(i=0;i<8;i++) *(array+i) = *(c.array + i);
    free(c.array);
}

void toBinary(int a,   unsigned int* array)
{
    unsigned int i, j;
    for (i=128, j=7; i>=1; i/=2, j--){
        *(array + j) = a/i;
        a -= (a/i)*i;
    }
}

int toDecimal(const   unsigned int *array)
{
    unsigned int i,j, result=0;
    for(i=0,j=1;i<8;i++,j*=2){
        result += *(array+i) * j;
    }
    return result;
}

void con_multiply(  unsigned int* result, const   unsigned int* left, const   unsigned int* right)
{
    unsigned int i, j;
    for(i=7;i>=0; i--){
        if(*(right+i)) {
            unsigned int temp[8];
            for(j=0;j<8;j++) temp[j] = *(left+j);
            multiplyNTimes(result, (  unsigned int*)temp, i);
        }
    }
}


void multiplyNTimes(unsigned int *result,   unsigned int* matrix, unsigned int n)
{
    unsigned int i, flag,j;
    for(i=0;i<n;i++){
        flag = *(matrix + 7);
        for(j=7;j>0;j--) *(matrix+j) = *(matrix+j-1);
        *(matrix) = 0;
        if(flag==1) xor(matrix, (  unsigned int*)Px);
    }
    xor(result, matrix);
}

bool testInverse(const   unsigned int*array)
{
    if(*(array)==0) return false;
    unsigned int i;
    for(i=1;i<8;i++){
        if(*(array+i)==1) return false;
    }
    return true;
}

void findInverse(const   unsigned int* array,   unsigned int*test)
{
    unsigned int i;
    for(i=0;i<256;i++){
        toBinary(i, test);
          unsigned int result[8]={0, 0, 0, 0, 0, 0, 0, 0};
        con_multiply(result, array, test);
        if(testInverse(result)) return;
    }
}

int SubSingleByte(  unsigned int n)
{
    unsigned int temp[8], inverse[8]={0, 0, 0, 0, 0, 0, 0, 0};
    if(n){
        toBinary(n, (  unsigned int*)temp);
        findInverse((  unsigned int*)temp, (  unsigned int*)inverse);
    }
    generateSBox((  unsigned int*) inverse);
    return toDecimal((  unsigned int*) inverse);
}

void SubBytes(struct matrix *mat)
{
    unsigned int i, j;
    for(i = 0; i<mat->row;i++){
        for (j=0;j<mat->col;j++) *(mat->array+i*mat->col + j) = SubSingleByte(*(mat->array+i*mat->col + j));
    }
}

void mixColumn(struct matrix *mat)
{
    unsigned int temp[4], i, j, k;
    for (i=0;i<4;i++){
        for(j=0; j<4; j++){
            temp[j] = 0;
            for(k=0;k<4;k++){
                temp[j]=con_multiply_num(toMix[j][k], *(mat->array + k*4 + i), temp[j]);
            }
        }
        for(j=0;j<4;j++) *(mat->array + j*4 + i) = temp[j];
    }
}

int con_multiply_num(int a, unsigned int b, unsigned int add)
{
      unsigned int temp_a[8], temp_b[8], temp_result[8] = {0, 0, 0, 0, 0, 0, 0, 0}, temp_add[8];
    toBinary(a, temp_a);
    toBinary(b, temp_b);
    con_multiply(temp_result, temp_a, temp_b);
    toBinary(add, temp_add);
    xor(temp_result, temp_add);
    return toDecimal(temp_result);
}

void change_key_matrix(struct matrix *key)
{
    unsigned int temp[4];
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

int xor_num(int a, unsigned int b)
{
    unsigned int temp_a[8], temp_b[8];
    toBinary(a, temp_a);
    toBinary(b, temp_b);
    xor(temp_a, temp_b);
    return toDecimal(temp_a);
}

int xor_num_array(int a,   unsigned int* b)
{
    unsigned int temp_a[8];
    toBinary(a, temp_a);
    xor(temp_a, b);
    return toDecimal(temp_a);
}

void trans_key(unsigned int* K, int round)
{
    unsigned int result[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    multiplyNTimes(result, r, round - 1);
    unsigned int temp = *(K), i;
    // Cycally shift
    for(i=0;i<3;i++) *(K+i) = *(K+i+1);
    *(K+3) = temp;
    // subbytes
    for(i=0;i<4;i++) *(K) = SubSingleByte(*(K));
    // xor a and r
    *(K) = xor_num_array(*(K), result);
}

void addRoundKey(struct matrix *mat, const struct matrix * key, unsigned int round)
{
    unsigned int i, j;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++)
            *(mat->array+4*i + j) = xor_num(*(mat->array+4*i + j), *(key->array + 4*i + 4*round +j));
    }
}
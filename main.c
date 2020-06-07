#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

struct matrix{
    int row;
    int col;
    int* array;
};

// Matrix added for Sbox.
int toAdd[8] = {1, 1, 0, 0, 0, 1, 1, 0};

// Mtrix multiplied for Sbox
int SboxMul[8][8] = {
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
int toMix[4][4] = {
        {2, 3, 1, 1},
        {1, 2, 3, 1},
        {1, 1, 2, 3},
        {3, 1, 1, 2}
};

// P(x) used in AES
int Px[8] = {1, 1, 0, 1, 1, 0, 0, 0};

// Global matrix, which is used to generate SBOX.
struct matrix toMul;

// Do the xor operation, array1 will be changed to the result, the second array will not be changed..
void xor(int* array1, const int* array2);

// Do the normal multiplication of two matrixs, the result of (left)(right) will be written in (result).
void multiply(struct matrix *result, const struct matrix *left, const struct matrix *right);

// ShiftRow layer, the matrix will be changed.
void shiftRows(struct matrix *mat);

// Add toAdd array to the input array. The input array will be changed.
void addForSBox(int *array);

// Input the array (length 8, binary, represents the bits), change the array to Sbox generated array.
void generateSBox(int *array);

// Write the binary array of int a in array.
void toBinary(int a, int* array);

// Input the binary array, return decimal number.
int toDecimal(const int *array);

// In the multiplication of AES binary columns, this is shift the array to left for one bits for n times.
// If the most significant bit is 1, it will do xor with P(x).
void multiplyNTimes(int *result, int* matrix, int n);

// Multiplication of AES binary columns. like 00000100*10000000. Result is return in result array.
void con_multiply(int* result, const int* left, const int* right);

// Used in findInverse function. Check whether the flag(input) array is 00000001.
bool testInverse(const int*array);

// Given a binary array, write the inverse of the array in test.
void findInverse(const int* array, int* test);

// Given a single byte, with decimal representation n, return decimal of the subbyte version.
int SubSingleByte(int n);

// Subbytes layer. Change the input matrix.
void SubBytes(struct matrix *mat);

// Mix columns layer, change the input matrix.
void mixColumn(struct matrix *mat);

// Used in mix column function. Turn a and b to binary arrays and do the AES multiplication. And then
// xor the result with the binary array of add. And return the decimal of xor result.
int con_multiply_num(int a, int b, int add);


int main()
{

    toMul.row = 8;
    toMul.col = 8;
    toMul.array = (int*)SboxMul;

// here are the plain text that has a bytes.
int a[4][4]={
        {219, 1, 2, 3} ,
        {19, 1, 6, 7} ,
        {83, 1, 10, 11},
        {69, 1, 14, 15}
};
struct matrix plain = {.array=(int*)a, .row = 4, .col = 4};

shiftRows(&plain);
SubBytes(&plain);
mixColumn(&plain);


int i,j;
for(i=0; i<4; i++) {
    for(j=0;j<4;j++) {
        printf("%d ", *(plain.array + 4*i +j));
    }
    printf("\n");
}

// Enter the value for plain text in decimal numbers. 0 to 2^8-1
//    for(i=0; i<4; i++) {
//        for(j=0;j<4;j++) {
//            printf("Enter value for a%d,%d:", i, j);
//            scanf("%d", &a[i][j]);
//        }
//    }
    return 0;
}

void xor(int* array1, const int* array2){
    int i;
    for(i=0; i<8; i++) *(array1+i) = (*(array1+i) + *(array2+i)) % 2;
}


void multiply(struct matrix *result, const struct matrix *left, const struct matrix *right)
{
    assert(left->col==right->row);
    int i,j,k;
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
    int i;
    for(i = 1; i <= 3; i++){
        int temp[4]  = {
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

void addForSBox(int *array)
{
    xor(array, toAdd);
}

void generateSBox(int *array)
{
    struct matrix b = {.col = 1, .row = 8, .array = array};
    struct matrix c = {.col = 1, .row = 8, .array = (int*) malloc(8)};
    multiply(&c, &toMul, &b);
    addForSBox(c.array);
    int i;
    for(i=0;i<8;i++) *(array+i) = *(c.array + i);
    free(c.array);
}

void toBinary(int a, int* array)
{
    int i, j;
    for (i=128, j=7; i>=1; i/=2, j--){
        *(array + j) = a/i;
        a -= (a/i)*i;
    }
}

int toDecimal(const int *array)
{
    int i,j, result=0;
    for(i=0,j=1;i<8;i++,j*=2){
        result += *(array+i) * j;
    }
    return result;
}

void con_multiply(int* result, const int* left, const int* right){
    int i, j;
    for(i=7;i>=0; i--){
        if(*(right+i)) {
            int temp[8];
            for(j=0;j<8;j++) temp[j] = *(left+j);
            multiplyNTimes(result, (int*)temp, i);
        }
    }
}


void multiplyNTimes(int *result, int* matrix, int n)
{
    int i, flag,j;
    for(i=0;i<n;i++){
        flag = *(matrix + 7);
        for(j=7;j>0;j--) *(matrix+j) = *(matrix+j-1);
        *(matrix) = 0;
        if(flag==1) xor(matrix, (int*)Px);
    }
    xor(result, matrix);
}

bool testInverse(const int*array){
    if(*(array)==0) return false;
    int i;
    for(i=1;i<8;i++){
        if(*(array+i)==1) return false;
    }
    return true;
}

void findInverse(const int* array, int*test){
    int i;
    for(i=0;i<256;i++){
        toBinary(i, test);
        int result[8]={0, 0, 0, 0, 0, 0, 0, 0};
        con_multiply(result, array, test);
        if(testInverse(result)) return;
    }
}

int SubSingleByte(int n){
    int temp[8], inverse[8]={0, 0, 0, 0, 0, 0, 0, 0};
    if(n){
        toBinary(n, (int*)temp);
        findInverse((int*)temp, (int*)inverse);
    }
    generateSBox((int*) inverse);
    return toDecimal((int*) inverse);
}

void SubBytes(struct matrix *mat)
{
    int i, j;
    for(i = 0; i<mat->row;i++){
        for (j=0;j<mat->col;j++) *(mat->array+i*mat->col + j) = SubSingleByte(*(mat->array+i*mat->col + j));
    }
}

void mixColumn(struct matrix *mat)
{
    int temp[4], i, j, k;
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

int con_multiply_num(int a, int b, int add){
    int temp_a[8], temp_b[8], temp_result[8] = {0, 0, 0, 0, 0, 0, 0, 0}, temp_add[8];
    toBinary(a, temp_a);
    toBinary(b, temp_b);
    con_multiply(temp_result, temp_a, temp_b);
    toBinary(add, temp_add);
    xor(temp_result, temp_add);
    return toDecimal(temp_result);
}
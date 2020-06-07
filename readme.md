## How to use:
- `./AES <key>` or `./AES <plain> <key>`. 
- For instance, `./AES plainplaintexttt keykeykeykeykeyk` or `./AES keykeykeykeykeyk`
- The output first print the cipher matrix in decimal, then print the ciphertext according to ASCii.
- If there if no input plaintext, the default plain text is `{{219, 1, 83, 69}, {19, 1, 1, 1}, {83, 1, 10, 11}, {69, 1, 14, 15}}`
- The input key and plaintext should be no less that 128bits string (16 chars). The part more than 16 chars will be ignored.

## Functions
### Structure
I used a structure to save plaintext and key. It is interpreted as matrix.
```c
struct matrix{
    int row;
    int col;
    unsigned char* array;
};
``` 
In all the layers, the address of the matrix is passed into the function, and matrix itself is changed.
### Four layers are in the following functions:
```c
// Subbytes layer. Change the input matrix.
void SubBytes(struct matrix *mat);

// ShiftRow layer, the matrix will be changed.
void shiftRows(struct matrix *mat);

// Mix column layer
void mixColumn(struct matrix *mat);

// Add round key layer.
void addRoundKey(struct matrix *mat, const struct matrix * key, unsigned char round);
 ```

### Generate Sbox function
```c
// Given a single byte, with decimal representation n, return decimal of the subbyte version.
   unsigned char SubSingleByte(unsigned char n);
```

### The first three layers
I have tested they are right.

### AddRoundKey
I haven't tested this layer. This layer is only for adding key and plaintext, while generating the 40 columns of key is in the function 
```c
void change_key_matrix(struct matrix *key);
```
This is based on the four initial columns are already in the key matrix.

### operation
All the operations are based on c logic operations `&` and `^`.
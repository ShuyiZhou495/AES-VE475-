# Four layers are in the following functions:
```
// ShiftRow layer, the matrix will be changed.
 void shiftRows(struct matrix *mat);

// Subbytes layer. Change the input matrix.
 void SubBytes(struct matrix *mat);

// Mix columns layer, change the input matrix.
 void mixColumn(struct matrix *mat);
 ```

void change_key_matrix(struct matrix *key)
void trans_key(unsigned int* K, int round)
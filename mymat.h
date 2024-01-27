#ifndef MATRIX_H
#define MATRIX_H

typedef struct {
    float values[4][4];
    const char* name;
} Matrix;

void read_mat(Matrix* mat, float values[]);
void print_mat(Matrix* mat);
void add_mat(Matrix* A, Matrix* B, Matrix* C);
void sub_mat(Matrix* A, Matrix* B, Matrix* C);
void mul_scalar(Matrix* A, Matrix* B, float alpha);
void mul_mat(Matrix* A, Matrix* B,Matrix* C);
void trans_mat(Matrix* A, Matrix* B);

#endif


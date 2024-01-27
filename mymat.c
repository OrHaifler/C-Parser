#include "mymat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
Insert values into matrix
*/
void read_mat(Matrix* mat, float values[]) {
    int i,j;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            mat->values[i][j] = values[4 * i + j];
        }
    }
}
/*
Print matrix
*/
void print_mat(Matrix* mat) {
    int i,j;
    
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            printf("%.2f\t", mat->values[i][j]);
        }
        printf("\n");
    }
}
/*
Add A and B and store the result in C
*/
void add_mat(Matrix* A, Matrix* B, Matrix* C) {
    int i,j;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            C->values[i][j] = A->values[i][j] + B->values[i][j];           
        }    
    }
}
/*
Subtract A and B and store the result in C
*/
void sub_mat(Matrix* A, Matrix* B, Matrix* C) {
    int i,j;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            C->values[i][j] = A->values[i][j] - B->values[i][j];           
        }    
    }
}
/*
Multiply A by alpha and store the result in B
*/
void mul_scalar(Matrix* A, Matrix* B, float alpha) {
    int i,j;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            B->values[i][j] = alpha * A->values[i][j];           
        }    
    }
}

/*
Multiply A and B and store the result in C
*/
void mul_mat(Matrix* A, Matrix* B, Matrix* C) {
    int i,j,k;
    Matrix* D = (Matrix*)malloc(sizeof(Matrix));
    mul_scalar(D, D, 0);
    
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            for(k = 0; k < 4; k++) {
                D->values[i][j] += A->values[i][k] * B->values[k][j];           
            }
        }    
    }
    
    mul_scalar(D,C,1);

    free(D);
}
/*
Transpose A and store the result in B
*/
void trans_mat(Matrix* A, Matrix* B) {
    int i,j;
    Matrix* C = (Matrix*)malloc(sizeof(Matrix));
    
    mul_scalar(C,C,0);
    
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            C->values[i][j] = A->values[j][i];           
        }    
    }
    
    mul_scalar(C,B,1);

    free(C);  
}



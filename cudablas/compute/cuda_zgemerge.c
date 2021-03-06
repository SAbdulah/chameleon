/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2016 Inria. All rights reserved.
 * @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file cuda_zmerge.c
 *
 *  MORSE cudablas kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver,
 *  and INRIA Bordeaux Sud-Ouest
 *
 * @author Florent Pruvost
 * @date 2015-09-16
 * @precisions normal z -> c d s
 *
 **/
#include "cudablas.h"

int
CUDA_zgemerge( MORSE_enum side, MORSE_enum diag,
               int M, int N,
               cuDoubleComplex *A, int LDA,
               cuDoubleComplex *B, int LDB,
               CUBLAS_STREAM_PARAM)
{
    int i;
    cuDoubleComplex *cola, *colb;

    if (M < 0) {
        return -1;
    }
    if (N < 0) {
        return -2;
    }
    if ( (LDA < chameleon_max(1,M)) && (M > 0) ) {
        return -5;
    }
    if ( (LDB < chameleon_max(1,M)) && (M > 0) ) {
        return -7;
    }

    CUBLAS_GET_STREAM;

    if (side == MorseLeft){
        for(i=0; i<N; i++){
            cola = A + i*LDA;
            colb = B + i*LDB;
            cudaMemcpyAsync(colb , cola,
                            (i+1)*sizeof(cuDoubleComplex),
                            cudaMemcpyDeviceToDevice, stream);
        }
    }else{
        for(i=0; i<N; i++){
            cola = A + i*LDA;
            colb = B + i*LDB;
            cudaMemcpyAsync(colb+i , cola+i,
                            (M-i)*sizeof(cuDoubleComplex),
                            cudaMemcpyDeviceToDevice, stream);
        }
    }

    (void)diag;
    return MORSE_SUCCESS;
}

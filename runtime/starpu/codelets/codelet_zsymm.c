/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2014 Inria. All rights reserved.
 * @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file codelet_zsymm.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for MORSE 1.0.0
 * @author Hatem Ltaief
 * @author Jakub Kurzak
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2010-11-15
 * @precisions normal z -> c d s
 *
 **/
#include "morse_starpu.h"
#include "codelet_z.h"

/**
 *
 * @ingroup CORE_MORSE_Complex64_t
 *
 **/

void MORSE_TASK_zsymm(MORSE_option_t *options,
                      MORSE_enum side, MORSE_enum uplo,
                      int m, int n, int nb,
                      MORSE_Complex64_t alpha, MORSE_desc_t *A, int Am, int An, int lda,
                      MORSE_desc_t *B, int Bm, int Bn, int ldb,
                      MORSE_Complex64_t beta, MORSE_desc_t *C, int Cm, int Cn, int ldc)
{
    (void)nb;
    struct starpu_codelet *codelet = &cl_zsymm;
    void (*callback)(void*) = options->profiling ? cl_zsymm_callback : NULL;

    if ( morse_desc_islocal( A, Am, An ) ||
         morse_desc_islocal( B, Bm, Bn ) ||
         morse_desc_islocal( C, Cm, Cn ) )
    {
        starpu_insert_task(
            codelet,
            STARPU_VALUE,    &side,                sizeof(MORSE_enum),
            STARPU_VALUE,    &uplo,                sizeof(MORSE_enum),
            STARPU_VALUE,       &m,                        sizeof(int),
            STARPU_VALUE,       &n,                        sizeof(int),
            STARPU_VALUE,   &alpha,         sizeof(MORSE_Complex64_t),
            STARPU_R,               RTBLKADDR(A, MORSE_Complex64_t, Am, An),
            STARPU_VALUE,     &lda,                        sizeof(int),
            STARPU_R,               RTBLKADDR(B, MORSE_Complex64_t, Bm, Bn),
            STARPU_VALUE,     &ldb,                        sizeof(int),
            STARPU_VALUE,    &beta,         sizeof(MORSE_Complex64_t),
            STARPU_RW,               RTBLKADDR(C, MORSE_Complex64_t, Cm, Cn),
            STARPU_VALUE,     &ldc,                        sizeof(int),
            STARPU_PRIORITY,    options->priority,
            STARPU_CALLBACK,    callback,
            0);
    }
}


static void cl_zsymm_cpu_func(void *descr[], void *cl_arg)
{
    MORSE_enum side;
    MORSE_enum uplo;
    int M;
    int N;
    MORSE_Complex64_t alpha;
    MORSE_Complex64_t *A;
    int LDA;
    MORSE_Complex64_t *B;
    int LDB;
    MORSE_Complex64_t beta;
    MORSE_Complex64_t *C;
    int LDC;

    A = (MORSE_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[0]);
    B = (MORSE_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[1]);
    C = (MORSE_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[2]);
    starpu_codelet_unpack_args(cl_arg, &side, &uplo, &M, &N, &alpha, &LDA, &LDB, &beta, &LDC);
    cblas_zsymm(
        CblasColMajor,
        (CBLAS_SIDE)side, (CBLAS_UPLO)uplo,
        M, N,
        CBLAS_SADDR(alpha), A, LDA,
        B, LDB,
        CBLAS_SADDR(beta), C, LDC);
}

#ifdef CHAMELEON_USE_CUDA
static void cl_zsymm_cuda_func(void *descr[], void *cl_arg)
{
    MORSE_enum side;
    MORSE_enum uplo;
    int M;
    int N;
    cuDoubleComplex alpha;
    cuDoubleComplex *A;
    int LDA;
    cuDoubleComplex *B;
    int LDB;
    cuDoubleComplex beta;
    cuDoubleComplex *C;
    int LDC;

    A = (cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[0]);
    B = (cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[1]);
    C = (cuDoubleComplex *)STARPU_MATRIX_GET_PTR(descr[2]);
    starpu_codelet_unpack_args(cl_arg, &side, &uplo, &M, &N, &alpha, &LDA, &LDB, &beta, &LDC);

    CUstream stream = starpu_cuda_get_local_stream();
    cublasSetKernelStream( stream );

    cublasZsymm(
        morse_lapack_const(side), morse_lapack_const(uplo),
        M, N,
        alpha, A, LDA,
        B, LDB,
        beta, C, LDC);

#ifndef STARPU_CUDA_ASYNC
    cudaStreamSynchronize( stream );
#endif

    return;
}
#endif

/*
 * Codelet definition
 */
CODELETS(zsymm, 3, cl_zsymm_cpu_func, cl_zsymm_cuda_func, STARPU_CUDA_ASYNC)

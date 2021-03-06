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
 * @precisions normal z -> c d s
 *
 **/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <morse.h>
#include <coreblas/cblas.h>
#include <coreblas/lapacke.h>
#include <coreblas.h>
#include "timing_zauxiliary.h"

/*-------------------------------------------------------------------
 * Check the orthogonality of Q
 */

int z_check_orthogonality(int M, int N, int LDQ, MORSE_Complex64_t *Q)
{
    double alpha, beta;
    double normQ;
    int info_ortho;
    int i;
    int minMN = chameleon_min(M, N);
    double eps;
    double *work = (double *)malloc(minMN*sizeof(double));

    eps = LAPACKE_dlamch_work('e');
    alpha = 1.0;
    beta  = -1.0;

    /* Build the idendity matrix USE DLASET?*/
    MORSE_Complex64_t *Id = (MORSE_Complex64_t *) malloc(minMN*minMN*sizeof(MORSE_Complex64_t));
    memset((void*)Id, 0, minMN*minMN*sizeof(MORSE_Complex64_t));
    for (i = 0; i < minMN; i++)
        Id[i*minMN+i] = (MORSE_Complex64_t)1.0;

    /* Perform Id - Q'Q */
    if (M >= N)
        cblas_zherk(CblasColMajor, CblasUpper, CblasConjTrans, N, M, alpha, Q, LDQ, beta, Id, N);
    else
        cblas_zherk(CblasColMajor, CblasUpper, CblasNoTrans, M, N, alpha, Q, LDQ, beta, Id, M);

    normQ = LAPACKE_zlansy_work(LAPACK_COL_MAJOR, 'I', 'u', minMN, Id, minMN, work);

    printf("============\n");
    printf("Checking the orthogonality of Q \n");
    printf("||Id-Q'*Q||_oo / (N*eps) = %e \n",normQ/(minMN*eps));

    if ( isnan(normQ / (minMN * eps)) || (normQ / (minMN * eps) > 10.0) ) {
        printf("-- Orthogonality is suspicious ! \n");
        info_ortho=1;
    }
    else {
        printf("-- Orthogonality is CORRECT ! \n");
        info_ortho=0;
    }

    free(work); free(Id);

    return info_ortho;
}

/*------------------------------------------------------------
 *  Check the factorization QR
 */

int z_check_QRfactorization(int M, int N, MORSE_Complex64_t *A1, MORSE_Complex64_t *A2, int LDA, MORSE_Complex64_t *Q)
{
    double Anorm, Rnorm;
    MORSE_Complex64_t alpha, beta;
    int info_factorization;
    int i,j;
    double eps;

    eps = LAPACKE_dlamch_work('e');

    MORSE_Complex64_t *Ql       = (MORSE_Complex64_t *)malloc(M*N*sizeof(MORSE_Complex64_t));
    MORSE_Complex64_t *Residual = (MORSE_Complex64_t *)malloc(M*N*sizeof(MORSE_Complex64_t));
    double *work              = (double *)malloc(chameleon_max(M,N)*sizeof(double));

    alpha=1.0;
    beta=0.0;

    if (M >= N) {
        /* Extract the R */
        MORSE_Complex64_t *R = (MORSE_Complex64_t *)malloc(N*N*sizeof(MORSE_Complex64_t));
        memset((void*)R, 0, N*N*sizeof(MORSE_Complex64_t));
        LAPACKE_zlacpy_work(LAPACK_COL_MAJOR,'u', M, N, A2, LDA, R, N);

        /* Perform Ql=Q*R */
        memset((void*)Ql, 0, M*N*sizeof(MORSE_Complex64_t));
        cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, M, N, N, CBLAS_SADDR(alpha), Q, LDA, R, N, CBLAS_SADDR(beta), Ql, M);
        free(R);
    }
    else {
        /* Extract the L */
        MORSE_Complex64_t *L = (MORSE_Complex64_t *)malloc(M*M*sizeof(MORSE_Complex64_t));
        memset((void*)L, 0, M*M*sizeof(MORSE_Complex64_t));
        LAPACKE_zlacpy_work(LAPACK_COL_MAJOR,'l', M, N, A2, LDA, L, M);

    /* Perform Ql=LQ */
        memset((void*)Ql, 0, M*N*sizeof(MORSE_Complex64_t));
        cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, M, N, M, CBLAS_SADDR(alpha), L, M, Q, LDA, CBLAS_SADDR(beta), Ql, M);
        free(L);
    }

    /* Compute the Residual */
    for (i = 0; i < M; i++)
        for (j = 0 ; j < N; j++)
            Residual[j*M+i] = A1[j*LDA+i]-Ql[j*M+i];

    Rnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Residual, M, work);
    Anorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, A2, LDA, work);

    if (M >= N) {
        printf("============\n");
        printf("Checking the QR Factorization \n");
        printf("-- ||A-QR||_oo/(||A||_oo.N.eps) = %e \n",Rnorm/(Anorm*N*eps));
    }
    else {
        printf("============\n");
        printf("Checking the LQ Factorization \n");
        printf("-- ||A-LQ||_oo/(||A||_oo.N.eps) = %e \n",Rnorm/(Anorm*N*eps));
    }

    if (isnan(Rnorm / (Anorm * N *eps)) || (Rnorm / (Anorm * N * eps) > 10.0) ) {
        printf("-- Factorization is suspicious ! \n");
        info_factorization = 1;
    }
    else {
        printf("-- Factorization is CORRECT ! \n");
        info_factorization = 0;
    }

    free(work); free(Ql); free(Residual);

    return info_factorization;
}

/*------------------------------------------------------------------------
 *  Check the factorization of the matrix A2
 */

int z_check_LLTfactorization(int N, MORSE_Complex64_t *A1, MORSE_Complex64_t *A2, int LDA, int uplo)
{
    double Anorm, Rnorm;
    MORSE_Complex64_t alpha;
    int info_factorization;
    int i,j;
    double eps;

    eps = LAPACKE_dlamch_work('e');

    MORSE_Complex64_t *Residual = (MORSE_Complex64_t *)malloc(N*N*sizeof(MORSE_Complex64_t));
    MORSE_Complex64_t *L1       = (MORSE_Complex64_t *)malloc(N*N*sizeof(MORSE_Complex64_t));
    MORSE_Complex64_t *L2       = (MORSE_Complex64_t *)malloc(N*N*sizeof(MORSE_Complex64_t));
    double *work              = (double *)malloc(N*sizeof(double));

    memset((void*)L1, 0, N*N*sizeof(MORSE_Complex64_t));
    memset((void*)L2, 0, N*N*sizeof(MORSE_Complex64_t));

    alpha= 1.0;

    LAPACKE_zlacpy_work(LAPACK_COL_MAJOR,' ', N, N, A1, LDA, Residual, N);

    /* Dealing with L'L or U'U  */
    if (uplo == MorseUpper){
        LAPACKE_zlacpy_work(LAPACK_COL_MAJOR,'u', N, N, A2, LDA, L1, N);
        LAPACKE_zlacpy_work(LAPACK_COL_MAJOR,'u', N, N, A2, LDA, L2, N);
        cblas_ztrmm(CblasColMajor, CblasLeft, CblasUpper, CblasConjTrans, CblasNonUnit, N, N, CBLAS_SADDR(alpha), L1, N, L2, N);
    }
    else{
        LAPACKE_zlacpy_work(LAPACK_COL_MAJOR,'l', N, N, A2, LDA, L1, N);
        LAPACKE_zlacpy_work(LAPACK_COL_MAJOR,'l', N, N, A2, LDA, L2, N);
        cblas_ztrmm(CblasColMajor, CblasRight, CblasLower, CblasConjTrans, CblasNonUnit, N, N, CBLAS_SADDR(alpha), L1, N, L2, N);
    }

    /* Compute the Residual || A -L'L|| */
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
           Residual[j*N+i] = L2[j*N+i] - Residual[j*N+i];

    Rnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', N, N, Residual, N, work);
    Anorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', N, N, A1, LDA, work);

    printf("============\n");
    printf("Checking the Cholesky Factorization \n");
    printf("-- ||L'L-A||_oo/(||A||_oo.N.eps) = %e \n",Rnorm/(Anorm*N*eps));

    if ( isnan(Rnorm/(Anorm*N*eps)) || (Rnorm/(Anorm*N*eps) > 10.0) ){
        printf("-- Factorization is suspicious ! \n");
        info_factorization = 1;
    }
    else{
        printf("-- Factorization is CORRECT ! \n");
        info_factorization = 0;
    }

    free(Residual); free(L1); free(L2); free(work);

    return info_factorization;
}

/*--------------------------------------------------------------
 * Check the gemm
 */
double z_check_gemm(MORSE_enum transA, MORSE_enum transB, int M, int N, int K,
                   MORSE_Complex64_t alpha, MORSE_Complex64_t *A, int LDA,
                   MORSE_Complex64_t *B, int LDB,
                   MORSE_Complex64_t beta, MORSE_Complex64_t *Cmorse,
                   MORSE_Complex64_t *Cref, int LDC,
                   double *Cinitnorm, double *Cmorsenorm, double *Clapacknorm )
{
    MORSE_Complex64_t beta_const = -1.0;
    double Rnorm;
    double *work = (double *)malloc(chameleon_max(K,chameleon_max(M, N))* sizeof(double));

    *Cinitnorm   = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Cref,    LDC, work);
    *Cmorsenorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Cmorse, LDC, work);

    cblas_zgemm(CblasColMajor, (CBLAS_TRANSPOSE)transA, (CBLAS_TRANSPOSE)transB, M, N, K,
                CBLAS_SADDR(alpha), A, LDA, B, LDB, CBLAS_SADDR(beta), Cref, LDC);

    *Clapacknorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Cref, LDC, work);

    cblas_zaxpy(LDC * N, CBLAS_SADDR(beta_const), Cmorse, 1, Cref, 1);

    Rnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N, Cref, LDC, work);

    free(work);

    return Rnorm;
}

/*--------------------------------------------------------------
 * Check the trsm
 */
double z_check_trsm(MORSE_enum side, MORSE_enum uplo, MORSE_enum trans, MORSE_enum diag,
                   int M, int NRHS, MORSE_Complex64_t alpha,
                   MORSE_Complex64_t *A, int LDA,
                   MORSE_Complex64_t *Bmorse, MORSE_Complex64_t *Bref, int LDB,
                   double *Binitnorm, double *Bmorsenorm, double *Blapacknorm )
{
    MORSE_Complex64_t beta_const = -1.0;
    double Rnorm;
    double *work = (double *)malloc(chameleon_max(M, NRHS)* sizeof(double));
    /*double eps = LAPACKE_dlamch_work('e');*/

    *Binitnorm   = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'i', M, NRHS, Bref,    LDB, work);
    *Bmorsenorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'i', M, NRHS, Bmorse, LDB, work);

    cblas_ztrsm(CblasColMajor, (CBLAS_SIDE)side, (CBLAS_UPLO)uplo,
                (CBLAS_TRANSPOSE)trans, (CBLAS_DIAG)diag, M, NRHS,
                CBLAS_SADDR(alpha), A, LDA, Bref, LDB);

    *Blapacknorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'i', M, NRHS, Bref, LDB, work);

    cblas_zaxpy(LDB * NRHS, CBLAS_SADDR(beta_const), Bmorse, 1, Bref, 1);

    Rnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'i', M, NRHS, Bref, LDB, work);
    Rnorm = Rnorm / *Blapacknorm;
    /* chameleon_max(M,NRHS) * eps);*/

    free(work);

    return Rnorm;
}

/*--------------------------------------------------------------
 * Check the solution
 */

double z_check_solution(int M, int N, int NRHS, MORSE_Complex64_t *A, int LDA,
                      MORSE_Complex64_t *B,  MORSE_Complex64_t *X, int LDB,
                      double *anorm, double *bnorm, double *xnorm )
{
/*     int info_solution; */
    double Rnorm = -1.00;
    MORSE_Complex64_t zone  =  1.0;
    MORSE_Complex64_t mzone = -1.0;
    double *work = (double *)malloc(chameleon_max(M, N)* sizeof(double));

    *anorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, N,    A, LDA, work);
    *xnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', M, NRHS, X, LDB, work);
    *bnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', N, NRHS, B, LDB, work);

    cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, M, NRHS, N, CBLAS_SADDR(zone), A, LDA, X, LDB, CBLAS_SADDR(mzone), B, LDB);

    Rnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'I', N, NRHS, B, LDB, work);

    free(work);

    return Rnorm;
}

/*------------------------------------------------------------------------
 *  *  Check the accuracy of the computed inverse
 *   */

int zcheck_inverse(int N, MORSE_Complex64_t *A1, MORSE_Complex64_t *A2, int LDA,
                        MORSE_enum uplo, double *rnorm, double *anorm, double *ainvnorm )
{
    int info_inverse;
    int i, j;
    double result;
    MORSE_Complex64_t alpha, beta, zone;
    MORSE_Complex64_t *workz = (MORSE_Complex64_t *)malloc(N*N*sizeof(MORSE_Complex64_t));
    double *workd = (double *)malloc(N*sizeof(double));
    double eps;

    eps = LAPACKE_dlamch_work('e');

    alpha = -1.0;
    beta  = 0.0;
    zone = 1.0;

    /* Rebuild the other part of the inverse matrix */
    if(uplo == MorseUpper){
       for(j=0; j<N; j++)
          for(i=0; i<j; i++)
             *(A2+j+i*LDA) = *(A2+i+j*LDA);
       cblas_zhemm(CblasColMajor, CblasLeft, CblasUpper, N, N, CBLAS_SADDR(alpha), A2, LDA, A1, LDA, CBLAS_SADDR(beta), workz, N);

    }
    else {
       for(j=0; j<N; j++)
          for(i=j; i<N; i++)
             *(A2+j+i*LDA) = *(A2+i+j*LDA);
       cblas_zhemm(CblasColMajor, CblasLeft, CblasLower, N, N, CBLAS_SADDR(alpha), A2, LDA, A1, LDA, CBLAS_SADDR(beta), workz, N);
    }

    /* Add the identity matrix to workz */
     for(i=0; i<N; i++)
        *(workz+i+i*N) = *(workz+i+i*N) + zone;


    *rnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'O', N, N,    workz, N, workd);
    *anorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'O', N, N, A1, LDA, workd);
    *ainvnorm = LAPACKE_zlange_work(LAPACK_COL_MAJOR, 'O', N, N, A2, LDA, workd);


      result = *rnorm / ( ((*anorm) * (*ainvnorm))*N*eps ) ;

    if (  isnan(*ainvnorm) || isinf(*ainvnorm) || isnan(result) || isinf(result) || (result > 60.0) ) {
        info_inverse = 1;
     }
    else{
        info_inverse = 0;
    }

    free(workz);
    free(workd);

    return info_inverse;
}

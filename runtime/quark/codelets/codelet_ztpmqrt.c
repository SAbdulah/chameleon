/**
 *
 * @copyright (c) 2009-2016 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                          Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file codelet_ztpqrt.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 0.9.0
 * @author Mathieu Faverge
 * @date 2016-12-15
 * @precisions normal z -> s d c
 *
 **/
#include "chameleon_quark.h"
#include "chameleon/morse_tasks_z.h"
#include "coreblas/coreblas_z.h"

static void
CORE_ztpmqrt_quark( Quark *quark )
{
    MORSE_enum side;
    MORSE_enum trans;
    int M;
    int N;
    int K;
    int L;
    int ib;
    const MORSE_Complex64_t *V;
    int ldv;
    const MORSE_Complex64_t *T;
    int ldt;
    MORSE_Complex64_t *A;
    int lda;
    MORSE_Complex64_t *B;
    int ldb;
    MORSE_Complex64_t *WORK;

    quark_unpack_args_16( quark, side, trans, M, N, K, L, ib,
                          V, ldv, T, ldt, A, lda, B, ldb, WORK );

    CORE_ztpmqrt( side, trans, M, N, K, L, ib,
                  V, ldv, T, ldt, A, lda, B, ldb, WORK );
}

void MORSE_TASK_ztpmqrt( const MORSE_option_t *options,
                         MORSE_enum side, MORSE_enum trans,
                         int M, int N, int K, int L, int ib, int nb,
                         const MORSE_desc_t *V, int Vm, int Vn, int ldv,
                         const MORSE_desc_t *T, int Tm, int Tn, int ldt,
                         const MORSE_desc_t *A, int Am, int An, int lda,
                         const MORSE_desc_t *B, int Bm, int Bn, int ldb )
{
    quark_option_t *opt = (quark_option_t*)(options->schedopt);
    DAG_CORE_TSMQR;

    QUARK_Insert_Task(
        opt->quark, CORE_ztpmqrt_quark, (Quark_Task_Flags*)opt,
        sizeof(MORSE_enum),              &side,  VALUE,
        sizeof(MORSE_enum),              &trans, VALUE,
        sizeof(int),                     &M,     VALUE,
        sizeof(int),                     &N,     VALUE,
        sizeof(int),                     &K,     VALUE,
        sizeof(int),                     &L,     VALUE,
        sizeof(int),                     &ib,    VALUE,
        sizeof(MORSE_Complex64_t)*nb*nb,  RTBLKADDR( V, MORSE_Complex64_t, Vm, Vn ), INPUT,
        sizeof(int),                     &ldv,   VALUE,
        sizeof(MORSE_Complex64_t)*ib*nb,  RTBLKADDR( T, MORSE_Complex64_t, Tm, Tn ), INPUT,
        sizeof(int),                     &ldt,   VALUE,
        sizeof(MORSE_Complex64_t)*nb*nb,  RTBLKADDR( A, MORSE_Complex64_t, Am, An ), INOUT,
        sizeof(int),                     &lda,   VALUE,
        sizeof(MORSE_Complex64_t)*nb*nb,  RTBLKADDR( B, MORSE_Complex64_t, Bm, Bn ), INOUT,
        sizeof(int),                     &ldb,   VALUE,
        sizeof(MORSE_Complex64_t)*ib*nb,  NULL, SCRATCH,
        0);
}

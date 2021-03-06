/**
 *
 * @copyright (c) 2009-2015 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2015 Inria. All rights reserved.
 * @copyright (c) 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 * @file codelet_zttlqt.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @author Reazul Hoque
 * @precisions normal z -> c d s
 *
 **/
#include "chameleon_parsec.h"
#include "chameleon/morse_tasks_z.h"
#include "coreblas/coreblas_z.h"

static int
CORE_zttlqt_parsec(parsec_execution_stream_t *context, parsec_task_t *this_task)
{
    int *m;
    int *n;
    int *ib;
    MORSE_Complex64_t *A1;
    int *lda1;
    MORSE_Complex64_t *A2;
    int *lda2;
    MORSE_Complex64_t *T;
    int *ldt;
    MORSE_Complex64_t *TAU;
    MORSE_Complex64_t *WORK;

    parsec_dtd_unpack_args(
        this_task,
        UNPACK_VALUE,   &m,
        UNPACK_VALUE,   &n,
        UNPACK_VALUE,   &ib,
        UNPACK_DATA,    &A1,
        UNPACK_VALUE,   &lda1,
        UNPACK_DATA,    &A2,
        UNPACK_VALUE,   &lda2,
        UNPACK_DATA,    &T,
        UNPACK_VALUE,   &ldt,
        UNPACK_SCRATCH, &TAU,
        UNPACK_SCRATCH, &WORK );

    CORE_zttlqt(*m, *n, *ib, A1, *lda1, A2, *lda2, T, *ldt, TAU, WORK);

    return 0;
}

void MORSE_TASK_zttlqt(const MORSE_option_t *options,
                       int m, int n, int ib, int nb,
                       const MORSE_desc_t *A1, int A1m, int A1n, int lda1,
                       const MORSE_desc_t *A2, int A2m, int A2n, int lda2,
                       const MORSE_desc_t *T, int Tm, int Tn, int ldt)
{

    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_zttlqt_parsec, options->priority, "ttlqt",
        sizeof(int),            &m,                     VALUE,
        sizeof(int),            &n,                     VALUE,
        sizeof(int),            &ib,                    VALUE,
        PASSED_BY_REF,          RTBLKADDR( A1, MORSE_Complex64_t, A1m, A1n ),    INOUT,
        sizeof(int),            &lda1,                  VALUE,
        PASSED_BY_REF,          RTBLKADDR( A2, MORSE_Complex64_t, A2m, A2n ),    INOUT,
        sizeof(int),            &lda2,                  VALUE,
        PASSED_BY_REF,          RTBLKADDR( T, MORSE_Complex64_t, Tm, Tn ),       OUTPUT,
        sizeof(int),            &ldt,                   VALUE,
        sizeof(MORSE_Complex64_t)*nb,       NULL,       SCRATCH,
        sizeof(MORSE_Complex64_t)*ib*nb,    NULL,       SCRATCH,
        0);
}

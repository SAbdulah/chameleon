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
 * @file codelet_zlange.c
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
CORE_zlange_parsec(parsec_execution_stream_t *context, parsec_task_t *this_task)
{
    MORSE_enum *norm;
    int *M;
    int *N;
    MORSE_Complex64_t *A;
    int *LDA;
    double *work;
    double *normA;

    parsec_dtd_unpack_args(
        this_task,
        UNPACK_VALUE,   &norm,
        UNPACK_VALUE,   &M,
        UNPACK_VALUE,   &N,
        UNPACK_DATA,    &A,
        UNPACK_VALUE,   &LDA,
        UNPACK_SCRATCH, &work,
        UNPACK_DATA,    &normA );

    CORE_zlange( *norm, *M, *N, A, *LDA, work, normA );

    return 0;
}

void MORSE_TASK_zlange(const MORSE_option_t *options,
                       MORSE_enum norm, int M, int N, int NB,
                       const MORSE_desc_t *A, int Am, int An, int LDA,
                       const MORSE_desc_t *B, int Bm, int Bn)
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    int szeW = chameleon_max( M, N );

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_zlange_parsec, options->priority, "lange",
        sizeof(MORSE_enum),            &norm,          VALUE,
        sizeof(int),                   &M,             VALUE,
        sizeof(int),                   &N,             VALUE,
        PASSED_BY_REF,         RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     INPUT,
        sizeof(int),                   &LDA,           VALUE,
        sizeof(double)*szeW,           NULL,           SCRATCH,
        PASSED_BY_REF,         RTBLKADDR( B, double, Bm, Bn ),     OUTPUT,
        0);
}

#if defined(PRECISION_d) || defined(PRECISION_s)
static int
CORE_zlange_max_parsec(parsec_execution_stream_t *context, parsec_task_t *this_task)
{
    double *A;
    double *normA;

    parsec_dtd_unpack_args(
        this_task,
        UNPACK_DATA,  &A,
        UNPACK_DATA,  &normA );

    if ( *A > *normA )
        *normA = *A;

    return 0;
}

void MORSE_TASK_zlange_max(const MORSE_option_t *options,
                           const MORSE_desc_t *A, int Am, int An,
                           const MORSE_desc_t *B, int Bm, int Bn)
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_zlange_max_parsec, options->priority, "lange_max",
        PASSED_BY_REF,         RTBLKADDR( A, double, Am, An ), INPUT,
        PASSED_BY_REF,         RTBLKADDR( B, double, Bm, Bn ), OUTPUT,
        0);
}

#endif /* defined(PRECISION_d) || defined(PRECISION_s) */

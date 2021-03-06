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
 * @file codelet_zlag2c.c
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

/***************************************************************************//**
 *
 * @ingroup CORE_MORSE_Complex64_t
 *
 **/
static int
CORE_zlag2c_parsec(parsec_execution_stream_t *context, parsec_task_t *this_task)
{
    int *m;
    int *n;
    MORSE_Complex64_t *A;
    int *lda;
    MORSE_Complex32_t *B;
    int *ldb;
    int info;

    parsec_dtd_unpack_args(
        this_task,
        UNPACK_VALUE, &m,
        UNPACK_VALUE, &n,
        UNPACK_DATA,  &A,
        UNPACK_VALUE, &lda,
        UNPACK_DATA,  &B,
        UNPACK_VALUE, &ldb );

    CORE_zlag2c( *m, *n, A, *lda, B, *ldb);

    return 0;
}

void MORSE_TASK_zlag2c(const MORSE_option_t *options,
                       int m, int n, int nb,
                       const MORSE_desc_t *A, int Am, int An, int lda,
                       const MORSE_desc_t *B, int Bm, int Bn, int ldb)
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    parsec_dtd_taskpool_insert_task(PARSEC_dtd_taskpool, CORE_zlag2c_parsec, "lag2c",
        sizeof(int),                        &m,         VALUE,
        sizeof(int),                        &n,         VALUE,
        PASSED_BY_REF,         RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     INPUT,
        sizeof(int),                        &lda,       VALUE,
        PASSED_BY_REF,         RTBLKADDR( B, MORSE_Complex32_t, Bm, Bn ),     OUTPUT,
        sizeof(int),                        &ldb,       VALUE,
        0);
}

/***************************************************************************//**
 *
 * @ingroup CORE_MORSE_Complex64_t
 *
 **/
static int
CORE_clag2z_parsec(parsec_execution_stream_t *context, parsec_task_t *this_task)
{
    int *m;
    int *n;
    MORSE_Complex32_t *A;
    int *lda;
    MORSE_Complex64_t *B;
    int *ldb;

    parsec_dtd_unpack_args(
        this_task,
        UNPACK_VALUE, &m,
        UNPACK_VALUE, &n,
        UNPACK_DATA,  &A,
        UNPACK_VALUE, &lda,
        UNPACK_DATA,  &B,
        UNPACK_VALUE, &ldb );

    CORE_clag2z( *m, *n, A, *lda, B, *ldb );

    return 0;
}

void MORSE_TASK_clag2z(const MORSE_option_t *options,
                       int m, int n, int nb,
                       const MORSE_desc_t *A, int Am, int An, int lda,
                       const MORSE_desc_t *B, int Bm, int Bn, int ldb)
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_clag2z_parsec, options->priority, "lag2z",
        sizeof(int),                        &m,         VALUE,
        sizeof(int),                        &n,         VALUE,
        PASSED_BY_REF,         RTBLKADDR( A, MORSE_Complex32_t, Am, An ),     INPUT,
        sizeof(int),                        &lda,       VALUE,
        PASSED_BY_REF,         RTBLKADDR( B, MORSE_Complex64_t, Bm, Bn ),     INOUT,
        sizeof(int),                        &ldb,       VALUE,
        0);
}

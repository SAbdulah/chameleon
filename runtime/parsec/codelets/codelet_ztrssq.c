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
 * @file codelet_ztrssq.c
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
CORE_ztrssq_parsec(parsec_execution_stream_t *context, parsec_task_t *this_task)
{
    MORSE_enum *uplo;
    MORSE_enum *diag;
    int *m;
    int *n;
    MORSE_Complex64_t *A;
    int *lda;
    double *SCALESUMSQ;

    parsec_dtd_unpack_args(
        this_task,
        UNPACK_VALUE, &uplo,
        UNPACK_VALUE, &diag,
        UNPACK_VALUE, &m,
        UNPACK_VALUE, &n,
        UNPACK_DATA,  &A,
        UNPACK_VALUE, &lda,
        UNPACK_DATA,  &SCALESUMSQ );

    CORE_ztrssq( *uplo, *diag, *m, *n, A, *lda, &SCALESUMSQ[0], &SCALESUMSQ[1]);

    return 0;
}

void MORSE_TASK_ztrssq( const MORSE_option_t *options,
                        MORSE_enum uplo, MORSE_enum diag,
                        int m, int n,
                        const MORSE_desc_t *A, int Am, int An, int lda,
                        const MORSE_desc_t *SCALESUMSQ, int SCALESUMSQm, int SCALESUMSQn )
{
    parsec_taskpool_t* PARSEC_dtd_taskpool = (parsec_taskpool_t *)(options->sequence->schedopt);

    parsec_dtd_taskpool_insert_task(
        PARSEC_dtd_taskpool, CORE_ztrssq_parsec, options->priority, "trssq",
        sizeof(MORSE_enum),     &uplo,                  VALUE,
        sizeof(MORSE_enum),     &diag,                  VALUE,
        sizeof(int),            &m,                     VALUE,
        sizeof(int),            &n,                     VALUE,
        PASSED_BY_REF,          RTBLKADDR( A, MORSE_Complex64_t, Am, An ),                    INPUT,
        sizeof(int),            &lda,                   VALUE,
        PASSED_BY_REF,          RTBLKADDR( SCALESUMSQ, double, SCALESUMSQm, SCALESUMSQn ),    INOUT,
        0);
}

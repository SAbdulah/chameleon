/**
 *
 * @copyright (c) 2009-2015 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2015 Inria. All rights reserved.
 * @copyright (c) 2012-2015 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
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
#include "runtime/parsec/include/morse_parsec.h"

static int
CORE_zlange_parsec(dague_execution_unit_t *context, dague_execution_context_t *this_task)
{
    MORSE_enum *norm;
    int *M;
    int *N;
    MORSE_Complex64_t *A;
    int *LDA;
    double *work;
    double *normA;

    dague_dtd_unpack_args(this_task,
                          UNPACK_VALUE, &norm,
                          UNPACK_VALUE, &M,
                          UNPACK_VALUE, &N,
                          UNPACK_DATA,  &A,
                          UNPACK_VALUE, &LDA,
                          UNPACK_SCRATCH, &work,
                          UNPACK_DATA,  &normA
                        );


    CORE_zlange( *norm, *M, *N, A, *LDA, work, normA );

    return 0;
}

void MORSE_TASK_zlange(MORSE_option_t *options,
                       MORSE_enum norm, int M, int N, int NB,
                       MORSE_desc_t *A, int Am, int An, int LDA,
                       MORSE_desc_t *B, int Bm, int Bn)
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    int szeW = max( M, N );

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_zlange_parsec,    "lange",
                             sizeof(MORSE_enum),            &norm,          VALUE,
                             sizeof(int),                   &M,             VALUE,
                             sizeof(int),                   &N,             VALUE,
                             PASSED_BY_REF,         RTBLKADDR( A, MORSE_Complex64_t, Am, An ),     INPUT | REGION_FULL,
                             sizeof(int),                   &LDA,           VALUE,
                             sizeof(double)*szeW,           NULL,           SCRATCH,
                             PASSED_BY_REF,         RTBLKADDR( B, double, Bm, Bn ),     OUTPUT | REGION_FULL,
                             0);
}

#if defined(PRECISION_d) || defined(PRECISION_s)
static int
CORE_zlange_max_parsec(dague_execution_unit_t *context, dague_execution_context_t *this_task)
{
    double *A;
    double *normA;

    dague_dtd_unpack_args(this_task,
                          UNPACK_DATA,  &A,
                          UNPACK_DATA,  &normA
                        );


    if ( *A > *normA )
        *normA = *A;

    return 0;
}

void MORSE_TASK_zlange_max(MORSE_option_t *options,
                           MORSE_desc_t *A, int Am, int An,
                           MORSE_desc_t *B, int Bm, int Bn)
{
    dague_dtd_handle_t* DAGUE_dtd_handle = (dague_dtd_handle_t *)(options->sequence->schedopt);

    insert_task_generic_fptr(DAGUE_dtd_handle,      CORE_zlange_max_parsec,               "lange_max",
                             PASSED_BY_REF,         RTBLKADDR( A, double, Am, An ), INPUT | REGION_FULL,
                             PASSED_BY_REF,         RTBLKADDR( B, double, Bm, Bn ), OUTPUT | REGION_FULL,
                             0);
}

#endif /* defined(PRECISION_d) || defined(PRECISION_s) */
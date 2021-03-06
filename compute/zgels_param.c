/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2017 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file zgels_param.c
 *
 *  MORSE computational routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 1.0.0
 * @author Raphael Boucherie
 * @author Mathieu Faverge
 * @date 2017-05-17
 * @precisions normal z -> s d c
 *
 **/
#include "control/common.h"
#include <stdlib.h>

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t
 *
 *  MORSE_zgels_param - solves overdetermined or underdetermined linear systems involving an M-by-N
 *  matrix A using the QR or the LQ factorization of A.  It is assumed that A has full rank.
 *  The following options are provided:
 *
 *  # trans = MorseNoTrans and M >= N: find the least squares solution of an overdetermined
 *    system, i.e., solve the least squares problem: minimize || B - A*X ||.
 *
 *  # trans = MorseNoTrans and M < N:  find the minimum norm solution of an underdetermined
 *    system A * X = B.
 *
 *  Several right hand side vectors B and solution vectors X can be handled in a single call;
 *  they are stored as the columns of the M-by-NRHS right hand side matrix B and the N-by-NRHS
 *  solution matrix X.
 *
 *******************************************************************************
 *
 * @param[in] qrtree
 *          The tree used for the factorization
 *
 * @param[in] trans
 *          Intended usage:
 *          = MorseNoTrans:   the linear system involves A;
 *          = MorseConjTrans: the linear system involves A**H.
 *          Currently only MorseNoTrans is supported.
 *
 * @param[in] M
 *          The number of rows of the matrix A. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrix A. N >= 0.
 *
 * @param[in] NRHS
 *          The number of right hand sides, i.e., the number of columns of the matrices B and X.
 *          NRHS >= 0.
 *
 * @param[in,out] A
 *          On entry, the M-by-N matrix A.
 *          On exit,
 *          if M >= N, A is overwritten by details of its QR factorization as returned by
 *                     MORSE_zgeqrf;
 *          if M < N, A is overwritten by details of its LQ factorization as returned by
 *                      MORSE_zgelqf.
 *
 * @param[in] LDA
 *          The leading dimension of the array A. LDA >= max(1,M).
 *
 * @param[out] descTS
 *          On exit, auxiliary factorization data.
 *
 * @param[out] descTT
 *          On exit, auxiliary factorization data.
 *
 * @param[in,out] B
 *          On entry, the M-by-NRHS matrix B of right hand side vectors, stored columnwise;
 *          On exit, if return value = 0, B is overwritten by the solution vectors, stored
 *          columnwise:
 *          if M >= N, rows 1 to N of B contain the least squares solution vectors; the residual
 *          sum of squares for the solution in each column is given by the sum of squares of the
 *          modulus of elements N+1 to M in that column;
 *          if M < N, rows 1 to N of B contain the minimum norm solution vectors;
 *
 * @param[in] LDB
 *          The leading dimension of the array B. LDB >= MAX(1,M,N).
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa MORSE_zgels_param_Tile
 * @sa MORSE_zgels_param_Tile_Async
 * @sa MORSE_cgels
 * @sa MORSE_dgels
 * @sa MORSE_sgels
 *
 ******************************************************************************/
int MORSE_zgels_param(const libhqr_tree_t *qrtree, MORSE_enum trans, int M, int N, int NRHS,
                      MORSE_Complex64_t *A, int LDA,
                      MORSE_desc_t *descTS, MORSE_desc_t *descTT,
                      MORSE_Complex64_t *B, int LDB)
{
    int i, j;
    int NB;
    int status;
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    MORSE_desc_t descA, descB;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zgels_param", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }

    /* Check input arguments */
    if (trans != MorseNoTrans) {
        morse_error("MORSE_zgels_param", "only MorseNoTrans supported");
        return MORSE_ERR_NOT_SUPPORTED;
    }
    if (M < 0) {
        morse_error("MORSE_zgels_param", "illegal value of M");
        return -2;
    }
    if (N < 0) {
        morse_error("MORSE_zgels_param", "illegal value of N");
        return -3;
    }
    if (NRHS < 0) {
        morse_error("MORSE_zgels_param", "illegal value of NRHS");
        return -4;
    }
    if (LDA < chameleon_max(1, M)) {
        morse_error("MORSE_zgels_param", "illegal value of LDA");
        return -6;
    }
    if (LDB < chameleon_max(1, chameleon_max(M, N))) {
        morse_error("MORSE_zgels_param", "illegal value of LDB");
        return -9;
    }
    /* Quick return */
    if (chameleon_min(M, chameleon_min(N, NRHS)) == 0) {
        for (i = 0; i < chameleon_max(M, N); i++)
            for (j = 0; j < NRHS; j++)
                B[j*LDB+i] = 0.0;
        return MORSE_SUCCESS;
    }

    /* Tune NB & IB depending on M, N & NRHS; Set NBNB */
    status = morse_tune(MORSE_FUNC_ZGELS, M, N, NRHS);
    if (status != MORSE_SUCCESS) {
        morse_error("MORSE_zgels_param", "morse_tune() failed");
        return status;
    }

    /* Set NT */
    NB = MORSE_NB;

    morse_sequence_create(morse, &sequence);

    if ( M >= N ) {
        /*        if ( MORSE_TRANSLATION == MORSE_OUTOFPLACE ) {*/
        morse_zooplap2tile( descA, A, NB, NB, LDA, N,    0, 0, M, N, sequence, &request,
                            morse_desc_mat_free(&(descA)) );
        morse_zooplap2tile( descB, B, NB, NB, LDB, NRHS, 0, 0, M, NRHS, sequence, &request,
                            morse_desc_mat_free(&(descA)); morse_desc_mat_free(&(descB)));
        /*        } else {*/
        /*            morse_ziplap2tile( descA, A, NB, NB, LDA, N,    0, 0, M, N,*/
        /*                                sequence, &request);*/
        /*            morse_ziplap2tile( descB, B, NB, NB, LDB, NRHS, 0, 0, M, NRHS,*/
        /*                                sequence, &request);*/
        /*        }*/
    } else {
        /*        if ( MORSE_TRANSLATION == MORSE_OUTOFPLACE ) {*/
        morse_zooplap2tile( descA, A, NB, NB, LDA, N,    0, 0, M, N, sequence, &request,
                            morse_desc_mat_free(&(descA)) );
        morse_zooplap2tile( descB, B, NB, NB, LDB, NRHS, 0, 0, N, NRHS, sequence, &request,
                            morse_desc_mat_free(&(descA)); morse_desc_mat_free(&(descB)));
        /*        } else {*/
        /*            morse_ziplap2tile( descA, A, NB, NB, LDA, N,    0, 0, M, N,*/
        /*                                sequence, &request);*/
        /*            morse_ziplap2tile( descB, B, NB, NB, LDB, NRHS, 0, 0, N, NRHS,*/
        /*                                sequence, &request);*/
        /*        }*/
    }

    /* Call the tile interface */
    MORSE_zgels_param_Tile_Async(qrtree, MorseNoTrans, &descA, descTS, descTT, &descB, sequence, &request);

    /*    if ( MORSE_TRANSLATION == MORSE_OUTOFPLACE ) {*/
    morse_zooptile2lap(descA, A, NB, NB, LDA, N,     sequence, &request);
    morse_zooptile2lap(descB, B, NB, NB, LDB, NRHS,  sequence, &request);
    morse_sequence_wait(morse, sequence);
    morse_desc_mat_free(&descA);
    morse_desc_mat_free(&descB);
    /*    } else {*/
    /*        morse_ziptile2lap( descA, A, NB, NB, LDA, N,     sequence, &request);*/
    /*        morse_ziptile2lap( descB, B, NB, NB, LDB, NRHS,  sequence, &request);*/
    /*        morse_sequence_wait(morse, sequence);*/
    /*    }*/

    status = sequence->status;
    morse_sequence_destroy(morse, sequence);
    return status;
}

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t_Tile
 *
 *  MORSE_zgels_param_Tile - Solves overdetermined or underdetermined linear system of equations
 *  using the tile QR or the tile LQ factorization.
 *  Tile equivalent of MORSE_zgels_param().
 *  Operates on matrices stored by tiles.
 *  All matrices are passed through descriptors.
 *  All dimensions are taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in] trans
 *          Intended usage:
 *          = MorseNoTrans:   the linear system involves A;
 *          = MorseConjTrans: the linear system involves A**H.
 *          Currently only MorseNoTrans is supported.
 *
 * @param[in,out] A
 *          On entry, the M-by-N matrix A.
 *          On exit,
 *          if M >= N, A is overwritten by details of its QR factorization as returned by
 *                     MORSE_zgeqrf;
 *          if M < N, A is overwritten by details of its LQ factorization as returned by
 *                      MORSE_zgelqf.
 *
 * @param[out] TS
 *          On exit, auxiliary factorization data.
 *
 * @param[out] TT
 *          On exit, auxiliary factorization data.
 *
 * @param[in,out] B
 *          On entry, the M-by-NRHS matrix B of right hand side vectors, stored columnwise;
 *          On exit, if return value = 0, B is overwritten by the solution vectors, stored
 *          columnwise:
 *          if M >= N, rows 1 to N of B contain the least squares solution vectors; the residual
 *          sum of squares for the solution in each column is given by the sum of squares of the
 *          modulus of elements N+1 to M in that column;
 *          if M < N, rows 1 to N of B contain the minimum norm solution vectors;
 *
 *******************************************************************************
 *
 * @return
 *          \return MORSE_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa MORSE_zgels_param
 * @sa MORSE_zgels_param_Tile_Async
 * @sa MORSE_cgels_Tile
 * @sa MORSE_dgels_Tile
 * @sa MORSE_sgels_Tile
 *
 ******************************************************************************/
int MORSE_zgels_param_Tile(const libhqr_tree_t *qrtree, MORSE_enum trans, MORSE_desc_t *A,
                           MORSE_desc_t *TS, MORSE_desc_t *TT, MORSE_desc_t *B)
{
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zgels_param_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create(morse, &sequence);
    MORSE_zgels_param_Tile_Async(qrtree, trans, A, TS, TT, B, sequence, &request);
    morse_sequence_wait(morse, sequence);
    RUNTIME_desc_getoncpu(A);
    RUNTIME_desc_getoncpu(B);

    status = sequence->status;
    morse_sequence_destroy(morse, sequence);
    return status;
}

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t_Tile_Async
 *
 *  MORSE_zgels_param_Tile_Async - Solves overdetermined or underdetermined linear
 *  system of equations using the tile QR or the tile LQ factorization.
 *  Non-blocking equivalent of MORSE_zgels_param_Tile().
 *  May return before the computation is finished.
 *  Allows for pipelining of operations at runtime.
 *
 *******************************************************************************
 *
 * @param[in] sequence
 *          Identifies the sequence of function calls that this call belongs to
 *          (for completion checks and exception handling purposes).
 *
 * @param[out] request
 *          Identifies this function call (for exception handling purposes).
 *
 *******************************************************************************
 *
 * @sa MORSE_zgels_param
 * @sa MORSE_zgels_param_Tile
 * @sa MORSE_cgels_Tile_Async
 * @sa MORSE_dgels_Tile_Async
 * @sa MORSE_sgels_Tile_Async
 *
 ******************************************************************************/
int MORSE_zgels_param_Tile_Async(const libhqr_tree_t *qrtree, MORSE_enum trans, MORSE_desc_t *A,
                                 MORSE_desc_t *TS, MORSE_desc_t *TT, MORSE_desc_t *B,
                                 MORSE_sequence_t *sequence, MORSE_request_t *request)
{
    MORSE_desc_t *subA;
    MORSE_desc_t *subB;
    MORSE_context_t *morse;
    MORSE_desc_t D, *Dptr = NULL;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zgels_param_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("MORSE_zgels_param_Tile", "NULL sequence");
        return MORSE_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("MORSE_zgels_param_Tile", "NULL request");
        return MORSE_ERR_UNALLOCATED;
    }
    /* Check sequence status */
    if (sequence->status == MORSE_SUCCESS)
        request->status = MORSE_SUCCESS;
    else
        return morse_request_fail(sequence, request, MORSE_ERR_SEQUENCE_FLUSHED);

    /* Check descriptors for correctness */
    if (morse_desc_check(A) != MORSE_SUCCESS) {
        morse_error("MORSE_zgels_param_Tile", "invalid first descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(TS) != MORSE_SUCCESS) {
        morse_error("MORSE_zgels_param_Tile", "invalid second descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(TT) != MORSE_SUCCESS) {
        morse_error("MORSE_zgels_param_Tile", "invalid third descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(B) != MORSE_SUCCESS) {
        morse_error("MORSE_zgels_param_Tile", "invalid fourth descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (A->nb != A->mb || B->nb != B->mb) {
        morse_error("MORSE_zgels_param_Tile", "only square tiles supported");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (trans != MorseNoTrans) {
        morse_error("MORSE_zgels_param_Tile", "only MorseNoTrans supported");
        return morse_request_fail(sequence, request, MORSE_ERR_NOT_SUPPORTED);
    }
    /* Quick return  - currently NOT equivalent to LAPACK's:
     if (chameleon_min(M, chameleon_min(N, NRHS)) == 0) {
     for (i = 0; i < chameleon_max(M, N); i++)
     for (j = 0; j < NRHS; j++)
     B[j*LDB+i] = 0.0;
     return MORSE_SUCCESS;
     }
     */
    if (A->m >= A->n) {

#if defined(CHAMELEON_COPY_DIAG)
        {
            int n = chameleon_min(A->mt, A->nt) * A->nb;
            morse_zdesc_alloc(D, A->mb, A->nb, A->m, n, 0, 0, A->m, n, );
            Dptr = &D;
        }
#endif

        subB = morse_desc_submatrix(B, 0, 0, A->n, B->n);
        subA = morse_desc_submatrix(A, 0, 0, A->n, A->n);

        morse_pzgeqrf_param(qrtree, A, TS, TT, Dptr, sequence, request);
        morse_pzunmqr_param(qrtree, MorseLeft, MorseConjTrans, A, B, TS, TT, Dptr, sequence, request);
        morse_pztrsm(MorseLeft, MorseUpper, MorseNoTrans, MorseNonUnit, 1.0, subA, subB, sequence, request);
    }
    else {
#if defined(CHAMELEON_COPY_DIAG)
        {
            int m = chameleon_min(A->mt, A->nt) * A->mb;
            morse_zdesc_alloc(D, A->mb, A->nb, m, A->n, 0, 0, m, A->n, );
            Dptr = &D;
        }
#endif

        subB = morse_desc_submatrix(B, 0, 0, A->m, B->n);
        subA = morse_desc_submatrix(A, 0, 0, A->m, A->m);

        morse_pzgelqf_param(qrtree, A, TS, TT, Dptr, sequence, request);
        morse_pztrsm(MorseLeft, MorseLower, MorseNoTrans, MorseNonUnit, 1.0, subA, subB, sequence, request);
        morse_pzunmlq_param(qrtree, MorseLeft, MorseConjTrans, A, B, TS, TT, Dptr, sequence, request);
    }

    free(subA);
    free(subB);

    if (Dptr != NULL) {
        morse_desc_mat_free(Dptr);
    }
    (void)D;
    return MORSE_SUCCESS;
}

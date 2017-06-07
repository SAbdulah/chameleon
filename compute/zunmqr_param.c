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
 * @file zunmqr_param.c
 *
 *  MORSE computational routines
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
 * @precisions normal z -> s d c
 *
 **/
#include "control/common.h"

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t
 *
 *  MORSE_zunmqr_param - Overwrites the general complex M-by-N matrix C with
 *
 *                  SIDE = 'L'     SIDE = 'R'
 *  TRANS = 'N':      Q * C          C * Q
 *  TRANS = 'C':      Q**H * C       C * Q**H
 *
 *  where Q is a complex unitary matrix defined as the product of k
 *  elementary reflectors
 *
 *        Q = H(1) H(2) . . . H(k)
 *
 *  as returned by MORSE_zgeqrf. Q is of order M if SIDE = MorseLeft
 *  and of order N if SIDE = MorseRight.
 *
 *******************************************************************************
 *
 * @param[in] side
 *          Intended usage:
 *          = MorseLeft:  apply Q or Q**H from the left;
 *          = MorseRight: apply Q or Q**H from the right.
 *
 * @param[in] trans
 *          Intended usage:
 *          = MorseNoTrans:   no transpose, apply Q;
 *          = MorseConjTrans: conjugate transpose, apply Q**H.
 *
 * @param[in] M
 *          The number of rows of the matrix C. M >= 0.
 *
 * @param[in] N
 *          The number of columns of the matrix C. N >= 0.
 *
 * @param[in] K
 *          The number of elementary reflectors whose product defines
 *          the matrix Q.
 *          If side == MorseLeft,  M >= K >= 0.
 *          If side == MorseRight, N >= K >= 0.
 *
 * @param[in] A
 *          Details of the QR factorization of the original matrix A as returned by MORSE_zgeqrf.
 *
 * @param[in] LDA
 *          The leading dimension of the array A.
 *          If side == MorseLeft,  LDA >= max(1,M).
 *          If side == MorseRight, LDA >= max(1,N).
 *
 * @param[in] descT
 *          Auxiliary factorization data, computed by MORSE_zgeqrf.
 *
 * @param[in,out] C
 *          On entry, the M-by-N matrix C.
 *          On exit, C is overwritten by Q*C or Q**H*C.
 *
 * @param[in] LDC
 *          The leading dimension of the array C. LDC >= max(1,M).
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *          \retval <0 if -i, the i-th argument had an illegal value
 *
 *******************************************************************************
 *
 * @sa MORSE_zunmqr_param_Tile
 * @sa MORSE_zunmqr_param_Tile_Async
 * @sa MORSE_cunmqr
 * @sa MORSE_dormqr
 * @sa MORSE_sormqr
 * @sa MORSE_zgeqrf
 *
 ******************************************************************************/
int MORSE_zunmqr_param(const libhqr_tree_t *qrtree,
                       MORSE_enum side, MORSE_enum trans, int M, int N, int K,
                       MORSE_Complex64_t *A, int LDA,
                       MORSE_desc_t *descTS, MORSE_desc_t *descTT,
                       MORSE_Complex64_t *C, int LDC)
{
    int NB, Am;
    int status;
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    MORSE_desc_t descA, descC;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zunmqr_param", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }

    if ( side == MorseLeft ) {
        Am = M;
    } else {
        Am = N;
    }

    /* Check input arguments */
    if ((side != MorseLeft) && (side != MorseRight)) {
        morse_error("MORSE_zunmqr_param", "illegal value of side");
        return -1;
    }
    if ((trans != MorseConjTrans) && (trans != MorseNoTrans)){
        morse_error("MORSE_zunmqr_param", "illegal value of trans");
        return -2;
    }
    if (M < 0) {
        morse_error("MORSE_zunmqr_param", "illegal value of M");
        return -3;
    }
    if (N < 0) {
        morse_error("MORSE_zunmqr_param", "illegal value of N");
        return -4;
    }
    if ((K < 0) || (K > Am)) {
        morse_error("MORSE_zunmqr_param", "illegal value of K");
        return -5;
    }
    if (LDA < chameleon_max(1, Am)) {
        morse_error("MORSE_zunmqr_param", "illegal value of LDA");
        return -7;
    }
    if (LDC < chameleon_max(1, M)) {
        morse_error("MORSE_zunmqr_param", "illegal value of LDC");
        return -10;
    }
    /* Quick return - currently NOT equivalent to LAPACK's:
     * CALL DLASET( 'Full', MAX( M, N ), NRHS, ZERO, ZERO, C, LDC ) */
    if (chameleon_min(M, chameleon_min(N, K)) == 0)
        return MORSE_SUCCESS;

    /* Tune NB & IB depending on M, K & N; Set NBNB */
    status = morse_tune(MORSE_FUNC_ZGELS, M, K, N);
    if (status != MORSE_SUCCESS) {
        morse_error("MORSE_zunmqr_param", "morse_tune() failed");
        return status;
    }

    /* Set MT, NT & NTRHS */
    NB   = MORSE_NB;
    morse_sequence_create(morse, &sequence);

/*    if ( MORSE_TRANSLATION == MORSE_OUTOFPLACE ) {*/
        morse_zooplap2tile( descA, A, NB, NB, LDA, K, 0, 0, Am, K, sequence, &request,
                             morse_desc_mat_free(&(descA)) );
        morse_zooplap2tile( descC, C, NB, NB, LDC, N, 0, 0, M,  N, sequence, &request,
                             morse_desc_mat_free(&(descA)); morse_desc_mat_free(&(descC)));
/*    } else {*/
/*        morse_ziplap2tile( descA, A, NB, NB, LDA, K, 0, 0, Am, K,*/
/*                            sequence, &request);*/
/*        morse_ziplap2tile( descC, C, NB, NB, LDC, N, 0, 0, M,  N,*/
/*                            sequence, &request);*/
/*    }*/

    /* Call the tile interface */
        MORSE_zunmqr_param_Tile_Async(
            qrtree, side, trans, &descA, descTS, descTT, &descC, sequence, &request);

/*    if ( MORSE_TRANSLATION == MORSE_OUTOFPLACE ) {*/
        morse_zooptile2lap(descC, C, NB, NB, LDC, N,  sequence, &request);
        morse_sequence_wait(morse, sequence);
        morse_desc_mat_free(&descA);
        morse_desc_mat_free(&descC);
/*    } else {*/
/*        morse_ziptile2lap( descA, A, NB, NB, LDA, K,  sequence, &request);*/
/*        morse_ziptile2lap( descC, C, NB, NB, LDC, N,  sequence, &request);*/
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
 *  MORSE_zunmqr_param_Tile - overwrites the general M-by-N matrix C with Q*C, where Q is an orthogonal
 *  matrix (unitary in the complex case) defined as the product of elementary reflectors returned
 *  by MORSE_zgeqrf_Tile Q is of order M.
 *  All matrices are passed through descriptors. All dimensions are taken from the descriptors.
 *
 *******************************************************************************
 *
 * @param[in] side
 *          Intended usage:
 *          = MorseLeft:  apply Q or Q**H from the left;
 *          = MorseRight: apply Q or Q**H from the right.
 *          Currently only MorseLeft is supported.
 *
 * @param[in] trans
 *          Intended usage:
 *          = MorseNoTrans:   no transpose, apply Q;
 *          = MorseConjTrans: conjugate transpose, apply Q**H.
 *          Currently only MorseConjTrans is supported.
 *
 * @param[in] A
 *          Details of the QR factorization of the original matrix A as returned by MORSE_zgeqrf.
 *
 * @param[in] T
 *          Auxiliary factorization data, computed by MORSE_zgeqrf.
 *          Can be obtained with MORSE_Alloc_Workspace_zgeqrf
 *
 * @param[in,out] C
 *          On entry, the M-by-N matrix C.
 *          On exit, C is overwritten by Q*C or Q**H*C.
 *
 *******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *******************************************************************************
 *
 * @sa MORSE_zunmqr_param
 * @sa MORSE_zunmqr_param_Tile_Async
 * @sa MORSE_cunmqr_Tile
 * @sa MORSE_dormqr_Tile
 * @sa MORSE_sormqr_Tile
 * @sa MORSE_zgeqrf_Tile
 *
 ******************************************************************************/
int MORSE_zunmqr_param_Tile(const libhqr_tree_t *qrtree, MORSE_enum side, MORSE_enum trans,
                            MORSE_desc_t *A, MORSE_desc_t *TS, MORSE_desc_t *TT, MORSE_desc_t *C)
{
    MORSE_context_t *morse;
    MORSE_sequence_t *sequence = NULL;
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    int status;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zunmqr_param_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    morse_sequence_create(morse, &sequence);
    MORSE_zunmqr_param_Tile_Async(qrtree, side, trans, A, TS, TT, C, sequence, &request);
    morse_sequence_wait(morse, sequence);
    RUNTIME_desc_getoncpu(A);
        RUNTIME_desc_getoncpu(C);

    status = sequence->status;
    morse_sequence_destroy(morse, sequence);
    return status;
}

/**
 *******************************************************************************
 *
 * @ingroup MORSE_Complex64_t_Tile_Async
 *
 *  Non-blocking equivalent of MORSE_zunmqr_param_Tile().
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
 * @sa MORSE_zunmqr_param
 * @sa MORSE_zunmqr_param_Tile
 * @sa MORSE_cunmqr_Tile_Async
 * @sa MORSE_dormqr_Tile_Async
 * @sa MORSE_sormqr_Tile_Async
 * @sa MORSE_zgeqrf_Tile_Async
 *
 ******************************************************************************/
int MORSE_zunmqr_param_Tile_Async(const libhqr_tree_t *qrtree,
                                  MORSE_enum side, MORSE_enum trans,
                                  MORSE_desc_t *A, MORSE_desc_t *TS, MORSE_desc_t *TT, MORSE_desc_t *C,
                                  MORSE_sequence_t *sequence, MORSE_request_t *request)
{
    MORSE_context_t *morse;
    MORSE_desc_t D;

    morse = morse_context_self();
    if (morse == NULL) {
        morse_fatal_error("MORSE_zunmqr_param_Tile", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    if (sequence == NULL) {
        morse_fatal_error("MORSE_zunmqr_param_Tile", "NULL sequence");
        return MORSE_ERR_UNALLOCATED;
    }
    if (request == NULL) {
        morse_fatal_error("MORSE_zunmqr_param_Tile", "NULL request");
        return MORSE_ERR_UNALLOCATED;
    }
    /* Check sequence status */
    if (sequence->status == MORSE_SUCCESS)
        request->status = MORSE_SUCCESS;
    else
        return morse_request_fail(sequence, request, MORSE_ERR_SEQUENCE_FLUSHED);

    /* Check descriptors for correctness */
    if (morse_desc_check(A) != MORSE_SUCCESS) {
        morse_error("MORSE_zunmqr_param_Tile", "invalid first descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(TS) != MORSE_SUCCESS) {
        morse_error("MORSE_zunmqr_param_Tile", "invalid second descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(TT) != MORSE_SUCCESS) {
        morse_error("MORSE_zunmqr_param_Tile", "invalid third descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if (morse_desc_check(C) != MORSE_SUCCESS) {
        morse_error("MORSE_zunmqr_param_Tile", "invalid fourth descriptor");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    /* Check input arguments */
    if (A->nb != A->mb || C->nb != C->mb) {
        morse_error("MORSE_zunmqr_param_Tile", "only square tiles supported");
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if ((side != MorseLeft) && (side != MorseRight)) {
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    if ((trans != MorseConjTrans) && (trans != MorseNoTrans)){
        return morse_request_fail(sequence, request, MORSE_ERR_ILLEGAL_VALUE);
    }
    /* Quick return - currently NOT equivalent to LAPACK's:
     * CALL DLASET( 'Full', MAX( M, N ), NRHS, ZERO, ZERO, C, LDC ) */
/*
    if (chameleon_min(M, chameleon_min(N, K)) == 0)
        return MORSE_SUCCESS;
*/

#if defined(CHAMELEON_COPY_DIAG)
    morse_zdesc_alloc(D, A->mb, A->nb, A->m, chameleon_min(A->m, A->n), 0, 0, A->m, chameleon_min(A->m, A->n), );
    morse_pzunmqr_param(qrtree, side, trans, A, C, TS, TT, &D, sequence, request);
    morse_desc_mat_free(&D);
#else
    morse_pzunmqr_param(qrtree, side, trans, A, C, TS, TT, NULL, sequence, request);
#endif
    (void)D;
    return MORSE_SUCCESS;
}

/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2016 Inria. All rights reserved.
 * @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file pzunglqrh.c
 *
 *  MORSE auxiliary routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for MORSE 1.0.0
 * @author Dulceneia Becker
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2011-05-24
 * @precisions normal z -> s d c
 *
 **/
#include "control/common.h"

#define A(m,n) A,  (m),  (n)
#define Q(m,n) Q,  (m),  (n)
#define T(m,n) T,  (m),  (n)
#define T2(m,n) T,  (m),  (n)+(A->nt)
#if defined(CHAMELEON_COPY_DIAG)
#define D(m,n) D, ((n)/BS), 0
#else
#define D(m,n) A, (m), (n)
#endif

/**
 *  Parallel construction of Q using tile V (application to identity;
 *  reduction Householder) - dynamic scheduling
 **/
void morse_pzunglqrh(MORSE_desc_t *A, MORSE_desc_t *Q,
                     MORSE_desc_t *T, MORSE_desc_t *D, int BS,
                     MORSE_sequence_t *sequence, MORSE_request_t *request)
{
    MORSE_context_t *morse;
    MORSE_option_t options;
    size_t ws_worker = 0;
    size_t ws_host = 0;

    int k, m, n;
    int K, N, RD, lastRD;
    int ldak;
    int ldqm;
    int tempkm, tempkmin, tempNn, tempnn, tempmm, tempNRDn;
    int ib;

    morse = morse_context_self();
    if (sequence->status != MORSE_SUCCESS)
        return;
    RUNTIME_options_init(&options, morse, sequence, request);

    ib = MORSE_IB;

    /*
     * zunmqr = A->nb * ib
     * ztsmqr = A->nb * ib
     * zttmqr = A->nb * ib
     */
    ws_worker = A->nb * ib;

#if defined(CHAMELEON_USE_CUDA)
    /* Worker space
     *
     * zunmqr = A->nb * ib
     * ztsmqr = 2 * A->nb * ib
     */
    ws_worker = chameleon_max( ws_worker, ib * A->nb * 2 );
#endif

    ws_worker *= sizeof(MORSE_Complex64_t);
    ws_host   *= sizeof(MORSE_Complex64_t);

    RUNTIME_options_ws_alloc( &options, ws_worker, ws_host );

    K = chameleon_min(A->mt, A->nt);
    for (k = K-1; k >= 0; k--) {
        RUNTIME_iteration_push(morse, k);

        tempkm = k == A->mt-1 ? A->m-k*A->mb : A->mb;
        ldak = BLKLDD(A, k);
        lastRD = 0;
        for (RD = BS; RD < A->nt-k; RD *= 2)
            lastRD = RD;
        for (RD = lastRD; RD >= BS; RD /= 2) {
            for (N = k; N+RD < A->nt; N += 2*RD) {
                tempNRDn = N+RD == A->nt-1 ? A->n-(N+RD)*A->nb : A->nb;
                for (m = k; m < Q->mt; m++) {
                    tempmm = m == Q->mt-1 ? Q->m-m*Q->mb : Q->mb;
                    ldqm   = BLKLDD(Q, m   );
                    MORSE_TASK_zttmlq(
                        &options,
                        MorseRight, MorseNoTrans,
                        tempmm, Q->nb, tempmm, tempNRDn,
                        tempkm, ib, T->nb,
                        Q (m, N   ), ldqm,
                        Q (m, N+RD), ldqm,
                        A (k, N+RD), ldak,
                        T2(k, N+RD), T->mb);
                }
            }
        }
        for (N = k; N < A->nt; N += BS) {
            tempNn = N == A->nt-1 ? A->n-N*A->nb : A->nb;
            tempkmin = chameleon_min(tempkm, tempNn);
            for (n = chameleon_min(N+BS, A->nt)-1; n > N; n--) {
                tempnn = n == Q->nt-1 ? Q->n-n*Q->nb : Q->nb;

                for (m = k; m < Q->mt; m++) {
                    tempmm = m == Q->mt-1 ? Q->m-m*Q->mb : Q->mb;
                    ldqm = BLKLDD(Q, m);
                    MORSE_TASK_ztsmlq(
                        &options,
                        MorseRight, MorseNoTrans,
                        tempmm, Q->nb, tempmm, tempnn,
                        tempkm, ib, T->nb,
                        Q(m, N), ldqm,
                        Q(m, n), ldqm,
                        A(k, n), ldak,
                        T(k, n), T->mb);
                }
            }
#if defined(CHAMELEON_COPY_DIAG)
            MORSE_TASK_zlacpy(
                &options,
                MorseUpper, tempkmin, tempNn, A->nb,
                A(k, N), ldak,
                D(k, N), ldak );
#if defined(CHAMELEON_USE_CUDA)
            MORSE_TASK_zlaset(
                &options,
                MorseLower, tempkmin, tempNn,
                0., 1.,
                D(k, N), ldak );
#endif
#endif
            for (m = k; m < Q->mt; m++) {
                tempmm = m == Q->mt-1 ? Q->m-m*Q->mb : Q->mb;
                ldqm = BLKLDD(Q, m);
                MORSE_TASK_zunmlq(
                    &options,
                    MorseRight, MorseNoTrans,
                    tempmm, tempNn,
                    tempkmin, ib, T->nb,
                    D(k, N), ldak,
                    T(k, N), T->mb,
                    Q(m, N), ldqm);
            }
        }
        RUNTIME_iteration_pop(morse);
    }

    MORSE_TASK_flush_desc( &options, MorseUpperLower, A );
    MORSE_TASK_flush_desc( &options, MorseUpperLower, Q );
    MORSE_TASK_flush_desc( &options, MorseUpper,      T );
    MORSE_TASK_flush_desc( &options, MorseUpperLower, D );
    RUNTIME_options_ws_free(&options);
    RUNTIME_options_finalize(&options, morse);
    (void)D;
}

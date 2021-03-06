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
 * @file pzsymm.c
 *
 *  MORSE auxiliary routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for MORSE 1.0.0
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @date 2010-11-15
 * @precisions normal z -> s d c
 *
 **/
#include "control/common.h"

#define A(m,n) A,  m,  n
#define B(m,n) B,  m,  n
#define C(m,n) C,  m,  n
/***************************************************************************//**
 *  Parallel tile symmetric matrix-matrix multiplication - dynamic scheduling
 **/
void morse_pzsymm(MORSE_enum side, MORSE_enum uplo,
                          MORSE_Complex64_t alpha, MORSE_desc_t *A, MORSE_desc_t *B,
                          MORSE_Complex64_t beta, MORSE_desc_t *C,
                          MORSE_sequence_t *sequence, MORSE_request_t *request)
{
    MORSE_context_t *morse;
    MORSE_option_t options;

    int k, m, n;
    int ldak, ldam, ldan, ldbk, ldbm, ldcm;
    int tempmm, tempnn, tempkn, tempkm;

    MORSE_Complex64_t zbeta;
    MORSE_Complex64_t zone = (MORSE_Complex64_t)1.0;

    morse = morse_context_self();
    if (sequence->status != MORSE_SUCCESS)
        return;
    RUNTIME_options_init(&options, morse, sequence, request);

    /*
     *  MorseLeft
     */
    if (side == MorseLeft) {
        for (k = 0; k < C->mt; k++) {
            tempkm = k == C->mt-1 ? C->m-k*C->mb : C->mb;
            ldak = BLKLDD(A, k);
            ldbk = BLKLDD(B, k);
            zbeta = k == 0 ? beta : zone;

            for (n = 0; n < C->nt; n++) {
                tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;

                for (m = 0; m < C->mt; m++) {
                    tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;
                    ldam = BLKLDD(A, m);
                    ldcm = BLKLDD(C, m);

                    /*
                     *  MorseLeft / MorseLower
                     */
                    if (uplo == MorseLower) {
                        if (k < m) {
                            MORSE_TASK_zgemm(
                                &options,
                                MorseNoTrans, MorseNoTrans,
                                tempmm, tempnn, tempkm, A->mb,
                                alpha, A(m, k), ldam,
                                       B(k, n), ldbk,
                                zbeta, C(m, n), ldcm);
                        }
                        else {
                            if (k == m) {
                                MORSE_TASK_zsymm(
                                    &options,
                                    side, uplo,
                                    tempmm, tempnn, A->mb,
                                    alpha, A(k, k), ldak,
                                           B(k, n), ldbk,
                                    zbeta, C(m, n), ldcm);
                            }
                            else {
                                MORSE_TASK_zgemm(
                                    &options,
                                    MorseTrans, MorseNoTrans,
                                    tempmm, tempnn, tempkm, A->mb,
                                    alpha, A(k, m), ldak,
                                           B(k, n), ldbk,
                                    zbeta, C(m, n), ldcm);
                            }
                        }
                    }
                    /*
                     *  MorseLeft / MorseUpper
                     */
                    else {
                        if (k < m) {
                            MORSE_TASK_zgemm(
                                &options,
                                MorseTrans, MorseNoTrans,
                                tempmm, tempnn, tempkm, A->mb,
                                alpha, A(k, m), ldak,
                                       B(k, n), ldbk,
                                zbeta, C(m, n), ldcm);
                        }
                        else {
                            if (k == m) {
                                MORSE_TASK_zsymm(
                                    &options,
                                    side, uplo,
                                    tempmm, tempnn, A->mb,
                                    alpha, A(k, k), ldak,
                                           B(k, n), ldbk,
                                    zbeta, C(m, n), ldcm);
                            }
                            else {
                                MORSE_TASK_zgemm(
                                    &options,
                                    MorseNoTrans, MorseNoTrans,
                                    tempmm, tempnn, tempkm, A->mb,
                                    alpha, A(m, k), ldam,
                                           B(k, n), ldbk,
                                    zbeta, C(m, n), ldcm);
                            }
                        }
                    }
                }
                MORSE_TASK_flush_data( &options, B(k, n) );
            }
            if (uplo == MorseLower) {
                for (n = 0; n <= k; n++) {
                    MORSE_TASK_flush_data( &options, A(k, n) );
                }
            }
            else {
                for (m = 0; m <= k; m++) {
                    MORSE_TASK_flush_data( &options, A(m, k) );
                }
            }
        }
    }
    /*
     *  MorseRight
     */
    else {
        for (k = 0; k < C->nt; k++) {
            tempkn = k == C->nt-1 ? C->n-k*C->nb : C->nb;
            ldak = BLKLDD(A, k);
            zbeta = k == 0 ? beta : zone;

            for (m = 0; m < C->mt; m++) {
                tempmm = m == C->mt-1 ? C->m-m*C->mb : C->mb;
                ldbm = BLKLDD(B, m);
                ldcm = BLKLDD(C, m);

                for (n = 0; n < C->nt; n++) {
                    tempnn = n == C->nt-1 ? C->n-n*C->nb : C->nb;
                    ldan = BLKLDD(A, n);

                    /*
                     *  MorseRight / MorseLower
                     */
                    if (uplo == MorseLower) {
                        if (k < n) {
                           MORSE_TASK_zgemm(
                               &options,
                               MorseNoTrans, MorseTrans,
                               tempmm, tempnn, tempkn, A->mb,
                               alpha, B(m, k), ldbm,
                                      A(n, k), ldan,
                               zbeta, C(m, n), ldcm);
                        }
                        else {
                            if (k == n) {
                               MORSE_TASK_zsymm(
                                   &options,
                                   MorseRight, uplo,
                                   tempmm, tempnn, A->mb,
                                   alpha, A(k, k), ldak,
                                          B(m, k), ldbm,
                                   zbeta, C(m, n), ldcm);
                            }
                            else {
                                MORSE_TASK_zgemm(
                                    &options,
                                    MorseNoTrans, MorseNoTrans,
                                    tempmm, tempnn, tempkn, A->mb,
                                    alpha, B(m, k), ldbm,
                                           A(k, n), ldak,
                                    zbeta, C(m, n), ldcm);
                            }
                        }
                    }
                    /*
                     *  MorseRight / MorseUpper
                     */
                    else {
                        if (k < n) {
                            MORSE_TASK_zgemm(
                                &options,
                                MorseNoTrans, MorseNoTrans,
                                tempmm, tempnn, tempkn, A->mb,
                                alpha, B(m, k), ldbm,
                                       A(k, n), ldak,
                                zbeta, C(m, n), ldcm);
                        }
                        else {
                            if (k == n) {
                                MORSE_TASK_zsymm(
                                    &options,
                                    MorseRight, uplo,
                                    tempmm, tempnn, A->mb,
                                    alpha, A(k, k), ldak,
                                           B(m, k), ldbm,
                                    zbeta, C(m, n), ldcm);
                            }
                            else {
                                MORSE_TASK_zgemm(
                                    &options,
                                    MorseNoTrans, MorseTrans,
                                    tempmm, tempnn, tempkn, A->mb,
                                    alpha, B(m, k), ldbm,
                                           A(n, k), ldan,
                                    zbeta, C(m, n), ldcm);
                            }
                        }
                    }
                }
                MORSE_TASK_flush_data( &options, B(m, k) );
            }
            if (uplo == MorseLower) {
                for (n = 0; n <= k; n++) {
                    MORSE_TASK_flush_data( &options, A(k, n) );
                }
            }
            else {
                for (m = 0; m <= k; m++) {
                    MORSE_TASK_flush_data( &options, A(m, k) );
                }
            }
        }
    }
    RUNTIME_options_finalize(&options, morse);
    MORSE_TASK_flush_all();
}

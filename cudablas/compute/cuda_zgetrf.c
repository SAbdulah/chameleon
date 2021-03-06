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
 * @file cuda_zgetrf.c
 *
 *  MORSE cudablas kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver,
 *  and INRIA Bordeaux Sud-Ouest
 *
 * @author Florent Pruvost
 * @date 2015-09-16
 * @precisions normal z -> c d s
 *
 **/
#include "cudablas.h"

#if defined(CHAMELEON_USE_MAGMA)
#if defined(HAVE_MAGMA_GETRF_INCPIV_GPU)
int CUDA_zgetrf_incpiv(
        char storev, magma_int_t m, magma_int_t n, magma_int_t ib,
        cuDoubleComplex *hA, magma_int_t ldha, cuDoubleComplex *dA, magma_int_t ldda,
        cuDoubleComplex *hL, magma_int_t ldhl, cuDoubleComplex *dL, magma_int_t lddl,
        magma_int_t *ipiv,
        cuDoubleComplex *dwork, magma_int_t lddwork,
        magma_int_t *info)
{
    magma_zgetrf_incpiv_gpu( storev, m, n, ib,
                             hA, ldha, dA, ldda,
                             hL, ldhl,  dL, lddl,
                             ipiv,
                             dwork, lddwork,
                             info );
    return MORSE_SUCCESS;
}
#endif
int CUDA_zgetrf_nopiv(
        magma_int_t m, magma_int_t n,
        cuDoubleComplex *dA, magma_int_t ldda,
        magma_int_t *info)
{
    magma_zgetrf_nopiv_gpu( m, n, dA, ldda, info );
    return MORSE_SUCCESS;
}
#endif

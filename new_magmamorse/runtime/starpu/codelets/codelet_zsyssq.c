/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University 
 *                          of Tennessee Research Foundation. 
 *                          All rights reserved.
 * @copyright (c) 2012-2014 Inria. All rights reserved.
 * @copyright (c) 2012-2014 IPB. All rights reserved. 
 *
 **/

/**
 *
 * @file codelet_zsyssq.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Inria Bordeaux - Sud-Ouest, LaBRI,
 *  University of Bordeaux, Bordeaux INP
 *
 * @version 2.6.0
 * @comment This file has been automatically generated
 *          from Plasma 2.6.0 for MORSE 1.0.0
 * @author Mathieu Faverge
 * @date 2010-11-15
 * @precisions normal z -> c d s
 *
 **/
#include "morse_starpu.h"
#include "codelet_z.h"

void MORSE_TASK_zsyssq( MORSE_option_t *options,
                        MORSE_enum uplo, int n,
                        MORSE_desc_t *A, int Am, int An, int lda,
                        MORSE_desc_t *SCALESUMSQ, int SCALESUMSQm, int SCALESUMSQn )
{
    struct starpu_codelet *codelet = &cl_zsyssq;
    void (*callback)(void*) = options->profiling ? cl_zgessq_callback : NULL;
    if ( morse_desc_islocal( A, Am, An ) ||
         morse_desc_islocal( SCALESUMSQ, SCALESUMSQm, SCALESUMSQn ) ){
        starpu_insert_task(codelet,
            STARPU_VALUE,    &uplo,                       sizeof(MORSE_enum),
            STARPU_VALUE,    &n,                          sizeof(int),
            STARPU_R,        RTBLKADDR(A, MORSE_Complex64_t, Am, An),
            STARPU_VALUE,    &lda,                        sizeof(int),
            STARPU_RW,       RTBLKADDR(SCALESUMSQ, double, SCALESUMSQm, SCALESUMSQn),
            STARPU_PRIORITY, options->priority,
            STARPU_CALLBACK, callback,
            0);
    }
}


static void cl_zsyssq_cpu_func(void *descr[], void *cl_arg)
{
    MORSE_enum uplo;
    int n;
    MORSE_Complex64_t *A;
    int lda;
    double *SCALESUMSQ;

    A          = (MORSE_Complex64_t *)STARPU_MATRIX_GET_PTR(descr[0]);
    SCALESUMSQ = (double *)STARPU_MATRIX_GET_PTR(descr[1]);
    starpu_codelet_unpack_args(cl_arg, &uplo, &n, &lda);
    CORE_zsyssq( uplo, n, A, lda, &SCALESUMSQ[0], &SCALESUMSQ[1] );
}
/*
 * Codelet definition
 */
CODELETS_CPU(zsyssq, 2, cl_zsyssq_cpu_func)

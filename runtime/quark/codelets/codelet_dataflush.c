/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2014 Inria. All rights reserved.
 * @copyright (c) 2012-2014, 2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file codelet_dataflush.c
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 2.5.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for MORSE 1.0.0
 * @author Mathieu Faverge
 *
 * @date 2014-02-05
 *
 **/
#include "chameleon_quark.h"

void MORSE_TASK_flush_data( const MORSE_option_t *options,
                           const MORSE_desc_t *A, int Am, int An )
{
    (void)options; (void)A; (void)Am; (void)An;

    /*
     * This is useful for StarPU and PaRSEC MPI implementation, if it happens in
     * Quark, it will need to be done carefuly to not break other runtimes.
     */
}

void MORSE_TASK_flush_desc( const MORSE_option_t *options,
                           MORSE_enum uplo, const MORSE_desc_t *A )
{
    (void)options; (void)uplo; (void)A;

    /*
     * This is useful for StarPU and PaRSEC MPI implementation, if it happens in
     * Quark, it will need to be done carefuly to not break other runtimes.
     */
}

void MORSE_TASK_flush_all()
{
}

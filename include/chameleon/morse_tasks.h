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
 *  @file morse_tasks.h
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver,
 *  and INRIA Bordeaux Sud-Ouest
 *
 *  @version 2.3.1
 *  @author Mathieu Faverge
 *  @author Cedric Augonnet
 *  @date 2011-06-01
 *
 **/
#ifndef _MORSE_TASKS_H_
#define _MORSE_TASKS_H_

#include "chameleon/chameleon_config.h"

BEGIN_C_DECLS

/*******************************************************************************
 * MORSE Task submission routines
 **/
void MORSE_TASK_ztile_zero(const MORSE_option_t *options,
                           int X1, int X2, int Y1, int Y2,
                           const MORSE_desc_t *A, int Am, int An, int lda);
void MORSE_TASK_dtile_zero(const MORSE_option_t *options,
                           int X1, int X2, int Y1, int Y2,
                           const MORSE_desc_t *A, int Am, int An, int lda);
void MORSE_TASK_ctile_zero(const MORSE_option_t *options,
                           int X1, int X2, int Y1, int Y2,
                           const MORSE_desc_t *A, int Am, int An, int lda);
void MORSE_TASK_stile_zero(const MORSE_option_t *options,
                           int X1, int X2, int Y1, int Y2,
                           const MORSE_desc_t *A, int Am, int An, int lda);

/*
 * Mark a data as unused after this call
 */
void MORSE_TASK_flush_data( const MORSE_option_t *options,
                           const MORSE_desc_t *A, int Am, int An);
void MORSE_TASK_flush_desc( const MORSE_option_t *options,
                           MORSE_enum uplo, const MORSE_desc_t *A );
void MORSE_TASK_flush_all();

#include "chameleon/morse_tasks_z.h"
#include "chameleon/morse_tasks_d.h"
#include "chameleon/morse_tasks_c.h"
#include "chameleon/morse_tasks_s.h"
#include "chameleon/morse_tasks_zc.h"
#include "chameleon/morse_tasks_ds.h"

END_C_DECLS

#endif /* _MORSE_TASKS_H_ */

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
 * @file runtime_async.c
 *
 *  MORSE auxiliary routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 0.9.0
 * @author Mathieu Faverge
 * @author Cedric Castagnede
 * @date 2010-11-15
 *
 **/
#include <stdlib.h>
#include "chameleon_starpu.h"

/*******************************************************************************
 *  Create a sequence
 **/
int RUNTIME_sequence_create( MORSE_context_t *morse, MORSE_sequence_t *sequence )
{
    (void)morse;
    (void)sequence;
    return MORSE_SUCCESS;
}

/*******************************************************************************
 *  Destroy a sequence
 **/
int RUNTIME_sequence_destroy( MORSE_context_t *morse, MORSE_sequence_t *sequence )
{
    (void)morse;
    (void)sequence;
    return MORSE_SUCCESS;
}

// Defined in control/auxilliary.c
extern void (*update_progress_callback)(int, int) ;

// no progress indicator for algorithms faster than 'PROGRESS_MINIMUM_DURATION' seconds
#define PROGRESS_MINIMUM_DURATION 10

/*******************************************************************************
 *  Display a progress information when executing the tasks
 **/
int RUNTIME_progress( MORSE_context_t *morse )
{
    int tasksLeft, current, timer = 0;
    int max;

#if defined(CHAMELEON_USE_MPI)
    if ( morse->my_mpi_rank != 0 )
        return MORSE_SUCCESS;
#endif

    max = starpu_task_nsubmitted();
    if ( max == 0 )
        return MORSE_SUCCESS;

    //  update_progress_callback(0, max);
    while ((tasksLeft = starpu_task_nsubmitted()) > 0) {
        current = max - tasksLeft;
        if (timer > PROGRESS_MINIMUM_DURATION)
            update_progress_callback(current, max);
        sleep(1);
        timer++;
    }
    if (timer > PROGRESS_MINIMUM_DURATION)
        update_progress_callback(max, max);

    (void)morse;
    return MORSE_SUCCESS;
}

/*******************************************************************************
 *  Wait for the completion of a sequence
 **/
int RUNTIME_sequence_wait( MORSE_context_t *morse, MORSE_sequence_t *sequence )
{
    (void)morse;
    (void)sequence;
    if (morse->progress_enabled)
        RUNTIME_progress(morse);
    starpu_task_wait_for_all();
#if defined(CHAMELEON_USE_MPI)
    starpu_mpi_barrier(MPI_COMM_WORLD);
#endif
    return MORSE_SUCCESS;
}

/*******************************************************************************
 *  Terminate a sequence
 **/
void RUNTIME_sequence_flush( void *schedopt, MORSE_sequence_t *sequence, MORSE_request_t *request, int status)
{
    (void)schedopt;
    sequence->request = request;
    sequence->status = status;
    request->status = status;
    starpu_task_wait_for_all();
    return;
}

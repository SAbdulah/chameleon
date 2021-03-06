/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2016 Inria. All rights reserved.
 * @copyright (c) 2012-2015 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file control.c
 *
 *  MORSE auxiliary routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 0.9.0
 * @author Jakub Kurzak
 * @author Mathieu Faverge
 * @author Cedric Castagnede
 * @date 2012-09-15
 *
 **/

/**
 *
 * @defgroup Control
 * @brief Group routines exposed to users to control MORSE state
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "control/auxiliary.h"
#include "control/common.h"
#include "chameleon/morse_runtime.h"

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_Init - Initialize MORSE.
 *
 ******************************************************************************
 *
 * @param[in] cores
 *          Number of cores to use.
 *
 * @param[in] gpus
 *          Number of cuda devices to use.
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_Init(int cores, int gpus)
{
    return MORSE_InitPar(cores, gpus, -1);
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_InitPar - Initialize MORSE.
 *
 ******************************************************************************
 *
 * @param[in] ncpus
 *          Number of cores to use.
 *
 * @param[in] ncudas
 *          Number of cuda devices to use.
 *
 * @param[in] nthreads_per_worker
 *          Number of threads per worker (cpu, cuda device).
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_InitPar(int ncpus, int ncudas, int nthreads_per_worker)
{
    MORSE_context_t *morse;

    /* Create context and insert in the context map */
    morse = morse_context_create();
    if (morse == NULL) {
        morse_fatal_error("MORSE_Init", "morse_context_create() failed");
        return MORSE_ERR_OUT_OF_RESOURCES;
    }

#if defined(CHAMELEON_USE_MPI)
#  if defined(CHAMELEON_SIMULATION)
    /* Assuming that we don't initialize MPI ourself (which SMPI doesn't support anyway) */
    morse->mpi_outer_init = 1;
#  else
    {
      int flag = 0, provided = 0;
      MPI_Initialized( &flag );
      morse->mpi_outer_init = flag;
      if ( !flag ) {
          MPI_Init_thread( NULL, NULL, MPI_THREAD_MULTIPLE, &provided );
      }
    }
#  endif
#endif
    RUNTIME_init_scheduler( morse, ncpus, ncudas, nthreads_per_worker );
    return MORSE_SUCCESS;
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_Finalize - Finalize MORSE.
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_Finalize(void)
{
    MORSE_context_t *morse = morse_context_self();
    if (morse == NULL) {
        morse_error("MORSE_Finalize()", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    MORSE_TASK_flush_all();
#  if !defined(CHAMELEON_SIMULATION)
    RUNTIME_barrier(morse);
#  endif
    RUNTIME_finalize_scheduler( morse );

#if defined(CHAMELEON_USE_MPI)
    if (!morse->mpi_outer_init)
        MPI_Finalize();
#endif

    morse_context_destroy();
    return MORSE_SUCCESS;
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_Pause - Suspend MORSE runtime to poll for new tasks.
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_Pause(void)
{
    MORSE_context_t *morse = morse_context_self();
    if (morse == NULL) {
        morse_error("MORSE_Pause()", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    RUNTIME_pause(morse);
    return MORSE_SUCCESS;
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_Resume - Symmetrical call to MORSE_Pause,
 *  used to resume the workers polling for new tasks.
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_Resume(void)
{
    MORSE_context_t *morse = morse_context_self();
    if (morse == NULL) {
        morse_error("MORSE_Resume()", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    RUNTIME_resume(morse);
    return MORSE_SUCCESS;
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_Distributed_start - Prepare the distributed processes for computation
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_Distributed_start(void)
{
    MORSE_context_t *morse = morse_context_self();
    if (morse == NULL) {
        morse_error("MORSE_Finalize()", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    RUNTIME_barrier (morse);
    return MORSE_SUCCESS;
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_Distributed_stop - Clean the distributed processes after computation
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_Distributed_stop(void)
{
    MORSE_context_t *morse = morse_context_self();
    if (morse == NULL) {
        morse_error("MORSE_Finalize()", "MORSE not initialized");
        return MORSE_ERR_NOT_INITIALIZED;
    }
    RUNTIME_barrier (morse);
    return MORSE_SUCCESS;
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_Comm_size - Return the size of the distributed computation
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_Comm_size( int *size )
{
    RUNTIME_comm_size (size);
    return MORSE_SUCCESS;
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_Comm_rank - Return the rank of the distributed computation
 *
 ******************************************************************************
 *
 * @return
 *          \retval MORSE_SUCCESS successful exit
 *
 *****************************************************************************/
int MORSE_Comm_rank( int *rank )
{
    RUNTIME_comm_rank (rank);
    return MORSE_SUCCESS;
}

/** ***************************************************************************
 *
 * @ingroup Control
 *
 *  MORSE_GetThreadNbr - Return the number of CPU workers initialized by the
 *  runtime
 *
 ******************************************************************************
 *
 * @return
 *          \retval The number of CPU workers started
 *
 *****************************************************************************/
int MORSE_GetThreadNbr( )
{
    return RUNTIME_get_thread_nbr();
}

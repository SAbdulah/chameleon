/**
 *
 * @copyright (c) 2009-2015 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2015 Inria. All rights reserved.
 * @copyright (c) 2012-2015 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include "chameleon_parsec.h"

#if defined(CHAMELEON_USE_MPI)
#include <mpi.h>
#endif

/*******************************************************************************
 * Thread rank.
 **/
int RUNTIME_rank(MORSE_context_t *morse)
{
    return 0;
}

/*******************************************************************************
 * Initialize MORSE
 **/
int RUNTIME_init_scheduler(MORSE_context_t *morse, int nworkers, int ncudas, int nthreads_per_worker)
{
    int hres = -1, default_ncores = -1;
    int *argc = (int *)malloc(sizeof(int));
    *argc = 0;

    /* Initializing parsec context */
    if( 0 < nworkers ) {
        default_ncores = nworkers;
    }
    morse->parallel_enabled = MORSE_TRUE;
    morse->schedopt = (void *)parsec_init(default_ncores, argc, NULL);

    if(NULL != morse->schedopt) {
        morse->nworkers = nworkers;
        morse->nthreads_per_worker = nthreads_per_worker;
        hres = 0;
    }

    free(argc);
    return hres;
}

/*******************************************************************************
 * Finalize MORSE
 */
void RUNTIME_finalize_scheduler(MORSE_context_t *morse)
{
    parsec_context_t *parsec = (parsec_context_t*)morse->schedopt;
    parsec_fini(&parsec);
    return;
}

/*******************************************************************************
 * Barrier MORSE.
 **/
void RUNTIME_barrier(MORSE_context_t *morse)
{
    parsec_context_t *parsec = (parsec_context_t*)morse->schedopt;
    // This will be a problem with the fake tasks inserted to detect end of DTD algorithms
    //parsec_context_wait( parsec );
    return;
}

/*******************************************************************************
 *  Set iteration numbers for traces
 **/
void RUNTIME_iteration_push( MORSE_context_t *morse, unsigned long iteration )
{
    (void)morse; (void)iteration;
    return;
}
void RUNTIME_iteration_pop( MORSE_context_t *morse )
{
    (void)morse;
    return;
}

/*******************************************************************************
 *  To suspend the processing of new tasks by workers
 **/
void RUNTIME_pause( MORSE_context_t *morse )
{
    (void)morse;
    return;
}

/*******************************************************************************
 *  This is the symmetrical call to RUNTIME_pause,
 *  used to resume the workers polling for new tasks.
 **/
void RUNTIME_resume( MORSE_context_t *morse )
{
    (void)morse;
    return;
}

/*******************************************************************************
 *  This returns the rank of this process
 **/
void RUNTIME_comm_rank( int *rank )
{
#if defined(CHAMELEON_USE_MPI)
    MPI_Comm_rank(MPI_COMM_WORLD, rank);
#else
    *rank = 0;
#endif
    return;
}

/*******************************************************************************
 *  This returns the size of the distributed computation
 **/
void RUNTIME_comm_size( int *size )
{
#if defined(CHAMELEON_USE_MPI)
    MPI_Comm_size(MPI_COMM_WORLD, size);
#else
    *size = 1;
#endif
    return;
}

/*******************************************************************************
 *  This returns the number of workers
 **/
int RUNTIME_get_thread_nbr()
{
    // TODO: fixme
    //return vpmap_get_nb_total_threads();
    return 0;
}

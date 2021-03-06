/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2016 Inria. All rights reserved.
 * @copyright (c) 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file runtime_control.c
 *
 *  MORSE auxiliary routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 0.9.0
 * @author Mathieu Faverge
 * @author Cedric Augonnet
 * @author Cedric Castagnede
 * @date 2010-11-15
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include "chameleon_starpu.h"

#if defined(CHAMELEON_SIMULATION)
# ifndef STARPU_SIMGRID
#  error "Starpu was not built with simgrid support (--enable-simgrid). Can not run Chameleon with simulation support."
# endif
#else
# ifdef STARPU_SIMGRID
#  warning "Starpu was built with simgrid support. Better build Chameleon with simulation support (-DCHAMELEON_SIMULATION=YES)."
# endif
#endif
/*******************************************************************************
 * Thread rank.
 **/
int RUNTIME_rank(MORSE_context_t *morse)
{
    (void)morse;
    return starpu_worker_get_id();
}

/*******************************************************************************
 *
 **/
int RUNTIME_init_scheduler( MORSE_context_t *morse, int ncpus, int ncudas, int nthreads_per_worker)
{
    starpu_conf_t *conf = (starpu_conf_t*)(morse->schedopt);
    int hres = -1;

    /* StarPU was already initialized by an external library */
    if (conf == NULL) {
        return 0;
    }

    conf->ncpus = ncpus;
    conf->ncuda = ncudas;
    conf->nopencl = 0;

    /* By default, use the dmdas strategy */
    if (!getenv("STARPU_SCHED")) {
        if (conf->ncuda > 0) {
            conf->sched_policy_name = "dmdas";
        }
        else {
            /**
             * Set scheduling to "ws"/"lws" if no cuda devices used because it
             * behaves better on homogneneous architectures. If the user wants
             * to use another scheduling strategy, he can set STARPU_SCHED
             * env. var. to whatever he wants
             */
#if (STARPU_MAJOR_VERSION > 1) || ((STARPU_MAJOR_VERSION == 1) && (STARPU_MINOR_VERSION >= 2))
            conf->sched_policy_name = "lws";
#else
            conf->sched_policy_name = "ws";
#endif
        }
    }

    if ((ncpus == -1)||(nthreads_per_worker == -1))
    {
        morse->parallel_enabled = MORSE_FALSE;

        hres = starpu_init( conf );
    }
    else {
        int worker;

        morse->parallel_enabled = MORSE_TRUE;

        for (worker = 0; worker < ncpus; worker++)
            conf->workers_bindid[worker] = (worker+1)*nthreads_per_worker - 1;

        for (worker = 0; worker < ncpus; worker++)
            conf->workers_bindid[worker + ncudas] = worker*nthreads_per_worker;

        conf->use_explicit_workers_bindid = 1;

        hres = starpu_init( conf );

        morse->nworkers = ncpus;
        morse->nthreads_per_worker = nthreads_per_worker;
    }

#ifdef HAVE_STARPU_MALLOC_ON_NODE_SET_DEFAULT_FLAGS
    starpu_malloc_on_node_set_default_flags(STARPU_MAIN_RAM, STARPU_MALLOC_PINNED | STARPU_MALLOC_COUNT
#ifdef STARPU_MALLOC_SIMULATION_FOLDED
            | STARPU_MALLOC_SIMULATION_FOLDED
#endif
            );
#endif

#if defined(CHAMELEON_USE_MPI)
    {
        int flag = 0;
#if !defined(CHAMELEON_SIMULATION)
        MPI_Initialized( &flag );
#endif
        starpu_mpi_init(NULL, NULL, !flag);
        RUNTIME_comm_rank(&(morse->my_mpi_rank));
        RUNTIME_comm_size(&(morse->mpi_comm_size));
    }
#endif

#if defined(CHAMELEON_USE_CUDA) && !defined(CHAMELEON_SIMULATION)
    starpu_cublas_init();
#endif

    return hres;
}

/*******************************************************************************
 *
 */
void RUNTIME_finalize_scheduler( MORSE_context_t *morse )
{
    (void)morse;

    /* StarPU was already initialized by an external library */
    if (morse->schedopt == NULL) {
        return;
    }

#if defined(CHAMELEON_USE_MPI)
    starpu_mpi_shutdown();
#endif
#if defined(CHAMELEON_USE_CUDA) && !defined(CHAMELEON_SIMULATION)
    starpu_cublas_shutdown();
#endif

    starpu_shutdown();
    return;
}

/*******************************************************************************
 *  Busy-waiting barrier
 **/
void RUNTIME_barrier( MORSE_context_t *morse )
{
    (void)morse;
    starpu_task_wait_for_all();
#if defined(CHAMELEON_USE_MPI)
    starpu_mpi_barrier(MPI_COMM_WORLD);
#endif
}

/*******************************************************************************
 *  Set iteration numbers for traces
 **/
void RUNTIME_iteration_push( MORSE_context_t *morse, unsigned long iteration )
{
    (void)morse;
#if defined(HAVE_STARPU_ITERATION_PUSH)
    starpu_iteration_push(iteration);
#endif
}

void RUNTIME_iteration_pop( MORSE_context_t *morse )
{
    (void)morse;
#if defined(HAVE_STARPU_ITERATION_PUSH)
    starpu_iteration_pop();
#endif
}

/*******************************************************************************
 *  To suspend the processing of new tasks by workers
 **/
void RUNTIME_pause( MORSE_context_t *morse )
{
    (void)morse;
    starpu_pause();
    return;
}

/*******************************************************************************
 *  This is the symmetrical call to RUNTIME_pause,
 *  used to resume the workers polling for new tasks.
 **/
void RUNTIME_resume( MORSE_context_t *morse )
{
    (void)morse;
    starpu_resume();
    return;
}

/*******************************************************************************
 *  This returns the rank of this process
 **/
void RUNTIME_comm_rank( int *rank )
{
#if defined(CHAMELEON_USE_MPI)
#  if defined(HAVE_STARPU_MPI_COMM_RANK)
    starpu_mpi_comm_rank(MPI_COMM_WORLD, rank);
#  else
    MPI_Comm_rank(MPI_COMM_WORLD, rank);
#  endif
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
#  if defined(HAVE_STARPU_MPI_COMM_RANK)
    starpu_mpi_comm_size(MPI_COMM_WORLD, size);
#  else
    MPI_Comm_size(MPI_COMM_WORLD, size);
#  endif
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
    return starpu_worker_get_count_by_type( STARPU_CPU_WORKER );
}

/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2014 Inria. All rights reserved.
 * @copyright (c) 2012-2015 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file step6.c
 *
 *  MORSE example routines
 *  MORSE is a software package provided by Inria Bordeaux - Sud-Ouest, LaBRI,
 *  University of Bordeaux, Bordeaux INP
 *
 * @version 1.0.0
 * @author Florent Pruvost
 * @date 2014-10-29
 *
 **/

#include "step6.h"

/*
 * @brief step6 introduces how to use MORSE with MPI.
 * @details This program is a copy of step5 with some additional parameters to
 * be set for the data distribution. To use this program properly MORSE must
 * use StarPU Runtime system and MPI option must be activated at configure.
 * The data distribution used here is 2D block cyclic, see for example
 * http://www.netlib.org/scalapack/slug/node75.html for explanation.
 * The user can enter the parameters of the distribution grid at execution with
 * --p= and --q=
 */
int main(int argc, char *argv[]) {

    size_t N;    // matrix order
    int NB;   // number of rows and columns in tiles
    int NRHS; // number of RHS vectors
    int NCPU; // number of cores to use
    int NGPU; // number of gpus (cuda devices) to use
    int GRID_P; // parameter of the 2D block cyclic distribution
    int GRID_Q; // parameter of the 2D block cyclic distribution
    int NMPIPROC = 1; // number of MPI processus
    int UPLO = MorseUpper; // where is stored L

    /* descriptors necessary for calling MORSE tile interface  */
    MORSE_desc_t *descA = NULL, *descAC = NULL, *descB = NULL, *descX = NULL;

    /* declarations to time the program and evaluate performances */
    double fmuls, fadds, flops, gflops, cpu_time;

    /* variable to check the numerical results */
    double anorm, bnorm, xnorm, eps, res;
    int hres;

    /* MORSE sequence uniquely identifies a set of asynchronous function calls
     * sharing common exception handling */
    MORSE_sequence_t *sequence = NULL;
    /* MORSE request uniquely identifies each asynchronous function call */
    MORSE_request_t request = MORSE_REQUEST_INITIALIZER;
    int status;

    /* initialize some parameters with default values */
    int iparam[IPARAM_SIZEOF];
    memset(iparam, 0, IPARAM_SIZEOF*sizeof(int));
    init_iparam(iparam);

    /* read arguments */
    read_args(argc, argv, iparam);
    N    = iparam[IPARAM_N];
    NB   = iparam[IPARAM_NB];
    NRHS = iparam[IPARAM_NRHS];

    /* compute the algorithm complexity to evaluate performances */
    fadds = (double)( FADDS_POTRF(N) + 2 * FADDS_TRSM(N,NRHS) );
    fmuls = (double)( FMULS_POTRF(N) + 2 * FMULS_TRSM(N,NRHS) );
    flops = 1e-9 * (fmuls + fadds);
    gflops = 0.0;
    cpu_time = 0.0;

    /* initialize the number of thread if not given by the user in argv
     * It makes sense only if this program is linked with pthread and
     * multithreaded BLAS and LAPACK */
    if ( iparam[IPARAM_THRDNBR] == -1 ) {
        get_thread_count( &(iparam[IPARAM_THRDNBR]) );
        /* reserve one thread par cuda device to optimize memory transfers */
        iparam[IPARAM_THRDNBR] -= iparam[IPARAM_NCUDAS];
    }
    NCPU = iparam[IPARAM_THRDNBR];
    NGPU = iparam[IPARAM_NCUDAS];

     /* Initialize MORSE with main parameters */
    if ( MORSE_Init( NCPU, NGPU ) != MORSE_SUCCESS ) {
        fprintf(stderr, "Error initializing MORSE library\n");
        return EXIT_FAILURE;
    }

    /* set some specific parameters related to MORSE: blocks size and inner-blocking size */
    MORSE_Set(MORSE_TILE_SIZE,        iparam[IPARAM_NB] );
    MORSE_Set(MORSE_INNER_BLOCK_SIZE, iparam[IPARAM_IB] );

#if defined(CHAMELEON_USE_MPI)
    MORSE_Comm_size( &NMPIPROC );
    /* Check P */
    if ( (iparam[IPARAM_P] > 1) &&
         (NMPIPROC % iparam[IPARAM_P] != 0) ) {
      fprintf(stderr, "ERROR: %d doesn't divide the number of MPI processus %d\n",
              iparam[IPARAM_P], NMPIPROC );
      return EXIT_FAILURE;
    }
#endif
    iparam[IPARAM_Q] = NMPIPROC / iparam[IPARAM_P];
    iparam[IPARAM_NMPI] = NMPIPROC;
    GRID_P = iparam[IPARAM_P];
    GRID_Q = iparam[IPARAM_Q];

    if ( MORSE_My_Mpi_Rank() == 0 ){
        /* print informations to user */
        print_header( argv[0], iparam);
    }

    /* Initialize the structure required for MORSE tile interface */
    MORSE_Desc_Create(&descA, NULL, MorseRealDouble,
                      NB, NB, NB*NB, N, N, 0, 0, N, N,
                      GRID_P, GRID_Q);
    MORSE_Desc_Create(&descB, NULL, MorseRealDouble,
                      NB, NB, NB*NB, N, NRHS, 0, 0, N, NRHS,
                      GRID_P, GRID_Q);
    MORSE_Desc_Create(&descX, NULL, MorseRealDouble,
                      NB, NB, NB*NB, N, NRHS, 0, 0, N, NRHS,
                      GRID_P, GRID_Q);
    MORSE_Desc_Create(&descAC, NULL, MorseRealDouble,
                      NB, NB, NB*NB, N, N, 0, 0, N, N,
                      GRID_P, GRID_Q);

    /* generate A matrix with random values such that it is spd */
    MORSE_dplgsy_Tile( (double)N, MorseUpperLower, descA, 51 );

    /* generate RHS */
    MORSE_dplrnt_Tile( descB, 5673 );

    /* copy A before facto. in order to check the result */
    MORSE_dlacpy_Tile(MorseUpperLower, descA, descAC);

    /* copy B in X before solving
     * same sense as memcpy(X, B, N*NRHS*sizeof(double)) but for descriptors */
    MORSE_dlacpy_Tile(MorseUpperLower, descB, descX);

    /************************************************************/
    /* solve the system AX = B using the Cholesky factorization */
    /************************************************************/

    cpu_time = -cWtime();

    MORSE_Sequence_Create(&sequence);

    /* Cholesky factorization:
     * A is replaced by its factorization L or L^T depending on uplo */
    MORSE_dpotrf_Tile_Async( UPLO, descA, sequence, &request );

    /* Solve:
     * B is stored in X on entry, X contains the result on exit.
     * Forward and back substitutions
     */
    MORSE_dpotrs_Tile_Async( UPLO, descA, descX, sequence, &request);

    /* Synchronization barrier (the runtime ensures that all submitted tasks
     * have been terminated */
    MORSE_Sequence_Wait(sequence);

    /* Ensure that all data processed on the gpus we are depending on are back
     * in main memory */
    RUNTIME_desc_getoncpu(descA);
    RUNTIME_desc_getoncpu(descX);

    status = sequence->status;
    if ( status != 0 ) {
        fprintf(stderr, "Error in computation (%d)\n", status);
        return EXIT_FAILURE;
    }
    MORSE_Sequence_Destroy(sequence);

    cpu_time += cWtime();

    /* print informations to user */
    gflops = flops / cpu_time;
    if ( MORSE_My_Mpi_Rank() == 0 )
        printf( "%9.3f %9.2f\n", cpu_time, gflops);
    fflush( stdout );

    /************************************************************/
    /* check if solve is correct i.e. AX-B = 0                  */
    /************************************************************/

    /* compute norms to check the result */
    anorm = MORSE_dlange_Tile( MorseInfNorm, descAC);
    bnorm = MORSE_dlange_Tile( MorseInfNorm, descB);
    xnorm = MORSE_dlange_Tile( MorseInfNorm, descX);

    /* compute A*X-B, store the result in B */
    MORSE_dgemm_Tile( MorseNoTrans, MorseNoTrans,
                      1.0, descAC, descX, -1.0, descB );
    res = MORSE_dlange_Tile( MorseInfNorm, descB );

    /* check residual and print a message */
    eps = LAPACKE_dlamch_work( 'e' );

    /*
     * if hres = 0 then the test succeed
     * else the test failed
     */
    hres = 0;
    hres = ( res / N / eps / (anorm * xnorm + bnorm ) > 100.0 );
    if ( MORSE_My_Mpi_Rank() == 0 ){
        printf( "   ||Ax-b||       ||A||       ||x||       ||b|| ||Ax-b||/N/eps/(||A||||x||+||b||)  RETURN\n");
        if (hres)
            printf( "%8.5e %8.5e %8.5e %8.5e                       %8.5e FAILURE \n",
                res, anorm, xnorm, bnorm,
                res / N / eps / (anorm * xnorm + bnorm ));
        else
            printf( "%8.5e %8.5e %8.5e %8.5e                       %8.5e SUCCESS \n",
                res, anorm, xnorm, bnorm,
                res / N / eps / (anorm * xnorm + bnorm ));
    }

    /* deallocate A, B, X, Acpy and associated descriptors descA, ... */
    MORSE_Desc_Destroy( &descA );
    MORSE_Desc_Destroy( &descB );
    MORSE_Desc_Destroy( &descX );
    MORSE_Desc_Destroy( &descAC );

    /* Finalize MORSE */
    MORSE_Finalize();

    return EXIT_SUCCESS;
}

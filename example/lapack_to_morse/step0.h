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
 * @file step0.h
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

#ifndef STEP0_H
#define STEP0_H

/* Common include for all steps of the tutorial */
#include "lapack_to_morse.h"

/* Specific includes for step 0 */
#ifndef CBLAS_SADDR
#define CBLAS_SADDR( _val_ ) &(_val_)
#endif
#include <coreblas/include/cblas.h>>
#include <coreblas/include/lapacke.h>

#define CHAMELEON_VERSION_MAJOR 0
#define CHAMELEON_VERSION_MINOR 9
#define CHAMELEON_VERSION_MICRO 0

/* Integer parameters for step0 */
enum iparam_step0 {
    IPARAM_THRDNBR,        /* Number of cores                            */
    IPARAM_N,              /* Number of columns of the matrix            */
    IPARAM_LDA,            /* Leading dimension of A                     */
    IPARAM_LDB,            /* Leading dimension of B                     */
    IPARAM_IB,             /* Inner-blocking size                        */
    IPARAM_NRHS,           /* Number of RHS                              */
    /* End */
    IPARAM_SIZEOF
};

/* Specific routines used in step0.c main program */

/******************************************************************************
 * Initialize integer parameters
 */
static void init_iparam(int iparam[IPARAM_SIZEOF]){
    iparam[IPARAM_THRDNBR       ] = -1;
    iparam[IPARAM_N             ] = 500;
    iparam[IPARAM_LDA           ] = iparam[IPARAM_N];
    iparam[IPARAM_LDB           ] = iparam[IPARAM_N];
    iparam[IPARAM_IB            ] = 32;
    iparam[IPARAM_NRHS          ] = 1;
 }

/******************************************************************************
 * Print how to use the program
 */
static void show_help(char *prog_name) {
    printf( "Usage:\n%s [options]\n\n", prog_name );
    printf( "Options are:\n"
            "  --help           Show this help\n"
            "\n"
            "  --n=X            dimension (N) .(default: 500)\n"
            "  --ib=X           IB size. (default: 32)\n"
            "  --nrhs=X         number of RHS. (default: 1)\n"
            "\n");
}

/******************************************************************************
 * Read arguments following step0 program call
 */
static void read_args(int argc, char *argv[], int *iparam){
    int i;
    for (i = 1; i < argc && argv[i]; ++i) {
        if ( startswith( argv[i], "--help") || startswith( argv[i], "-help") ||
             startswith( argv[i], "--h") || startswith( argv[i], "-h") ) {
            show_help( argv[0] );
            exit(0);
        } else if (startswith( argv[i], "--n=" )) {
            sscanf( strchr( argv[i], '=' ) + 1, "%d", &(iparam[IPARAM_N]) );
        } else if (startswith( argv[i], "--ib=" )) {
            sscanf( strchr( argv[i], '=' ) + 1, "%d", &(iparam[IPARAM_IB]) );
        } else if (startswith( argv[i], "--nrhs=" )) {
            sscanf( strchr( argv[i], '=' ) + 1, "%d", &(iparam[IPARAM_NRHS]) );
        } else {
            fprintf( stderr, "Unknown option: %s\n", argv[i] );
        }
    }
}

/******************************************************************************
 * Print a header message to summarize main parameters
 */
static void print_header(char *prog_name, int * iparam) {
#if defined(CHAMELEON_SIMULATION)
    double    eps = 0.;
#else
    double    eps = LAPACKE_dlamch_work( 'e' );
#endif

    printf( "#\n"
            "# CHAMELEON %d.%d.%d, %s\n"
            "# Nb threads: %d\n"
            "# N:          %d\n"
            "# IB:         %d\n"
            "# eps:        %e\n"
            "#\n",
            CHAMELEON_VERSION_MAJOR,
            CHAMELEON_VERSION_MINOR,
            CHAMELEON_VERSION_MICRO,
            prog_name,
            iparam[IPARAM_THRDNBR],
            iparam[IPARAM_N],
            iparam[IPARAM_IB],
            eps );

    printf( "#      M       N  K/NRHS   seconds   Gflop/s\n");
    printf( "#%7d %7d %7d ", iparam[IPARAM_N], iparam[IPARAM_N], iparam[IPARAM_NRHS]);
    fflush( stdout );
    return;
}

/******************************************************************************
 * Macro to allocate a matrix as a 1D array
 */
#define PASTE_CODE_ALLOCATE_MATRIX(_name_, _type_, _m_, _n_)    \
    _type_ *_name_ = NULL;                                      \
    _name_ = (_type_*)malloc( (_m_) * (_n_) * sizeof(_type_) ); \
    if ( ! _name_ ) {                                           \
        fprintf(stderr, "Out of Memory for %s\n", #_name_);     \
        return -1;                                              \
    }

#endif /* STEP0_H */

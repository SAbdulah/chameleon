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
 * @file timing/auxiliary.h
 *
 *  MORSE timing auxiliary routines
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver
 *
 * @version 1.1.0
 * @author Mathieu Faverge
 * @author Cedric Castagnede
 * @date 2010-11-15
 *
 **/
#ifndef AUXILIARY_H
#define AUXILIARY_H

#ifndef max
#define max(a,b) ( ( (a) > (b) ) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ( ( (a) < (b) ) ? (a) : (b))
#endif

#include "zauxiliary.h"
#include "cauxiliary.h"
#include "dauxiliary.h"
#include "sauxiliary.h"

extern int IONE;
extern int ISEED[4];

extern int format[6];
extern int trans[3];
extern int uplo[2];
extern int side[2];
extern int diag[2];
extern char *formatstr[6];
extern char *transstr[3];
extern char *uplostr[2];
extern char *sidestr[2];
extern char *diagstr[2];

extern void *formatmap[6];

int map_CM  (int m, int n, int mb, int nb, int i, int j);
int map_RM  (int m, int n, int mb, int nb, int i, int j);
int map_CCRB(int m, int n, int mb, int nb, int i, int j);
int map_CRRB(int m, int n, int mb, int nb, int i, int j);
int map_RCRB(int m, int n, int mb, int nb, int i, int j);
int map_RRRB(int m, int n, int mb, int nb, int i, int j);

#endif /* AUXILIARY_H */

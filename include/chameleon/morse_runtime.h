/**
 *
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2014 Inria. All rights reserved.
 * @copyright (c) 2012-2016 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 *
 **/

/**
 *
 * @file runtime.h
 *
 *  MORSE codelets kernel
 *  MORSE is a software package provided by Univ. of Tennessee,
 *  Univ. of California Berkeley and Univ. of Colorado Denver,
 *  and INRIA Bordeaux Sud-Ouest
 *
 * @version 0.9.0
 * @author Mathieu Faverge
 * @author Cedric Augonnet
 * @author Cedric Castagnede
 * @date 2011-06-01
 *
 **/
#ifndef _MORSE_RUNTIME_H_
#define _MORSE_RUNTIME_H_

#include "chameleon/chameleon_config.h"
#include "chameleon/morse_struct.h"

BEGIN_C_DECLS

/*******************************************************************************
 * RUNTIME Async
 **/
int   RUNTIME_sequence_create  (MORSE_context_t*, MORSE_sequence_t*);
int   RUNTIME_sequence_destroy (MORSE_context_t*, MORSE_sequence_t*);
int   RUNTIME_sequence_wait    (MORSE_context_t*, MORSE_sequence_t*);
void  RUNTIME_sequence_flush   (void* quark, MORSE_sequence_t*, MORSE_request_t*, int);

/*******************************************************************************
 * RUNTIME Context
 **/
void  RUNTIME_context_create  (MORSE_context_t*);
void  RUNTIME_context_destroy (MORSE_context_t*);
void  RUNTIME_enable          (MORSE_enum);
void  RUNTIME_disable         (MORSE_enum);

/*******************************************************************************
 * RUNTIME Control
 **/
int   RUNTIME_rank               (MORSE_context_t*);
int   RUNTIME_init_scheduler     (MORSE_context_t*, int, int, int);
void  RUNTIME_finalize_scheduler (MORSE_context_t*);
void  RUNTIME_barrier            (MORSE_context_t*);
void  RUNTIME_iteration_push     (MORSE_context_t*, unsigned long iteration);
void  RUNTIME_iteration_pop      (MORSE_context_t*);
void  RUNTIME_pause              (MORSE_context_t*);
void  RUNTIME_resume             (MORSE_context_t*);
void  RUNTIME_comm_rank          (int*);
void  RUNTIME_comm_size          (int*);
int   RUNTIME_get_thread_nbr     (void);

/*******************************************************************************
 * RUNTIME Descriptor
 **/
void* RUNTIME_mat_alloc      (size_t);
void  RUNTIME_mat_free       (void*, size_t);
void  RUNTIME_desc_init      (MORSE_desc_t*);
void  RUNTIME_desc_create    (MORSE_desc_t*);
void  RUNTIME_desc_destroy   (MORSE_desc_t*);
void  RUNTIME_desc_submatrix (MORSE_desc_t*);
void* RUNTIME_desc_getaddr   (const MORSE_desc_t*, int, int);
/* Acquire in main memory an up-to-date copy of the data described by the descriptor for read-write access. */
int   RUNTIME_desc_acquire   (MORSE_desc_t*);
/* Release the data described by the descriptor to be used by the StarPU tasks again. */
int   RUNTIME_desc_release   (MORSE_desc_t*);
int   RUNTIME_desc_getoncpu  (MORSE_desc_t*);
void  RUNTIME_user_tag_size  (int, int) ;

/*******************************************************************************
 * RUNTIME Options
 **/
void  RUNTIME_options_init     (MORSE_option_t*, MORSE_context_t*, MORSE_sequence_t*, MORSE_request_t*);
void  RUNTIME_options_finalize (MORSE_option_t*, MORSE_context_t *);
int   RUNTIME_options_ws_alloc (MORSE_option_t*, size_t, size_t);
int   RUNTIME_options_ws_free  (MORSE_option_t*);
/* int   RUNTIME_options_ws_gethost   (MORSE_option_t*); */
/* int   RUNTIME_options_ws_getdevice (MORSE_option_t*); */

/*******************************************************************************
 * RUNTIME Locality
 **/
void RUNTIME_zlocality_allrestore ();
void RUNTIME_clocality_allrestore ();
void RUNTIME_dlocality_allrestore ();
void RUNTIME_slocality_allrestore ();

void RUNTIME_zlocality_allrestrict(uint32_t);
void RUNTIME_zlocality_onerestrict(MORSE_kernel_t, uint32_t);
void RUNTIME_zlocality_onerestore (MORSE_kernel_t);

void RUNTIME_clocality_allrestrict(uint32_t);
void RUNTIME_clocality_onerestrict(MORSE_kernel_t, uint32_t);
void RUNTIME_clocality_onerestore (MORSE_kernel_t);

void RUNTIME_dlocality_allrestrict(uint32_t);
void RUNTIME_dlocality_onerestrict(MORSE_kernel_t, uint32_t);
void RUNTIME_dlocality_onerestore (MORSE_kernel_t);

void RUNTIME_slocality_allrestrict(uint32_t);
void RUNTIME_slocality_onerestrict(MORSE_kernel_t, uint32_t);
void RUNTIME_slocality_onerestore (MORSE_kernel_t);

/*******************************************************************************
 * RUNTIME Profiling
 **/
void   RUNTIME_schedprofile_display ();
void   RUNTIME_kernelprofile_display();
double RUNTIME_get_time();

void RUNTIME_start_profiling();
void RUNTIME_stop_profiling();

void RUNTIME_start_stats();
void RUNTIME_stop_stats();

void RUNTIME_zdisplay_allprofile ();
void RUNTIME_zdisplay_oneprofile (MORSE_kernel_t);
void RUNTIME_cdisplay_allprofile ();
void RUNTIME_cdisplay_oneprofile (MORSE_kernel_t);
void RUNTIME_ddisplay_allprofile ();
void RUNTIME_ddisplay_oneprofile (MORSE_kernel_t);
void RUNTIME_sdisplay_allprofile ();
void RUNTIME_sdisplay_oneprofile (MORSE_kernel_t);

END_C_DECLS

#endif /* _MORSE_RUNTIME_H_ */

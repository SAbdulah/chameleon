###
#
# @copyright (c) 2009-2014 The University of Tennessee and The University
#                          of Tennessee Research Foundation.
#                          All rights reserved.
# @copyright (c) 2012-2014 Inria. All rights reserved.
# @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
#
###
#
#  @file CMakeLists.txt
#
#  @project MORSE
#  MORSE is a software package provided by:
#     Inria Bordeaux - Sud-Ouest,
#     Univ. of Tennessee,
#     King Abdullah Univesity of Science and Technology
#     Univ. of California Berkeley,
#     Univ. of Colorado Denver.
#
#  @version 0.9.0
#  @author Cedric Castagnede
#  @author Emmanuel Agullo
#  @author Mathieu Faverge
#  @date 13-07-2012
#
###

# List of codelets required by all runtimes
# -----------------------------------------
set(CODELETS_ZSRC
    codelets/codelet_ztile_zero.c
    codelets/codelet_zasum.c
    ##################
    # BLAS 1
    ##################
    codelets/codelet_zaxpy.c
    ##################
    # BLAS 3
    ##################
    codelets/codelet_zgemm.c
    codelets/codelet_zhemm.c
    codelets/codelet_zher2k.c
    codelets/codelet_zherk.c
    codelets/codelet_zsymm.c
    codelets/codelet_zsyr2k.c
    codelets/codelet_zsyrk.c
    codelets/codelet_ztrmm.c
    codelets/codelet_ztrsm.c
    ##################
    # LAPACK
    ##################
    codelets/codelet_zgeadd.c
    codelets/codelet_zlascal.c
    codelets/codelet_zgelqt.c
    codelets/codelet_zgeqrt.c
    codelets/codelet_zgessm.c
    codelets/codelet_zgessq.c
    codelets/codelet_zgetrf.c
    codelets/codelet_zgetrf_incpiv.c
    codelets/codelet_zgetrf_nopiv.c
    codelets/codelet_zhe2ge.c
    codelets/codelet_zherfb.c
    codelets/codelet_zhessq.c
    codelets/codelet_zlacpy.c
    codelets/codelet_zlange.c
    codelets/codelet_zlanhe.c
    codelets/codelet_zlansy.c
    codelets/codelet_zlantr.c
    codelets/codelet_zlaset2.c
    codelets/codelet_zlaset.c
    codelets/codelet_zlatro.c
    codelets/codelet_zlauum.c
    codelets/codelet_zplghe.c
    codelets/codelet_zplgsy.c
    codelets/codelet_zplrnt.c
    codelets/codelet_zplssq.c
    codelets/codelet_zpotrf.c
    codelets/codelet_zssssm.c
    codelets/codelet_zsyssq.c
    codelets/codelet_zsytrf_nopiv.c
    codelets/codelet_ztpqrt.c
    codelets/codelet_ztpmqrt.c
    codelets/codelet_ztradd.c
    codelets/codelet_ztrasm.c
    codelets/codelet_ztrssq.c
    codelets/codelet_ztrtri.c
    codelets/codelet_ztslqt.c
    codelets/codelet_ztsmlq.c
    codelets/codelet_ztsmqr.c
    codelets/codelet_ztsmlq_hetra1.c
    codelets/codelet_ztsmqr_hetra1.c
    codelets/codelet_ztsqrt.c
    codelets/codelet_ztstrf.c
    codelets/codelet_zttlqt.c
    codelets/codelet_zttmlq.c
    codelets/codelet_zttmqr.c
    codelets/codelet_zttqrt.c
    codelets/codelet_zunmlq.c
    codelets/codelet_zunmqr.c
    ##################
    # BUILD
    ##################
    codelets/codelet_zbuild.c
    )

# Check for the subdirectories
# ----------------------------
if( CHAMELEON_SCHED_QUARK )
    add_subdirectory(quark)
elseif( CHAMELEON_SCHED_PARSEC )
    add_subdirectory(parsec)
elseif( CHAMELEON_SCHED_STARPU )
    add_subdirectory(starpu)
endif()

###
### END CMakeLists.txt
###

###
#
# @copyright (c) 2009-2014 The University of Tennessee and The University
#                          of Tennessee Research Foundation.
#                          All rights reserved.
# @copyright (c) 2012-2016 Inria. All rights reserved.
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

if ( CHAMELEON_USE_CUDA )
  set( CHAMELEON_COPY_DIAG ON )
else()
  option(CHAMELEON_COPY_DIAG
    "This options enables the duplication of the diagonal tiles in some algorithm to avoid anti-dependencies on lower/upper triangular parts (Might be useful to StarPU)" ON)
endif()

if ( CHAMELEON_SCHED_QUARK )
  # No need for those extra diagonale tiles
  set( CHAMELEON_COPY_DIAG OFF )
endif()

mark_as_advanced(CHAMELEON_COPY_DIAG)

if (CHAMELEON_COPY_DIAG)
  add_definitions(-DCHAMELEON_COPY_DIAG)
endif()

# Define the list of sources
# --------------------------
set(CHAMELEON_CONTROL
    ../control/async.c
    ../control/auxiliary.c
    ../control/context.c
    ../control/control.c
    ../control/descriptor.c
    ../control/workspace.c
    ../control/tile.c
    ../control/morse_f77.c
    ../control/morse_mf77.c
#    ../control/morsewinthread.c
   )

set(flags_to_add "")
foreach(_prec ${CHAMELEON_PRECISION})
    set(flags_to_add "${flags_to_add} -DPRECISION_${_prec}")
endforeach()
set_source_files_properties(../control/tile.c PROPERTIES COMPILE_FLAGS "${flags_to_add}")

# Generate the morse sources for all possible precisions
# ------------------------------------------------------
set(CHAMELEON_SRCS_GENERATED "")
set(ZSRC
    ##################
    # BLAS 3
    ##################
    pzgemm.c
    pzhemm.c
    pzher2k.c
    pzherk.c
    pzsymm.c
    pzsyr2k.c
    pzsyrk.c
    pztrmm.c
    pztrsm.c
    pztrsmpl.c
    pztradd.c
    pzlascal.c
    ###
    zgeadd.c
    zlascal.c
    zgemm.c
    zhemm.c
    zher2k.c
    zherk.c
    zsymm.c
    zsyr2k.c
    zsyrk.c
    ztradd.c
    ztrmm.c
    ztrsm.c
    ztrsmpl.c
    ##################
    # LAPACK
    ##################
    pzgelqf.c
    pzgelqf_param.c
    pzgelqfrh.c
    pzgeqrf.c
    pzgeqrfrh.c
    pzgeqrf_param.c
    pzgetrf_incpiv.c
    pzgetrf_nopiv.c
    pzlacpy.c
    pzlange.c
    pzlanhe.c
    pzlansy.c
    pzlantr.c
    pzlaset2.c
    pzlaset.c
    pzlauum.c
    pzplghe.c
    pzplgsy.c
    pzplrnt.c
    pzpotrf.c
    pzsytrf.c
    pztrtri.c
    pzpotrimm.c
    pzunglq.c
    pzunglq_param.c
    pzunglqrh.c
    pzungqr.c
    pzungqr_param.c
    pzungqrrh.c
    pzunmlq.c
    pzunmlq_param.c
    pzunmlqrh.c
    pzunmqr.c
    pzunmqr_param.c
    pzunmqrrh.c
    pztpgqrt.c
    pztpqrt.c
    ###
    zgels.c
    zgels_param.c
    zgelqf.c
    zgelqf_param.c
    zgelqs.c
    zgelqs_param.c
    zgeqrf.c
    zgeqrf_param.c
    zgeqrs.c
    zgeqrs_param.c
    #zgesv.c
    zgesv_incpiv.c
    zgesv_nopiv.c
    #zgetrf.c
    zgetrf_incpiv.c
    zgetrf_nopiv.c
    zgetrs_incpiv.c
    zgetrs_nopiv.c
    zlacpy.c
    zlange.c
    zlanhe.c
    zlansy.c
    zlantr.c
    zlaset.c
    zlauum.c
    zplghe.c
    zplgsy.c
    zplrnt.c
    zposv.c
    zsysv.c
    zpotrf.c
    zsytrf.c
    zpotri.c
    zpotrimm.c
    zpotrs.c
    zsytrs.c
    ztrtri.c
    ztpgqrt.c
    ztpqrt.c
    zunglq.c
    zunglq_param.c
    zungqr.c
    zungqr_param.c
    zunmlq.c
    zunmlq_param.c
    zunmqr.c
    zunmqr_param.c
    ##################
    # MIXED PRECISION
    ##################
    pzlag2c.c
    ###
    #zcgels.c
    #zcgesv.c
    #zcposv.c
    #zcungesv.c
    ##################
    # OTHERS
    ##################
    pztile2band.c
    #pzgebrd_gb2bd.c
    pzgebrd_ge2gb.c
    #pzgetmi2.c
    #pzgetrf_reclap.c
    #pzgetrf_rectil.c
    #pzhegst.c
    #pzherbt.c
    #pzhetrd_hb2ht.c
    pzhetrd_he2hb.c
    #pzlarft_blgtrd.c
    #pzlaswp.c
    #pzlaswpc.c
    #pztrsmrv.c
    #pzunmqr_blgtrd.c
    #########################
    #zgebrd.c
    #zgecfi.c
    #zgecfi2.c
    zgesvd.c
    #zgetmi.c
    #zgetri.c
    #zgetrs.c
    #zheev.c
    zheevd.c
    #zhegst.c
    #zhegv.c
    #zhegvd.c
    zhetrd.c
    #zlaswp.c
    #zlaswpc.c
    #ztrsmrv.c
    ##################
    # CONTROL
    ##################
    #pzshift.c
    #pzpack.c
    pztile.c
    ztile.c
    ##################
    # BUILD
    ##################
    zbuild.c
    pzbuild.c
)

precisions_rules_py(CHAMELEON_SRCS_GENERATED "${ZSRC}"
                    PRECISIONS "${CHAMELEON_PRECISION}")

set(CONTROL_SRCS_GENERATED "")
set(ZSRC
    ../control/workspace_z.c
    ../control/morse_zf77.c
)

precisions_rules_py(CONTROL_SRCS_GENERATED "${ZSRC}"
                    PRECISIONS "${CHAMELEON_PRECISION}"
                    TARGETDIR "control" )

set(CHAMELEON_SRCS
    ${CHAMELEON_CONTROL}
    ${CHAMELEON_SRCS_GENERATED}
    ${CONTROL_SRCS_GENERATED}
   )

# Generate the morse fortran sources for all possible precisions
# --------------------------------------------------------------
if(HAVE_ISO_C_BINDING)
    set(CHAMELEON_SRCS_F_GENERATED "")
    set(ZSRCF
        ../control/morse_zcf90.F90
        ../control/morse_zf90.F90
        ../control/morse_zf90_wrappers.F90
       )
    precisions_rules_py(CHAMELEON_SRCS_F_GENERATED "${ZSRCF}"
                        PRECISIONS "${CHAMELEON_PRECISION}"
                        TARGETDIR "control" )

    set(CHAMELEON_SRCSF
        ../control/morse_f90.f90
        ${CHAMELEON_SRCS_F_GENERATED}
       )
endif(HAVE_ISO_C_BINDING)

# Force generation of sources
# ---------------------------
add_custom_target(chameleon_sources ALL SOURCES "${CHAMELEON_SRCS};${CHAMELEON_SRCSF}")
set(CHAMELEON_SOURCES_TARGETS "${CHAMELEON_SOURCES_TARGETS};chameleon_sources" CACHE INTERNAL "List of targets of sources")

# Compile step
# ------------
add_library(chameleon ${CHAMELEON_SRCS} ${CHAMELEON_SRCSF})
if(CHAMELEON_SCHED_STARPU)
  target_link_libraries(chameleon chameleon_starpu)
  list(INSERT CHAMELEON_DEP 0 -lchameleon_starpu)
elseif(CHAMELEON_SCHED_PARSEC)
  target_link_libraries(chameleon chameleon_parsec)
  list(INSERT CHAMELEON_DEP 0 -lchameleon_parsec)
elseif(CHAMELEON_SCHED_QUARK)
  target_link_libraries(chameleon chameleon_quark)
  list(INSERT CHAMELEON_DEP 0 -lchameleon_quark)
endif()
if (NOT CHAMELEON_SIMULATION)
  add_dependencies(chameleon
    coreblas_include)
  target_link_libraries(chameleon coreblas)
endif()
target_link_libraries(chameleon hqr)
list(INSERT CHAMELEON_DEP 0 -lchameleon)

add_dependencies(chameleon
  chameleon_include
  control_include
  chameleon_sources
)

set_property(TARGET chameleon PROPERTY LINKER_LANGUAGE Fortran)
set_property(TARGET chameleon PROPERTY Fortran_MODULE_DIRECTORY "${CMAKE_BINARY_DIR}/include")
set_property(TARGET chameleon PROPERTY INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

# installation
# ------------
install(TARGETS chameleon
        DESTINATION lib)

###
### END CMakeLists.txt
###

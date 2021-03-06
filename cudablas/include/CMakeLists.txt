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
#  @author Florent Pruvost
#  @date 16-09-2015
#
###

# Generate header files
# ---------------------
set(CUDABLAS_HDRS_GENERATED "")
set(ZHDR
    cudablas/cudablas_z.h
)
precisions_rules_py(
  CUDABLAS_HDRS_GENERATED "${ZHDR}"
  TARGETDIR cudablas
  PRECISIONS "s;d;c;z;zc;ds" )

# Define the list of headers
# --------------------------
set(CUDABLAS_HDRS
    cudablas.h
    )

# Add generated headers
# ---------------------
foreach( hdr_file ${CUDABLAS_HDRS_GENERATED} )
  list(APPEND CUDABLAS_HDRS ${CMAKE_CURRENT_BINARY_DIR}/${hdr_file})
endforeach()

# Force generation of headers
# ---------------------------
add_custom_target(cudablas_include ALL SOURCES ${CUDABLAS_HDRS})
set(CHAMELEON_SOURCES_TARGETS "${CHAMELEON_SOURCES_TARGETS};cudablas_include" CACHE INTERNAL "List of targets of sources")

# Installation
# ------------
install( FILES cudablas.h
         DESTINATION include )

install( FILES ${CUDABLAS_HDRS}
         DESTINATION include/cudablas )

###
### END CMakeLists.txt
###

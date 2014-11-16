# - Find LAPACK library
# This module finds an installed fortran library that implements the LAPACK
# linear-algebra interface (see http://www.netlib.org/lapack/).
#
# The approach follows that taken for the autoconf macro file, acx_lapack.m4
# (distributed at http://ac-archive.sourceforge.net/ac-archive/acx_lapack.html).
#
# This module sets the following variables:
#  LAPACK_FOUND - set to true if a library implementing the LAPACK interface
#    is found
#  LAPACK_LINKER_FLAGS - uncached list of required linker flags (excluding -l
#    and -L).
#  LAPACK_LIBRARIES - uncached list of libraries (using full path name) to
#    link against to use LAPACK
#  LAPACK95_LIBRARIES - uncached list of libraries (using full path name) to
#    link against to use LAPACK95
#  LAPACK95_FOUND - set to true if a library implementing the LAPACK f95
#    interface is found
#  BLA_STATIC  if set on this determines what kind of linkage we do (static)
#  BLA_VENDOR  if set checks only the specified vendor, if not set checks
#     all the possibilities
#  BLA_F95     if set on tries to find the f95 interfaces for BLAS/LAPACK
# The user can give specific paths where to find the libraries adding cmake 
# options at configure (ex: cmake path/to/project -DLAPACK_DIR=path/to/lapack):
#  LAPACK_DIR            - Where to find the base directory of lapack
#  LAPACK_INCDIR         - Where to find the header files
#  LAPACK_LIBDIR         - Where to find the library files
# Note that if BLAS_DIR is set, it will also look for lapack in it
### List of vendors (BLA_VENDOR) valid in this module
##  Intel(mkl), ACML,Apple, NAS, Generic

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)


# Some macros to print status when search for headers and libs
# PrintFindStatus.cmake is in cmake_modules/morse/find directory of magmamorse
include(PrintFindStatus)
option(LAPACK_VERBOSE "Print some additional information during LAPACK 
libraries detection" OFF)
if (BLAS_VERBOSE)
    set(LAPACK_VERBOSE ON)
endif ()
set(_lapack_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

get_property(_LANGUAGES_ GLOBAL PROPERTY ENABLED_LANGUAGES)
if (NOT _LANGUAGES_ MATCHES Fortran)
include(CheckFunctionExists)
else (NOT _LANGUAGES_ MATCHES Fortran)
include(CheckFortranFunctionExists)
endif (NOT _LANGUAGES_ MATCHES Fortran)

set(LAPACK_FOUND FALSE)
set(LAPACK95_FOUND FALSE)

# TODO: move this stuff to separate module

macro(Check_Lapack_Libraries LIBRARIES _prefix _name _flags _list _blas _threads)
# This macro checks for the existence of the combination of fortran libraries
# given by _list.  If the combination is found, this macro checks (using the
# Check_Fortran_Function_Exists macro) whether can link against that library
# combination using the name of a routine given by _name using the linker
# flags given by _flags.  If the combination of libraries is found and passes
# the link test, LIBRARIES is set to the list of complete library paths that
# have been found.  Otherwise, LIBRARIES is set to FALSE.

# N.B. _prefix is the prefix applied to the names of all cached variables that
# are generated internally and marked advanced by this macro.

set(_libraries_work TRUE)
set(${LIBRARIES})
set(_combined_name)
if (NOT _libdir)
  if (BLAS_DIR)
    list(APPEND _libdir "${BLAS_DIR}")  
    list(APPEND _libdir "${BLAS_DIR}/lib")
    list(APPEND _libdir "${BLAS_DIR}/lib32")
    list(APPEND _libdir "${BLAS_DIR}/lib64")
    list(APPEND _libdir "${BLAS_DIR}/lib/ia32")
    list(APPEND _libdir "${BLAS_DIR}/lib/intel64")
  endif ()
  if (BLAS_LIBDIR)
    list(APPEND _libdir "${BLAS_LIBDIR}")
  endif ()  
  if (LAPACK_DIR)
    list(APPEND _libdir "${LAPACK_DIR}")  
    list(APPEND _libdir "${LAPACK_DIR}/lib")
    list(APPEND _libdir "${LAPACK_DIR}/lib32")
    list(APPEND _libdir "${LAPACK_DIR}/lib64")
    list(APPEND _libdir "${LAPACK_DIR}/lib/ia32")
    list(APPEND _libdir "${LAPACK_DIR}/lib/intel64")
  endif ()
  if (LAPACK_LIBIR)
    list(APPEND _libdir "${LAPACK_LIBIR}")
  endif ()
  if (WIN32)
    string(REPLACE ":" ";" _libdir2 "$ENV{LIB}")
  elseif (APPLE)
    string(REPLACE ":" ";" _libdir2 "$ENV{DYLD_LIBRARY_PATH}")
  else ()
    string(REPLACE ":" ";" _libdir2 "$ENV{LD_LIBRARY_PATH}")
  endif ()
  list(APPEND _libdir "${_libdir2}")  
  list(APPEND _libdir "${CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES}")
  list(APPEND _libdir "${CMAKE_C_IMPLICIT_LINK_DIRECTORIES}")  
endif ()

if (LAPACK_VERBOSE)
    message("${Cyan}Try to find LAPACK libraries: ${_list}")
endif ()
    
foreach(_library ${_list})
  set(_combined_name ${_combined_name}_${_library})

  if(_libraries_work)
    if (BLA_STATIC)
      if (WIN32)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
      endif ( WIN32 )
      if (APPLE)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
      else (APPLE)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
      endif (APPLE)
    else (BLA_STATIC)
			if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # for ubuntu's libblas3gf and liblapack3gf packages
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} .so.3gf)
      endif ()
    endif (BLA_STATIC)
    find_library(${_prefix}_${_library}_LIBRARY
      NAMES ${_library}
      HINTS ${_libdir}
      )
    mark_as_advanced(${_prefix}_${_library}_LIBRARY)
    # Print status if not found
    # -------------------------
    if (NOT ${_prefix}_${_library}_LIBRARY AND LAPACK_VERBOSE)
        Print_Find_Library_Blas_Status(lapack ${_library} ${_libdir})
    endif ()
    set(${LIBRARIES} ${${LIBRARIES}} ${${_prefix}_${_library}_LIBRARY})
    set(_libraries_work ${${_prefix}_${_library}_LIBRARY})
  endif(_libraries_work)
endforeach(_library ${_list})

if(_libraries_work)
  # Test this combination of libraries.
  if(UNIX AND BLA_STATIC)
    set(CMAKE_REQUIRED_LIBRARIES ${_flags} "-Wl,--start-group" ${${LIBRARIES}} ${_blas} "-Wl,--end-group" ${_threads})
  else(UNIX AND BLA_STATIC)
    set(CMAKE_REQUIRED_LIBRARIES ${_flags} ${${LIBRARIES}} ${_blas} ${_threads})
  endif(UNIX AND BLA_STATIC)
  if (LAPACK_VERBOSE)
      message("${Cyan}LAPACK libs found. Try to compile symbol ${_name} with"
              "following libraries: ${CMAKE_REQUIRED_LIBRARIES}")
  endif ()    
  if (NOT _LANGUAGES_ MATCHES Fortran)
    check_function_exists("${_name}_" ${_prefix}${_combined_name}_WORKS)
  else (NOT _LANGUAGES_ MATCHES Fortran)
    check_fortran_function_exists(${_name} ${_prefix}${_combined_name}_WORKS)
  endif (NOT _LANGUAGES_ MATCHES Fortran)
  set(CMAKE_REQUIRED_LIBRARIES)
  mark_as_advanced(${_prefix}${_combined_name}_WORKS)
  set(_libraries_work ${${_prefix}${_combined_name}_WORKS})
endif(_libraries_work)

 if(_libraries_work)
   set(${LIBRARIES} ${${LIBRARIES}} ${_blas} ${_threads})
 else(_libraries_work)
    set(${LIBRARIES} FALSE)
 endif(_libraries_work)

endmacro(Check_Lapack_Libraries)


set(LAPACK_LINKER_FLAGS)
set(LAPACK_LIBRARIES)
set(LAPACK95_LIBRARIES)

if (NOT BLAS_FOUND)
    if(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)
    find_package(BLASEXT)
    else(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)
    find_package(BLASEXT REQUIRED)
    endif(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)
endif ()

if(BLAS_FOUND)
  set(LAPACK_LINKER_FLAGS ${BLAS_LINKER_FLAGS})
  if ($ENV{BLA_VENDOR} MATCHES ".+")
    set(BLA_VENDOR $ENV{BLA_VENDOR})
  else ($ENV{BLA_VENDOR} MATCHES ".+")
    if(NOT BLA_VENDOR)
      set(BLA_VENDOR "All")
    endif(NOT BLA_VENDOR)
  endif ($ENV{BLA_VENDOR} MATCHES ".+")

if (BLA_VENDOR STREQUAL "Goto" OR BLA_VENDOR STREQUAL "All")
 if(NOT LAPACK_LIBRARIES)
  check_lapack_libraries(
  LAPACK_LIBRARIES
  LAPACK
  cheev
  ""
  "goto2"
  "${BLAS_LIBRARIES}"
  ""
  )
 endif(NOT LAPACK_LIBRARIES)
endif (BLA_VENDOR STREQUAL "Goto" OR BLA_VENDOR STREQUAL "All")


#acml lapack
 if (BLA_VENDOR MATCHES "ACML.*" OR BLA_VENDOR STREQUAL "All")
   if (BLAS_LIBRARIES MATCHES ".+acml.+")
     set (LAPACK_LIBRARIES ${BLAS_LIBRARIES})
   endif ()
 endif ()

# Apple LAPACK library?
if (BLA_VENDOR STREQUAL "Apple" OR BLA_VENDOR STREQUAL "All")
 if(NOT LAPACK_LIBRARIES)
  check_lapack_libraries(
  LAPACK_LIBRARIES
  LAPACK
  cheev
  ""
  "Accelerate"
  "${BLAS_LIBRARIES}"
  ""
  )
 endif(NOT LAPACK_LIBRARIES)
endif (BLA_VENDOR STREQUAL "Apple" OR BLA_VENDOR STREQUAL "All")
if (BLA_VENDOR STREQUAL "NAS" OR BLA_VENDOR STREQUAL "All")
  if ( NOT LAPACK_LIBRARIES )
    check_lapack_libraries(
    LAPACK_LIBRARIES
    LAPACK
    cheev
    ""
    "vecLib"
    "${BLAS_LIBRARIES}"
    ""
    )
  endif ( NOT LAPACK_LIBRARIES )
endif (BLA_VENDOR STREQUAL "NAS" OR BLA_VENDOR STREQUAL "All")
# Generic LAPACK library?
if (BLA_VENDOR STREQUAL "Generic" OR
    BLA_VENDOR STREQUAL "ATLAS" OR
    BLA_VENDOR STREQUAL "All")
  if ( NOT LAPACK_LIBRARIES )
    check_lapack_libraries(
    LAPACK_LIBRARIES
    LAPACK
    cheev
    ""
    "lapack"
    "${BLAS_LIBRARIES}"
    ""
    )
  endif ( NOT LAPACK_LIBRARIES )
endif ()
#intel lapack
if (BLA_VENDOR MATCHES "Intel*" OR BLA_VENDOR STREQUAL "All")
  if (NOT WIN32)
    set(LM "-lm")
  endif ()
  if (_LANGUAGES_ MATCHES C OR _LANGUAGES_ MATCHES CXX)
    if(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)
      find_PACKAGE(Threads)
    else(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)
      find_package(Threads REQUIRED)
    endif(LAPACK_FIND_QUIETLY OR NOT LAPACK_FIND_REQUIRED)
    if (BLA_F95)
      if(NOT LAPACK95_LIBRARIES)
        # old
        check_lapack_libraries(
          LAPACK95_LIBRARIES
          LAPACK
          cheev
          ""
          "mkl_lapack95"
          "${BLAS95_LIBRARIES}"
          "${CMAKE_THREAD_LIBS_INIT};${LM}"
          )
      endif(NOT LAPACK95_LIBRARIES)
      if(NOT LAPACK95_LIBRARIES)
        # new >= 10.3
        check_lapack_libraries(
          LAPACK95_LIBRARIES
          LAPACK
          CHEEV
          ""
          "mkl_intel_lp64"
          "${BLAS95_LIBRARIES}"
          "${CMAKE_THREAD_LIBS_INIT};${LM}"
          )
      endif(NOT LAPACK95_LIBRARIES)
    else(BLA_F95)
      if(NOT LAPACK_LIBRARIES)
        # old
        check_lapack_libraries(
          LAPACK_LIBRARIES
          LAPACK
          cheev
          ""
          "mkl_lapack"
          "${BLAS_LIBRARIES}"
          "${CMAKE_THREAD_LIBS_INIT};${LM}"
          )
      endif(NOT LAPACK_LIBRARIES)
      if(NOT LAPACK_LIBRARIES)
        if (CMAKE_C_COMPILER MATCHES ".+gcc.*")
            set(LM "${LM};-lgomp")
        endif()
        # new >= 10.3
        check_lapack_libraries(
          LAPACK_LIBRARIES
          LAPACK
          cheev
          ""
          "mkl_intel_lp64"
          "${BLAS_LIBRARIES}"
          "${CMAKE_THREAD_LIBS_INIT};${LM}"
          )
      endif(NOT LAPACK_LIBRARIES)
    endif(BLA_F95)
  endif (_LANGUAGES_ MATCHES C OR _LANGUAGES_ MATCHES CXX)
endif(BLA_VENDOR MATCHES "Intel*" OR BLA_VENDOR STREQUAL "All")
else(BLAS_FOUND)
  message(STATUS "LAPACK requires BLAS")
endif(BLAS_FOUND)

if(BLA_F95)
 if(LAPACK95_LIBRARIES)
  set(LAPACK95_FOUND TRUE)
 else(LAPACK95_LIBRARIES)
  set(LAPACK95_FOUND FALSE)
 endif(LAPACK95_LIBRARIES)
 if(NOT LAPACK_FIND_QUIETLY)
  if(LAPACK95_FOUND)
    message(STATUS "A library with LAPACK95 API found.")
    message(STATUS "LAPACK_LIBRARIES ${LAPACK_LIBRARIES}")
  else(LAPACK95_FOUND)
    message(WARNING "BLA_VENDOR has been set to ${BLA_VENDOR} but LAPACK 95 libraries could not be found or check of symbols failed."
        "\nPlease indicate where to find LAPACK libraries. You have three options:\n"
        "- Option 1: Provide the root directory of LAPACK library with cmake option: -DLAPACK_DIR=your/path/to/lapack\n"
        "- Option 2: Provide the directory where to find BLAS libraries with cmake option: -DBLAS_LIBDIR=your/path/to/blas/libs\n"
        "- Option 3: Update your environment variable (Linux: LD_LIBRARY_PATH, Windows: LIB, Mac: DYLD_LIBRARY_PATH)\n"                
        "\nTo follow libraries detection more precisely you can activate a verbose mode with -DLAPACK_VERBOSE=ON at cmake configure."
        "\nYou could also specify a BLAS vendor to look for by setting -DBLA_VENDOR=blas_vendor_name."
        "\nList of possible BLAS vendor: Goto, ATLAS PhiPACK, CXML, DXML, SunPerf, SCSL, SGIMATH, IBMESSL, Intel10_32 (intel mkl v10 32 bit),"
        "Intel10_64lp (intel mkl v10 64 bit, lp thread model, lp64 model), Intel10_64lp_seq (intel mkl v10 64 bit, sequential code, lp64 model),"
        "Intel( older versions of mkl 32 and 64 bit), ACML, ACML_MP, ACML_GPU, Apple, NAS, Generic")   
    if(LAPACK_FIND_REQUIRED)
      message(FATAL_ERROR
      "A required library with LAPACK95 API not found. Please specify library location."
      )
    else(LAPACK_FIND_REQUIRED)
      message(STATUS
      "A library with LAPACK95 API not found. Please specify library location."
      )
    endif(LAPACK_FIND_REQUIRED)
  endif(LAPACK95_FOUND)
 endif(NOT LAPACK_FIND_QUIETLY)
 set(LAPACK_FOUND "${LAPACK95_FOUND}")
 set(LAPACK_LIBRARIES "${LAPACK95_LIBRARIES}")
else(BLA_F95)
 if(LAPACK_LIBRARIES)
  set(LAPACK_FOUND TRUE)
 else(LAPACK_LIBRARIES)
  set(LAPACK_FOUND FALSE)
 endif(LAPACK_LIBRARIES)

 if(NOT LAPACK_FIND_QUIETLY)
  if(LAPACK_FOUND)
    message(STATUS "A library with LAPACK API found.")
    message(STATUS "LAPACK_LIBRARIES ${LAPACK_LIBRARIES}")
  else(LAPACK_FOUND)
    message(WARNING "BLA_VENDOR has been set to ${BLA_VENDOR} but LAPACK libraries could not be found or check of symbols failed."
        "\nPlease indicate where to find LAPACK libraries. You have three options:\n"
        "- Option 1: Provide the root directory of LAPACK library with cmake option: -DLAPACK_DIR=your/path/to/lapack\n"
        "- Option 2: Provide the directory where to find BLAS libraries with cmake option: -DBLAS_LIBDIR=your/path/to/blas/libs\n"
        "- Option 3: Update your environment variable (Linux: LD_LIBRARY_PATH, Windows: LIB, Mac: DYLD_LIBRARY_PATH)\n"                
        "\nTo follow libraries detection more precisely you can activate a verbose mode with -DLAPACK_VERBOSE=ON at cmake configure."
        "\nYou could also specify a BLAS vendor to look for by setting -DBLA_VENDOR=blas_vendor_name."
        "\nList of possible BLAS vendor: Goto, ATLAS PhiPACK, CXML, DXML, SunPerf, SCSL, SGIMATH, IBMESSL, Intel10_32 (intel mkl v10 32 bit),"
        "Intel10_64lp (intel mkl v10 64 bit, lp thread model, lp64 model), Intel10_64lp_seq (intel mkl v10 64 bit, sequential code, lp64 model),"
        "Intel( older versions of mkl 32 and 64 bit), ACML, ACML_MP, ACML_GPU, Apple, NAS, Generic")
    if(LAPACK_FIND_REQUIRED)
      message(FATAL_ERROR
      "A required library with LAPACK API not found. Please specify library location."
      )
    else(LAPACK_FIND_REQUIRED)
      message(STATUS
      "A library with LAPACK API not found. Please specify library location."
      )
    endif(LAPACK_FIND_REQUIRED)
  endif(LAPACK_FOUND)
 endif(NOT LAPACK_FIND_QUIETLY)
endif(BLA_F95)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${_lapack_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})

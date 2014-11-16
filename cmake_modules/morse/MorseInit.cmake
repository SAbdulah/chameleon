###
#
# @copyright (c) 2009-2014 The University of Tennessee and The University 
#                          of Tennessee Research Foundation. 
#                          All rights reserved.
# @copyright (c) 2012-2014 Inria. All rights reserved.
# @copyright (c) 2012-2014 IPB. All rights reserved. 
#
###
#
#  @file MorseInit.cmake
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
#  @author Florent Pruvost
#  @date 13-07-2012
#
###

# This include is required to check symbols of libs in the main CMakeLists.txt
include(CheckFunctionExists)

# Add the path where we handle our FindFOO.cmake to seek for liraries
list(APPEND CMAKE_MODULE_PATH ${MORSE_CMAKE_MODULE_PATH}/find)

# Set some colors
if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(ColourBold  "${Esc}[1m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
    set(Blue        "${Esc}[34m")
    set(Magenta     "${Esc}[35m")
    set(Cyan        "${Esc}[36m")
    set(White       "${Esc}[37m")
    set(BoldRed     "${Esc}[1;31m")
    set(BoldGreen   "${Esc}[1;32m")
    set(BoldYellow  "${Esc}[1;33m")
    set(BoldBlue    "${Esc}[1;34m")
    set(BoldMagenta "${Esc}[1;35m")
    set(BoldCyan    "${Esc}[1;36m")
    set(BoldWhite   "${Esc}[1;37m")
endif()

# Colorize cmake messages during configure
function(message)
  list(GET ARGV 0 MessageType)
  if(MessageType STREQUAL FATAL_ERROR OR MessageType STREQUAL SEND_ERROR)
    list(REMOVE_AT ARGV 0)
    string (REPLACE ";" " " ARGV_STR "${ARGV}")
    _message(${MessageType} "${BoldRed}${ARGV_STR}${ColourReset}")
  elseif(MessageType STREQUAL WARNING)
    list(REMOVE_AT ARGV 0)
    string (REPLACE ";" " " ARGV_STR "${ARGV}")
    _message(${MessageType} "${BoldYellow}${ARGV_STR}${ColourReset}")
  elseif(MessageType STREQUAL AUTHOR_WARNING)
    list(REMOVE_AT ARGV 0)
    string (REPLACE ";" " " ARGV_STR "${ARGV}")
    _message(${MessageType} "${BoldCyan}${ARGV_STR}${ColourReset}")
  elseif(MessageType STREQUAL STATUS)
    list(REMOVE_AT ARGV 0)
    string (REPLACE ";" " " ARGV_STR "${ARGV}")
    _message(${MessageType} "${Green}${ARGV_STR}${ColourReset}")
  else()
    string (REPLACE ";" " " ARGV_STR "${ARGV}")
    string (REPLACE "${Esc}[1 " "${Esc}[1;" ARGV_STR "${ARGV_STR}")
    _message("${ARGV_STR}")
  endif()
endfunction()

##
## @end file MorseInit.cmake
##

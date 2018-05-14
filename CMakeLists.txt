#
# This file is part of SPORE.
#
# Copyright (C) 2016, the SPORE team (see AUTHORS).
#
# SPORE is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# SPORE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SPORE.  If not, see <http://www.gnu.org/licenses/>.
#
# For more information see: https://github.com/IGITUGraz/spore-nest-module
#

cmake_minimum_required( VERSION 2.8.12 )

set(MIN_NEST_VERSION 2.14.0)

set( with-python default CACHE STRING "Set python version: default|2|3|<executable> [default=version used for NEST]" )

# get information from NEST installation
execute_process(
    COMMAND nest-config --version
    RESULT_VARIABLE RES_VAR
    OUTPUT_VARIABLE NEST_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND nest-config --python-version
    RESULT_VARIABLE RES_VAR
    OUTPUT_VARIABLE NEST_PYTHON_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# check NEST version
if (NEST_VERSION STREQUAL "")
  message( "-- NEST found: NO!" )
  message( FATAL_ERROR "NEST not found! NEST is either not installed or not set up correctly. If required, you can specify the path to the program `nest-config` by hand by setting -Dwith-nest=..." )
else()
  string(REPLACE "." ";" VERSION_LIST ${NEST_VERSION})
  list(GET VERSION_LIST 0 NEST_VERSION_MAJOR)
  list(GET VERSION_LIST 1 NEST_VERSION_MINOR)
  list(GET VERSION_LIST 2 NEST_VERSION_PATCH)
  string(REPLACE "." ";" VERSION_LIST ${MIN_NEST_VERSION})
  list(GET VERSION_LIST 0 MIN_NEST_VERSION_MAJOR)
  list(GET VERSION_LIST 1 MIN_NEST_VERSION_MINOR)
  list(GET VERSION_LIST 2 MIN_NEST_VERSION_PATCH)
  if (NEST_VERSION_MAJOR GREATER_EQUAL ${MIN_NEST_VERSION_MAJOR} AND
      NEST_VERSION_MINOR GREATER_EQUAL ${MIN_NEST_VERSION_MINOR} AND
      NEST_VERSION_PATCH GREATER_EQUAL ${MIN_NEST_VERSION_PATCH})
    message( "-- NEST found: " ${NEST_VERSION} )
  else()
    message( "-- NEST found: " ${NEST_VERSION} " (< ${MIN_NEST_VERSION}, ignored)"  )
  endif()
  message( "-- Python version suggested by NEST installation: " ${NEST_PYTHON_VERSION} )
endif()

# select python version
if ( ${with-python} STREQUAL "default" OR  ${with-python} STREQUAL "2" OR  ${with-python} STREQUAL "3" )
  if ( ${with-python} STREQUAL "default" )
    find_package( PythonInterp ${NEST_PYTHON_VERSION} REQUIRED )
  elseif ( ${with-python} STREQUAL "2" )
    find_package( PythonInterp 2 REQUIRED )
  elseif ( ${with-python} STREQUAL "3" )
    find_package( PythonInterp 3 REQUIRED )
  endif ()

  if ( NOT PYTHONINTERP_FOUND )
    message( FATAL_ERROR "Could not locate python installation" )
  endif ()
elseif ( EXISTS ${with-python} )
    set( PYTHON_EXECUTABLE ${with-python} )
else ()
  message( FATAL_ERROR "Invalid option: -Dwith-python=" ${with-python} )
endif ()

enable_testing()

# Include SPORE library
add_subdirectory( src )

# Include unit tests
add_subdirectory( unittests )


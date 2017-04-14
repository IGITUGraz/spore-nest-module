#
# This file is part of SPORE.
#
# Copyright (C) 2016 Institute for Theoretical Computer Science, Graz University of Technology
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

set( with-python default CACHE STRING "Set python version: default|2|3|<executable> [default=system default]" )

if ( ${with-python} STREQUAL "default" OR  ${with-python} STREQUAL "2" OR  ${with-python} STREQUAL "3" )
  if ( ${with-python} STREQUAL "default" )
    find_package( PythonInterp )
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


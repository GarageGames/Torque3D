# -----------------------------------------------------------------------------
# Copyright (c) 2014 GarageGames, LLC
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
# -----------------------------------------------------------------------------

project(${TORQUE_APP_NAME})

# TODO: fmod support

###############################################################################
# process modules
###############################################################################
file(GLOB modules "modules/*.cmake")
foreach(module ${modules})
    string(FIND ${module} ".target.cmake" FIND_RESULT)
    if( ${FIND_RESULT} STREQUAL "-1")
        include(${module})
    endif()
endforeach()


###############################################################################
###############################################################################
finishExecutable()
###############################################################################
###############################################################################


###############################################################################
# process modules with target requeriment
###############################################################################
file(GLOB modules "modules/*.target.cmake")
foreach(module ${modules})
    include(${module})
endforeach()


###############################################################################
# configure files
###############################################################################
message(STATUS "writing ${projectSrcDir}/torqueConfig.h")
CONFIGURE_FILE("${cmakeDir}/torqueConfig.h.in" "${projectSrcDir}/torqueConfig.h")

# configure the relevant files only once
if(NOT EXISTS "${projectSrcDir}/torque.ico")
    CONFIGURE_FILE("${cmakeDir}/torque.ico" "${projectSrcDir}/torque.ico" COPYONLY)
endif()
if(NOT EXISTS "${projectOutDir}/${PROJECT_NAME}.torsion")
    CONFIGURE_FILE("${cmakeDir}/template.torsion.in" "${projectOutDir}/${PROJECT_NAME}.torsion")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/main.cs.in" AND NOT EXISTS "${projectOutDir}/main.cs")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game/main.cs.in" "${projectOutDir}/main.cs")
endif()


###############################################################################
# Installation
###############################################################################

if(TORQUE_TEMPLATE)
    message("Prepare Template(${TORQUE_TEMPLATE}) install...")
    INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/Templates/${TORQUE_TEMPLATE}/game"                 DESTINATION "${TORQUE_APP_DIR}")
endif()

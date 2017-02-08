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

project("Torque3DEngine")

if( CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8 )
    set( TORQUE_CPU_X64 ON )
elseif( CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 4 )
    set( TORQUE_CPU_X32 ON )
endif()

if(NOT TORQUE_TEMPLATE)
    set(TORQUE_TEMPLATE "Full" CACHE STRING "the template to use")
endif()
if(NOT TORQUE_APP_DIR)
    set(TORQUE_APP_DIR "${CMAKE_SOURCE_DIR}/My Projects/${TORQUE_APP_NAME}")
endif()
if(NOT projectOutDir)
    set(projectOutDir "${TORQUE_APP_DIR}/game")
endif()
if(NOT projectSrcDir)
    set(projectSrcDir "${TORQUE_APP_DIR}/source")
endif()
set(libDir        "${CMAKE_SOURCE_DIR}/Engine/lib")
set(srcDir        "${CMAKE_SOURCE_DIR}/Engine/source")
set(cmakeDir      "${CMAKE_SOURCE_DIR}/Tools/CMake")

# hide some things
mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(CMAKE_CONFIGURATION_TYPES)

# output folders
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${projectOutDir}/game)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${projectOutDir}/game)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${projectOutDir}/game)

###############################################################################
### Source File Handling
###############################################################################

# finds and adds sources files in a folder
macro(addPath dir)
    set(tmp_files "")
    set(glob_config GLOB)
    if(${ARGC} GREATER 1 AND "${ARGV1}" STREQUAL "REC")
        set(glob_config GLOB_RECURSE)
    endif()
	set(mac_files "")
	if(APPLE)
		set(mac_files ${dir}/*.mm ${dir}/*.m)
	endif()
    file(${glob_config} tmp_files
             ${dir}/*.cpp
             ${dir}/*.c
             ${dir}/*.cc
             ${dir}/*.h
             ${mac_files}
             #${dir}/*.asm
             )
    foreach(entry ${BLACKLIST})
 		list(REMOVE_ITEM tmp_files ${dir}/${entry})
 	endforeach()
    LIST(APPEND ${PROJECT_NAME}_files "${tmp_files}")
    LIST(APPEND ${PROJECT_NAME}_paths "${dir}")
    #message(STATUS "addPath ${PROJECT_NAME} : ${tmp_files}")
endmacro()

# adds a file to the sources
macro(addFile filename)
    LIST(APPEND ${PROJECT_NAME}_files "${filename}")
    #message(STATUS "addFile ${PROJECT_NAME} : ${filename}")
endmacro()

# finds and adds sources files in a folder recursively
macro(addPathRec dir)
    addPath("${dir}" "REC")
endmacro()

###############################################################################
### Definition Handling
###############################################################################
macro(__addDef def config)
    # two possibilities: a) target already known, so add it directly, or b) target not yet known, so add it to its cache
    if(TARGET ${PROJECT_NAME})
        #message(STATUS "directly applying defs: ${PROJECT_NAME} with config ${config}: ${def}")
        if("${config}" STREQUAL "")
            set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPILE_DEFINITIONS ${def})
        else()
            set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:${config}>:${def}>)
        endif()
    else()
        if("${config}" STREQUAL "")
            list(APPEND ${PROJECT_NAME}_defs_ ${def})
        else()
            list(APPEND ${PROJECT_NAME}_defs_ $<$<CONFIG:${config}>:${def}>)
        endif()
        #message(STATUS "added definition to cache: ${PROJECT_NAME}_defs_: ${${PROJECT_NAME}_defs_}")
    endif()
endmacro()

# adds a definition: argument 1: Nothing(for all), _DEBUG, _RELEASE, <more build configurations>
macro(addDef def)
    set(def_configs "")
    if(${ARGC} GREATER 1)
        foreach(config ${ARGN})
            __addDef(${def} ${config})
        endforeach()
    else()
        __addDef(${def} "")
    endif()
endmacro()

# this applies cached definitions onto the target
macro(_process_defs)
    if(DEFINED ${PROJECT_NAME}_defs_)
        set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPILE_DEFINITIONS ${${PROJECT_NAME}_defs_})
        #message(STATUS "applying defs to project ${PROJECT_NAME}: ${${PROJECT_NAME}_defs_}")
    endif()
endmacro()

###############################################################################
###  Source Library Handling
###############################################################################
macro(addLibSrc libPath)
    set(cached_project_name ${PROJECT_NAME})
    include(${libPath})
    project(${cached_project_name})
endmacro()

###############################################################################
### Linked Library Handling
###############################################################################
macro(addLib libs)
   foreach(lib ${libs})
        # check if we can build it ourselfs
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libraries/${lib}.cmake")
            addLibSrc("${CMAKE_CURRENT_SOURCE_DIR}/libraries/${lib}.cmake")
        endif()
        # then link against it
        # two possibilities: a) target already known, so add it directly, or b) target not yet known, so add it to its cache
        if(TARGET ${PROJECT_NAME})
            target_link_libraries(${PROJECT_NAME} "${lib}")
        else()
            list(APPEND ${PROJECT_NAME}_libs ${lib})
        endif()
   endforeach()
endmacro()

#addLibRelease will add to only release builds
macro(addLibRelease libs)
   foreach(lib ${libs})
        # check if we can build it ourselfs
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libraries/${lib}.cmake")
            addLibSrc("${CMAKE_CURRENT_SOURCE_DIR}/libraries/${lib}.cmake")
        endif()
        # then link against it
        # two possibilities: a) target already known, so add it directly, or b) target not yet known, so add it to its cache
        if(TARGET ${PROJECT_NAME})
            target_link_libraries(${PROJECT_NAME} optimized "${lib}")
        else()
            list(APPEND ${PROJECT_NAME}_libsRelease ${lib})
        endif()
   endforeach()
endmacro()

#addLibDebug will add to only debug builds
macro(addLibDebug libs)
   foreach(lib ${libs})
        # check if we can build it ourselfs
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libraries/${lib}.cmake")
            addLibSrc("${CMAKE_CURRENT_SOURCE_DIR}/libraries/${lib}.cmake")
        endif()
        # then link against it
        # two possibilities: a) target already known, so add it directly, or b) target not yet known, so add it to its cache
        if(TARGET ${PROJECT_NAME})
            target_link_libraries(${PROJECT_NAME} debug "${lib}")
        else()
            list(APPEND ${PROJECT_NAME}_libsDebug ${lib})
        endif()
   endforeach()
endmacro()

# this applies cached definitions onto the target
macro(_process_libs)
    if(DEFINED ${PROJECT_NAME}_libs)
        target_link_libraries(${PROJECT_NAME} "${${PROJECT_NAME}_libs}")
    endif()
    if(DEFINED ${PROJECT_NAME}_libsRelease)
        target_link_libraries(${PROJECT_NAME} optimized "${${PROJECT_NAME}_libsRelease}")
    endif()
    if(DEFINED ${PROJECT_NAME}_libsDebug)
        target_link_libraries(${PROJECT_NAME} debug "${${PROJECT_NAME}_libsDebug}")
    endif()

endmacro()

# apple frameworks
macro(addFramework framework)
	if (APPLE)
		addLib("-framework ${framework}")
	endif()
endmacro()

###############################################################################
### Include Handling
###############################################################################
macro(addInclude incPath)
    if(TARGET ${PROJECT_NAME})
        set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY INCLUDE_DIRECTORIES "${incPath}")
    else()
        list(APPEND ${PROJECT_NAME}_includes ${incPath})
    endif()
endmacro()

# this applies cached definitions onto the target
macro(_process_includes)
    if(DEFINED ${PROJECT_NAME}_includes)
        set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY INCLUDE_DIRECTORIES "${${PROJECT_NAME}_includes}")
    endif()
endmacro()

###############################################################################

macro(_postTargetProcess)
    _process_includes()
    _process_defs()
    _process_libs()
endmacro()

# adds a path to search for libs
macro(addLibPath dir)
    link_directories(${dir})
endmacro()

# creates a proper filter for VS
macro(generateFilters relDir)
    foreach(f ${${PROJECT_NAME}_files})
        # Get the path of the file relative to ${DIRECTORY},
        # then alter it (not compulsory)
        file(RELATIVE_PATH SRCGR ${relDir} ${f})
        set(SRCGR "${PROJECT_NAME}/${SRCGR}")
        # Extract the folder, ie remove the filename part
        string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
        # do not have any ../ dirs
        string(REPLACE "../" "" SRCGR ${SRCGR})
        # Source_group expects \\ (double antislash), not / (slash)
        string(REPLACE / \\ SRCGR ${SRCGR})
        #STRING(REPLACE "//" "/" SRCGR ${SRCGR})
        #message(STATUS "FILE: ${f} -> ${SRCGR}")
        source_group("${SRCGR}" FILES ${f})
    endforeach()
endmacro()

# creates a proper filter for VS
macro(generateFiltersSpecial relDir)
    foreach(f ${${PROJECT_NAME}_files})
        # Get the path of the file relative to ${DIRECTORY},
        # then alter it (not compulsory)
        file(RELATIVE_PATH SRCGR ${relDir} ${f})
        set(SRCGR "torque3d/${SRCGR}")
        # Extract the folder, ie remove the filename part
        string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
        # do not have any ../ dirs
        string(REPLACE "../" "" SRCGR ${SRCGR})
        IF("${SRCGR}" MATCHES "^torque3d/My Projects/.*$")
            string(REPLACE "torque3d/My Projects/${PROJECT_NAME}/" "" SRCGR ${SRCGR})
            string(REPLACE "/source" "" SRCGR ${SRCGR})
        endif()
        # Source_group expects \\ (double antislash), not / (slash)
        string(REPLACE / \\ SRCGR ${SRCGR})
        #STRING(REPLACE "//" "/" SRCGR ${SRCGR})
        IF(EXISTS "${f}" AND NOT IS_DIRECTORY "${f}")
            #message(STATUS "FILE: ${f} -> ${SRCGR}")
            source_group("${SRCGR}" FILES ${f})
        endif()
    endforeach()
endmacro()

# macro to add a static library
macro(finishLibrary)
    # more paths?
    if(${ARGC} GREATER 0)
        foreach(dir ${ARGV0})
            addPath("${dir}")
        endforeach()
    endif()
    # now inspect the paths we got
    set(firstDir "")
    foreach(dir ${${PROJECT_NAME}_paths})
        if("${firstDir}" STREQUAL "")
            set(firstDir "${dir}")
        endif()
    endforeach()
    generateFilters("${firstDir}")

    # set per target compile flags
    if(TORQUE_CXX_FLAGS_${PROJECT_NAME})
        set_source_files_properties(${${PROJECT_NAME}_files} PROPERTIES COMPILE_FLAGS "${TORQUE_CXX_FLAGS_${PROJECT_NAME}}")
    else()
        set_source_files_properties(${${PROJECT_NAME}_files} PROPERTIES COMPILE_FLAGS "${TORQUE_CXX_FLAGS_LIBS}")
    endif()

    if(TORQUE_STATIC)
        add_library("${PROJECT_NAME}" STATIC ${${PROJECT_NAME}_files})
    else()
        add_library("${PROJECT_NAME}" SHARED ${${PROJECT_NAME}_files})
    endif()

    # omg - only use the first folder ... otherwise we get lots of header name collisions
    #foreach(dir ${${PROJECT_NAME}_paths})
    addInclude("${firstDir}")
    #endforeach()

    _postTargetProcess()
endmacro()

# macro to add an executable
macro(finishExecutable)
    # now inspect the paths we got
    set(firstDir "")
    foreach(dir ${${PROJECT_NAME}_paths})
        if("${firstDir}" STREQUAL "")
            set(firstDir "${dir}")
        endif()
    endforeach()
    generateFiltersSpecial("${firstDir}")

    # set per target compile flags
    if(TORQUE_CXX_FLAGS_${PROJECT_NAME})
        set_source_files_properties(${${PROJECT_NAME}_files} PROPERTIES COMPILE_FLAGS "${TORQUE_CXX_FLAGS_${PROJECT_NAME}}")
    else()
        set_source_files_properties(${${PROJECT_NAME}_files} PROPERTIES COMPILE_FLAGS "${TORQUE_CXX_FLAGS_EXECUTABLES}")
    endif()

    if (APPLE)
      set(ICON_FILE "${projectSrcDir}/torque.icns")
        set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        add_executable("${PROJECT_NAME}" MACOSX_BUNDLE ${ICON_FILE} ${${PROJECT_NAME}_files})
    else()
        add_executable("${PROJECT_NAME}" WIN32 ${${PROJECT_NAME}_files})
    endif()
    addInclude("${firstDir}")

    _postTargetProcess()
endmacro()

macro(setupVersionNumbers)
    set(TORQUE_APP_VERSION_MAJOR 1 CACHE INTEGER "")
    set(TORQUE_APP_VERSION_MINOR 0 CACHE INTEGER "")
    set(TORQUE_APP_VERSION_PATCH 0 CACHE INTEGER "")
    set(TORQUE_APP_VERSION_TWEAK 0 CACHE INTEGER "")
    mark_as_advanced(TORQUE_APP_VERSION_TWEAK)
    MATH(EXPR TORQUE_APP_VERSION "${TORQUE_APP_VERSION_MAJOR} * 1000 + ${TORQUE_APP_VERSION_MINOR} * 100 + ${TORQUE_APP_VERSION_PATCH} * 10 + ${TORQUE_APP_VERSION_TWEAK}")
    set(TORQUE_APP_VERSION_STRING "${TORQUE_APP_VERSION_MAJOR}.${TORQUE_APP_VERSION_MINOR}.${TORQUE_APP_VERSION_PATCH}.${TORQUE_APP_VERSION_TWEAK}")
    #message(STATUS "version numbers: ${TORQUE_APP_VERSION} / ${TORQUE_APP_VERSION_STRING}")
endmacro()

macro(setupPackaging)
    INCLUDE(CPack)
    # only enable zips for now
    set(CPACK_BINARY_NSIS OFF CACHE INTERNAL "" FORCE)
    set(CPACK_BINARY_ZIP   ON CACHE INTERNAL "" FORCE)
    set(CPACK_SOURCE_ZIP  OFF CACHE INTERNAL "" FORCE)
    SET(CPACK_GENERATOR "ZIP")
    SET(CPACK_PACKAGE_VENDOR "${PROJECT_NAME}")
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}")
    SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)
    SET(CPACK_OUTPUT_FILE_PREFIX "${TORQUE_APP_DIR}/packages/${PROJECT_NAME}")
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "")
    #SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
    #SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
    SET(CPACK_PACKAGE_VERSION_MAJOR "${TORQUE_APP_VERSION_MAJOR}")
    SET(CPACK_PACKAGE_VERSION_MINOR "${TORQUE_APP_VERSION_MINOR}")
    SET(CPACK_PACKAGE_VERSION_PATCH "${TORQUE_APP_VERSION_PATCH}")
    #SET(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "${PROJECT_NAME}")
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${TORQUE_APP_VERSION_STRING}")
    #SET(CPACK_SOURCE_STRIP_FILES "")
endmacro()
# always static for now
set(TORQUE_STATIC ON)
#option(TORQUE_STATIC "enables or disable static" OFF)

if(WIN32)
    set(TORQUE_CXX_FLAGS_EXECUTABLES "/wd4018 /wd4100 /wd4121 /wd4127 /wd4130 /wd4244 /wd4245 /wd4389 /wd4511 /wd4512 /wd4800 /wd4995 " CACHE TYPE STRING)
    mark_as_advanced(TORQUE_CXX_FLAGS_EXECUTABLES)

    set(TORQUE_CXX_FLAGS_LIBS "/W0" CACHE TYPE STRING)
    mark_as_advanced(TORQUE_CXX_FLAGS_LIBS)

    set(TORQUE_CXX_FLAGS_COMMON_DEFAULT "-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS /MP /O2 /Ob2 /Oi /Ot /Oy /GT /Zi /W4 /nologo /GF /EHsc /GS- /Gy- /Qpar- /fp:precise /fp:except- /GR /Zc:wchar_t-" )
    if( TORQUE_CPU_X32 )
       set(TORQUE_CXX_FLAGS_COMMON_DEFAULT "${TORQUE_CXX_FLAGS_COMMON_DEFAULT} /arch:SSE2")
    endif()
    set(TORQUE_CXX_FLAGS_COMMON ${TORQUE_CXX_FLAGS_COMMON_DEFAULT} CACHE TYPE STRING)

    mark_as_advanced(TORQUE_CXX_FLAGS_COMMON)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORQUE_CXX_FLAGS_COMMON}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "/LARGEADDRESSAWARE")
    #set(STATIC_LIBRARY_FLAGS "/OPT:NOREF")

    # Force static runtime libraries
    if(TORQUE_STATIC)
        FOREACH(flag
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_DEBUG_INIT
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_DEBUG_INIT)
            STRING(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
            SET("${flag}" "${${flag}} /EHsc")
        ENDFOREACH()
    endif()
else()
    # TODO: improve default settings on other platforms
    set(TORQUE_CXX_FLAGS_EXECUTABLES "" CACHE TYPE STRING)
    mark_as_advanced(TORQUE_CXX_FLAGS_EXECUTABLES)
    set(TORQUE_CXX_FLAGS_LIBS "" CACHE TYPE STRING)
    mark_as_advanced(TORQUE_CXX_FLAGS_LIBS)
    set(TORQUE_CXX_FLAGS_COMMON "" CACHE TYPE STRING)
    mark_as_advanced(TORQUE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORQUE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")
endif()

if(UNIX AND NOT APPLE)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${projectOutDir}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${projectOutDir}")
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${projectOutDir}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${projectOutDir}")
endif()

if(APPLE)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${projectOutDir}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${projectOutDir}")
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${projectOutDir}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${projectOutDir}")
endif()

# fix the debug/release subfolders on windows
if(MSVC)
    SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY" "${projectOutDir}")
    FOREACH(CONF ${CMAKE_CONFIGURATION_TYPES})
        # Go uppercase (DEBUG, RELEASE...)
        STRING(TOUPPER "${CONF}" CONF)
        #SET("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${projectOutDir}")
        SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${projectOutDir}")
    ENDFOREACH()
endif()

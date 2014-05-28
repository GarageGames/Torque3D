project("Torque3DEngine")

set(TORQUE_TEMPLATE "Full" CACHE STRING "the template to use")

if(NOT projectDir)
    set(projectDir    "${CMAKE_SOURCE_DIR}/My Projects/${TORQUE_APP_NAME}")
endif()
if(NOT projectOutDir)
    set(projectOutDir "${projectDir}/game")
endif()
if(NOT projectSrcDir)
    set(projectSrcDir "${projectDir}/source")
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

# finds and adds sources files in a folder
macro(addPath dir)
    set(tmpa "")
    file(GLOB tmpa
             ${dir}/*.cpp
             ${dir}/*.c
             ${dir}/*.cc
             ${dir}/*.h)
    LIST(APPEND ${PROJECT_NAME}_files "${tmpa}")
    LIST(APPEND ${PROJECT_NAME}_paths "${dir}")
    #message(STATUS "addPath ${PROJECT_NAME} : ${tmpa}")
    #set(t "${${t}};${tmpa}")
endmacro()

# adds a file to the sources
macro(addFile filename)
    LIST(APPEND ${PROJECT_NAME}_files "${filename}")
    #message(STATUS "addFile ${PROJECT_NAME} : ${filename}")
endmacro()

# finds and adds sources files in a folder recursively
macro(addPathRec dir)
    set(tmpa "")
    file(GLOB_RECURSE tmpa
             ${dir}/*.cpp
             ${dir}/*.c
             ${dir}/*.cc
             ${dir}/*.h)
    LIST(APPEND ${PROJECT_NAME}_files "${tmpa}")
    LIST(APPEND ${PROJECT_NAME}_paths "${dir}")
    #message(STATUS "addPathRec ${PROJECT_NAME} : ${tmpa}")
endmacro()

# adds a definition
macro(addDef def)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPILE_DEFINITIONS "${def}")
endmacro()
# adds a definition
macro(addDebugDef def)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG "${def}")
endmacro()

# adds a required definition. Are processed on addExecutable or addStaticLib
macro(addRequiredDefinition def)
    #message(STATUS "${PROJECT_NAME} : add def : ${def}")
    LIST( APPEND ${PROJECT_NAME}_required_definition ${def} )    
endmacro()
# adds a required debug definition. Are processed on addExecutable or addStaticLib
macro(addRequiredDebugDefinition def)
    #message(STATUS "${PROJECT_NAME} : add def : ${def}")
    LIST( APPEND ${PROJECT_NAME}_required_debug_definition ${def} )    
endmacro()

# add definitions to project
macro( _processProjectDefinition )
   foreach( def ${${PROJECT_NAME}_required_definition} )
       addDef( ${def} )
   endforeach()   
   
   foreach( def ${${PROJECT_NAME}_required_debug_definition} )
       addDebugDef( ${def} )
   endforeach()
   
   #clear required defs
   set( ${PROJECT_NAME}_required_definition )
   set( ${PROJECT_NAME}_required_debug_definition )
endmacro()

# adds an include path
macro(addInclude incPath)
    #message(STATUS "${PROJECT_NAME} : add include path : ${incPath}")
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY INCLUDE_DIRECTORIES "${incPath}")
endmacro()

# adds a library to link against
macro(addLib lib)
    #message(STATUS "${PROJECT_NAME} : add lib : ${lib}")
    target_link_libraries(${PROJECT_NAME} "${lib}")
endmacro()

# adds a library dependency. Are processed on addExecutable or addStaticLib
macro(addRequiredLibrary lib)
    #message(STATUS "${PROJECT_NAME} : add lib : ${lib}")
    LIST( APPEND ${PROJECT_NAME}_required_library ${lib} )    
endmacro()

# adds a link dependency. Are processed on addExecutable or addStaticLib
macro(addRequiredLink lib)
    #message(STATUS "${PROJECT_NAME} : add lib : ${lib}")
    LIST( APPEND ${PROJECT_NAME}_required_link ${lib} )    
endmacro()

macro( _processProjectLibrary )
   # Append currect project to PROJECT_STACK
   LIST( APPEND PROJECT_STACK "${PROJECT_NAME}" )
   
   foreach( lib ${${PROJECT_NAME}_required_library} )
       #message( "adding library dependency: ${lib}" )
       include( ${lib} )
   endforeach()
   
   #clear required libraries
   set( ${PROJECT_NAME}_required_library )
   
   # pop currect project form PROJECT_STACK
   LIST(REMOVE_AT PROJECT_STACK -1)
   
   # get currect project form stack
   if( PROJECT_STACK )      
      LIST(GET PROJECT_STACK -1 TEMP_PROJECT)
      project( ${TEMP_PROJECT} )
   endif()
   
   
endmacro()

macro( _processProjectLinks )
   #message( "_processProjectLinks: ${PROJECT_NAME}" )
   foreach( lib ${${PROJECT_NAME}_required_link} )
       addLib( ${lib} )
   endforeach()
   
   #clear required libraries
   set( ${PROJECT_NAME}_required_link )
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
macro(addStaticLib)
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
    if(TORQUE_STATIC)
        add_library("${PROJECT_NAME}" STATIC ${${PROJECT_NAME}_files})
    else()
        add_library("${PROJECT_NAME}" SHARED ${${PROJECT_NAME}_files})
    endif()
    # omg - only use the first folder ... otehrwise we get lots of header name collisions
    #foreach(dir ${${PROJECT_NAME}_paths})
    addInclude("${firstDir}")
    #endforeach()
    
    _processProjectLinks()
    _processProjectLibrary()
    _processProjectDefinition()
endmacro()

# macro to add an executable
macro(addExecutable)
    # now inspect the paths we got
    set(firstDir "")
    foreach(dir ${${PROJECT_NAME}_paths})
        if("${firstDir}" STREQUAL "")
            set(firstDir "${dir}")
        endif()
    endforeach()
    generateFiltersSpecial("${firstDir}")
    add_executable("${PROJECT_NAME}" WIN32 ${${PROJECT_NAME}_files})
    # omg - only use the first folder ... otehrwise we get lots of header name collisions
    addInclude("${firstDir}")
    
    _processProjectLinks()
    _processProjectLibrary()
    _processProjectDefinition()
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
    SET(CPACK_OUTPUT_FILE_PREFIX "${projectDir}/packages/${PROJECT_NAME}")
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
    set(TORQUE_CXX_FLAGS "/MP /O2 /Ob2 /Oi /Ot /Oy /GT /Zi /W4 /nologo /GF /EHsc /GS- /Gy- /Qpar- /arch:SSE2 /fp:fast /fp:except- /GR /Zc:wchar_t- /wd4018 /wd4100 /wd4121 /wd4127 /wd4130 /wd4244 /wd4245 /wd4389 /wd4511 /wd4512 /wd4800 /wd4995 /D_CRT_SECURE_NO_WARNINGS " CACHE TYPE STRING)
    mark_as_advanced(TORQUE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORQUE_CXX_FLAGS}")
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
endif()


# fix the debug/release subfolders on windows
if(MSVC)
    FOREACH(CONF ${CMAKE_CONFIGURATION_TYPES})
        # Go uppercase (DEBUG, RELEASE...)
        STRING(TOUPPER "${CONF}" CONF)
        #SET("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${projectOutDir}")
        SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${projectOutDir}")
    ENDFOREACH()
endif()

project("Torque3DEngine")

set(TORQUE_TEMPLATE "Empty" CACHE STRING "the template to use")

set(projectOutDir "${CMAKE_SOURCE_DIR}/My Projects/${TORQUE_TEMPLATE}")
set(projectSrcDir "${CMAKE_SOURCE_DIR}/My Projects/${TORQUE_TEMPLATE}/source")
set(libDir "${CMAKE_SOURCE_DIR}/Engine/lib")
set(srcDir "${CMAKE_SOURCE_DIR}/Engine/source")
set(cmakeDir "${CMAKE_SOURCE_DIR}/Tools/CMake")

# output folders
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${projectOutDir}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${projectOutDir}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${projectOutDir}/bin)

SET(CMAKE_INSTALL_PREFIX "${projectOutDir}" CACHE INTERNAL "Prefix prepended to install directories" FORCE)


function(addLibPath varname dir mode)
    set(tmpa "")
    file(${mode} tmpa
             ${dir}/*.cpp
             ${dir}/*.c
             ${dir}/*.cc
             ${dir}/*.h)
    set("${varname}" "${${varname}};${tmpa}" PARENT_SCOPE)
endfunction()

function(addLibraryFinal paths libName relDir)
    foreach(f ${paths})
        # Get the path of the file relative to ${DIRECTORY},
        # then alter it (not compulsory)
        file(RELATIVE_PATH SRCGR ${relDir} ${f})
        set(SRCGR "${libName}/${SRCGR}")            
        # Extract the folder, ie remove the filename part
        string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
        # Source_group expects \\ (double antislash), not / (slash)
        string(REPLACE / \\ SRCGR ${SRCGR})
        source_group("${SRCGR}" FILES ${f})
    endforeach()
    add_library("${libName}" STATIC ${paths})
endfunction()

function(addExecutableFinal paths exeName relDir)
    foreach(f ${paths})
        # Get the path of the file relative to ${DIRECTORY},
        # then alter it (not compulsory)
        file(RELATIVE_PATH SRCGR ${relDir} ${f})
        set(SRCGR "${exeName}/${SRCGR}")            
        # Extract the folder, ie remove the filename part
        string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
        # Source_group expects \\ (double antislash), not / (slash)
        string(REPLACE / \\ SRCGR ${SRCGR})
        source_group("${SRCGR}" FILES ${f})
    endforeach()
    add_executable("${exeName}" WIN32 ${paths})
endfunction()

function(addLibrary dirs libName mode)
    set(tmp "")
    set(firstDir "")
    #message(STATUS "${libName}")
    foreach(dir ${dirs})
        if("${firstDir}" STREQUAL "")
            set(firstDir "${dir}")
        endif()
        addLibPath(tmp "${dir}" ${mode})
    endforeach()
    addLibraryFinal("${tmp}" "${libName}" "${firstDir}")
    foreach(dir ${dirs})
        set_property(TARGET "${libName}" PROPERTY INCLUDE_DIRECTORIES "${dir}")
    endforeach()
endfunction()

if(WIN32)
    # default disabled warnings: 4018;4100;4121;4127;4130;4244;4245;4389;4511;4512;4800;
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /O2 /Ob2 /Oi /Ot /Oy /GT /Zi /W2 /nologo /GF /EHsc /GS- /Gy- /Qpar- /arch:SSE2 /fp:fast /fp:except- /GR /Zc:wchar_t-")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")
    #set(CMAKE_EXE_LINKER_FLAGS "/OPT:NOREF")
    #set(STATIC_LIBRARY_FLAGS "/OPT:NOREF")
    
    # Force static runtime libraries
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




project("Torque3DEngine")

function(addLibPath varname dir mode)
    set(tmpa "")
    file(${mode} tmpa
             ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.c
             ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h)
    set("${varname}" "${${varname}};${tmpa}" PARENT_SCOPE)
endfunction()

function(addLibraryFinal paths libName)
    foreach(f ${paths})
        # Get the path of the file relative to ${DIRECTORY},
        # then alter it (not compulsory)
        file(RELATIVE_PATH SRCGR ${CMAKE_CURRENT_SOURCE_DIR} ${f})
        set(SRCGR "${libName}/${SRCGR}")            
        # Extract the folder, ie remove the filename part
        string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
        # Source_group expects \\ (double antislash), not / (slash)
        string(REPLACE / \\ SRCGR ${SRCGR})
        source_group("${SRCGR}" FILES ${f})
    endforeach()
    add_library("${libName}" STATIC ${paths})
endfunction()

function(addLibrary dirs libName mode)
    set(tmp "")
    foreach(dir ${dirs})
        #message(STATUS " - ${libName} : ${dir}")
        addLibPath(tmp "${dir}" ${mode})
    endforeach()
    addLibraryFinal("${tmp}" "${libName}")
endfunction()

if(WIN32)
    # default disabled warnings: 4018;4100;4121;4127;4130;4244;4245;4389;4511;4512;4800;
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /O2 /Ob2 /Oi /Ot /Oy /GT /Zi /W2 /nologo /GF /EHsc /GS- /Gy- /Qpar- /arch:SSE2 /fp:fast /fp:except- /MT /GR /Zc:wchar_t-")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")
endif()

# the projects
add_subdirectory(lib/lmng)
add_subdirectory(lib/lpng)
add_subdirectory(lib/lungif)
add_subdirectory(lib/zlib)
add_subdirectory(lib/ljpeg)
add_subdirectory(lib/tinyxml)
add_subdirectory(lib/opcode)
add_subdirectory(lib/squish)
add_subdirectory(lib/collada)
add_subdirectory(lib/pcre)
add_subdirectory(lib/convexDecomp)
if(TORQUE_SFX_VORBIS)
    add_subdirectory(lib/libvorbis)
    add_subdirectory(lib/libogg)
endif()
if(TORQUE_THEORA)
    add_subdirectory(lib/libtheora)
endif()
add_subdirectory(source)


